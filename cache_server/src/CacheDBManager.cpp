#include "CacheDBManager.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "SeparatedStream.h"
#include "StreamDataType.h"

#define DBSERVER_WAIT_TIME 1800000

using namespace db;
using namespace util;


struct SCBCtrlDBID {
	const uint32_t m_nSize;
	volatile uint32_t m_nCount;
	thd::CSpinEvent m_event;
	uint16_t m_u16DBId;

	SCBCtrlDBID(uint32_t nSize)
		: m_nSize(nSize)
		, m_nCount(0)
		, m_event()
		, m_u16DBId(0)
	{
		m_event.Suspend();
	}
};

static void SelectDBIdCallback(db::QueryResultVector & arg, uint16_t & p1, util::CAutoPointer<SCBCtrlDBID> & p2) {
	bool bFound = false;
	for(size_t i = 0; i < arg.size(); ++i) {
		if(!arg[i].result.IsInvalid()) {
			bFound = true;
		}
	}
	if(bFound) {
		if(0 == p2->m_u16DBId) {
			p2->m_u16DBId = p1;
		} else {
			OutputError("@SelectDBIdCallback already found the id.");
			assert(false);
		}
	}
	
	if((uint32_t)atomic_inc(&p2->m_nCount) >= p2->m_nSize) {
		p2->m_event.Resume();
	}
}

struct SCBCtrlMinLoad {
	volatile uint64_t m_u64MinLoad;
	const uint32_t m_nSize;
	volatile uint32_t m_nCount;
	thd::CSpinEvent m_event;
	uint16_t m_u16DBId;

	SCBCtrlMinLoad(uint32_t nSize)
		: m_u64MinLoad(-1)
		, m_nSize(nSize)
		, m_nCount(0)
		, m_event()
		, m_u16DBId(0)
	{
		m_event.Suspend();
	}
	~SCBCtrlMinLoad(){}
};

static void MinLoadDBIdCallback(db::QueryResultVector & arg, uint16_t & p1, util::CAutoPointer<SCBCtrlMinLoad> & p2) {
	for(size_t i = 0; i < arg.size(); ++i) {
		util::CAutoPointer<QueryResult> pQueryResult(arg[i].result);
		if(!pQueryResult.IsInvalid() && 0 != pQueryResult->GetFieldCount()) {
			Field* pField = pQueryResult->Fetch();
			if(NULL == pField) {
				continue;
			}
			uint64_t u64Count = pField[0].GetUInt64();
			uint64_t u64Old;

			do {
				u64Old = p2->m_u64MinLoad;
				if(u64Old <= u64Count) {
					break;
				}
				p2->m_u16DBId = p1;
			} while (atomic_cmpxchg64(&p2->m_u64MinLoad,
				u64Old, u64Count) != u64Old);
		}
	}

	if((uint32_t)atomic_inc(&p2->m_nCount) >= p2->m_nSize) {
		p2->m_event.Resume();
	}
}

struct SCheckKeyExist {
	const uint32_t m_nSize;
	volatile uint32_t m_nCount;
	thd::CSpinEvent m_event;
	bool m_bFound;

	SCheckKeyExist(uint32_t nSize)
		: m_nSize(nSize)
		, m_nCount(0)
		, m_event()
		, m_bFound(false)
	{
		m_event.Suspend();
	}
};

static void SelectKeyExistCallback(db::QueryResultVector & arg, util::CAutoPointer<SCheckKeyExist> & p1) {
	for (size_t i = 0; i < arg.size(); ++i) {
		if (!arg[i].result.IsInvalid()) {
			p1->m_bFound = true;
		}
	}

	if (p1->m_bFound || (uint32_t)atomic_inc(&p1->m_nCount) >= p1->m_nSize) {
		p1->m_event.Resume();
	}
}

struct SSelectResults {
	const uint32_t m_nSize;
	volatile uint32_t m_nCount;
	thd::CSpinEvent m_event;
	DB_RESUTLT_VEC_T* m_pResults;

	SSelectResults(uint32_t nSize, DB_RESUTLT_VEC_T* pResults)
		: m_nSize(nSize)
		, m_nCount(0)
		, m_event()
		, m_pResults(pResults)
	{
		m_event.Suspend();
	}
};

static void SelectResultsCallback(db::QueryResultVector & arg, util::CAutoPointer<SSelectResults> & p1) {

	if (NULL != p1->m_pResults) {
		for (size_t i = 0; i < arg.size(); ++i) {
			if (!arg[i].result.IsInvalid()) {
				p1->m_pResults->push_back(arg[i].result);
			}
		}
	}

	if ((uint32_t)atomic_inc(&p1->m_nCount) >= p1->m_nSize) {
		p1->m_event.Resume();
	}
}


CCacheDBManager::CCacheDBManager()
	: m_bInit(false)
	, m_cSeparator(0)
	, m_cTableMapDelimiter(0)
{
}

CCacheDBManager::~CCacheDBManager()
{
}

void CCacheDBManager::Init() {
	if(m_bInit) {
        OutputError("m_bInit");
		return;
	}
	m_bInit = true;

    AppConfig::PTR_T pConfig(AppConfig::Pointer());

    printf("connect database ...\n");
    util::CAutoPointer<db::Database> pDatabase(Database::CreateDatabaseInterface());
    if(pDatabase->Initialize(pConfig->GetString(APPCONFIG_DATABASEHOST).c_str(),
        pConfig->GetInt(APPCONFIG_DATABASEPORT),pConfig->GetString(APPCONFIG_DATABASEUSER).c_str(),
        pConfig->GetString(APPCONFIG_DATABASEPSW).c_str(),pConfig->GetString(APPCONFIG_INFODBNAME).c_str(),
        pConfig->GetInt(APPCONFIG_DATABASECONNECTION)))
    {
        printf("connect database success.\n");
    } else {
        printf("connect database fail.\n");
		assert(false);
		pDatabase->Dispose();
        return;
    }
    // 加载配置数据
    LoadConfigOptions(pDatabase);
	// 获得数据存储数据库名
	std::string strDataDBNames(pConfig->GetString(APPCONFIG_DATADBNAMES));
    // 加载表信息数据
    LoadContainers(pDatabase, strDataDBNames.c_str());
	// 加载数据库连接
	LoadDBServers(pDatabase, strDataDBNames.c_str());
	// 释放临时的数据库访问
	pDatabase->Dispose();
	// 让有效的数据库连接开始工作
	SetAllDBServerState(DBSERVER_STATE_START);
}

void CCacheDBManager::Dispose() {
    if(!m_bInit) {
        OutputError("!m_bInit");
        return;
    }
    m_bInit = false;
	DisposeAllDBServer();
}

void CCacheDBManager::LoadConfigOptions(util::CAutoPointer<db::Database>& pDatabase)
{
    CAutoPointer<QueryResult> pQueryResult(pDatabase->QueryNA(SQL_SP_CONFIG_OPTIONS));
    if(pQueryResult.IsInvalid()) {
        OutputError("%s Fail !", SQL_SP_CONFIG_OPTIONS);
        assert(false);
        return;
    }
	if(pQueryResult->GetFieldCount() != 2) {
		OutputError("%s pQueryResult->GetFieldCount() != 2", SQL_SP_CONFIG_OPTIONS);
		assert(false);
		return;
	}
    do {
        Field* pField = pQueryResult->Fetch();
		if(NULL != pField) {
			if(strcmp(pField[0].GetString(),"separator") == 0) {
				m_cSeparator = *pField[1].GetString();
			} else if(strcmp(pField[0].GetString(),"table_map_delimiter") == 0){
				m_cTableMapDelimiter = *pField[1].GetString();
			}
		} else {
			OutputError("%s NULL == pField", SQL_SP_CONFIG_OPTIONS);
			assert(false);
		}
    } while(pQueryResult->NextRow());
    if(0 == m_cSeparator) {
        OutputError("The \"config_options\" table, \"separator\" field must be set !");
        assert(false);
    }
    if(0 == m_cTableMapDelimiter) {
        OutputError("The \"config_options\" table, \"table_map_delimiter\" field must be set !");
        assert(false);
    }
}

void CCacheDBManager::LoadContainers(util::CAutoPointer<db::Database>& pDatabase, const char* szDataDBNames)
{
    CAutoPointer<QueryResult> pQueryResult(pDatabase->Query(SQL_SP_CONTAINERS, szDataDBNames));
    if(pQueryResult.IsInvalid()) {
        OutputError("%s Fail !", SQL_SP_CONTAINERS);
        assert(false);
        return;
    }
	if(pQueryResult->GetFieldCount() != 8) {
		OutputError("%s pQueryResult->GetFieldCount() != 8", SQL_SP_CONTAINERS);
		assert(false);
		return;
	}
    do {
        Field* pField = pQueryResult->Fetch();
		if(NULL != pField) {
			// name field
			const char* szField0 = pField[0].GetString();
			if(NULL == szField0 || szField0[0] == '\0') {
				OutputError("NULL == szField0 || szField0[0] == '\0'");
				assert(false);
				continue;
			}

			CContainerData containerData;
			// schema field
			const char* szField1 = pField[1].GetString();
			if(NULL != szField1 && szField1[0] != '\0') {
				containerData.m_strSchema = szField1;
			} else {
				OutputError("NULL == szField1 || szField1[0] == '\0'");
				assert(false);
				continue;
			}
			// table field
			const char* szField2 = pField[2].GetString();
			if(NULL != szField2 && szField2[0] != '\0') {
				containerData.m_strDbTable = szField2;
			}
			// cas field
			const char* szField5 = pField[5].GetString();
			// unique key field
			const char* szField6 = pField[6].GetString();
			// flags field 
			const char* szField7 = pField[7].GetString();

			bool hasField5 = (NULL != szField5 && szField5[0] != '\0');
			bool hasField6 = (NULL != szField6 && szField6[0] != '\0');
			bool hasField7 = (NULL != szField7 && szField7[0] != '\0');
			// Isn't Stored Procedures ?
			if(hasField6 || hasField5) {
				//////////////////////////////////////////////////////////////////////////
				DB_COLUMNS_TYPE_T columnsType;
				if(!containerData.m_strDbTable.empty() && !containerData.m_strSchema.empty()) {
					GetColumnsType(pDatabase, columnsType, containerData.m_strDbTable, containerData.m_strSchema);
				}
				containerData.m_nRealColumnSize = (int)columnsType.size();
				//////////////////////////////////////////////////////////////////////////
				// key field
				const char* szField3 = pField[3].GetString();
				if(NULL != szField3 && szField3[0] != '\0') {
					CSeparatedStream separated(szField3, strlen(szField3), false,
						m_cSeparator, m_cSeparator);
					separated >> containerData.m_strKeyColumns;

					int nKeySize = (int)containerData.m_strKeyColumns.size();
					for(int i = 0; i < nKeySize; ++i) {
						DB_COLUMNS_TYPE_T::const_iterator it(columnsType.find(
							containerData.m_strKeyColumns[i]));
						if(columnsType.end() != it) {
							containerData.m_keyColumnsType.push_back(it->second);
						} else {
							containerData.m_keyColumnsType.push_back(DB_TYPE_SIZE);
							OutputError("container name[%s] Can not find the field[%s]"
								, szField0, containerData.m_strKeyColumns[i].c_str());
							assert(false);
						}
					}
				}
				// value field
				const char* szField4 = pField[4].GetString();
				if(NULL != szField4 && szField4[0] != '\0') {
					CSeparatedStream separated(szField4, strlen(szField4), false,
						m_cSeparator, m_cSeparator);
					separated >> containerData.m_strValueColumns;
					int nValueSize = (int)containerData.m_strValueColumns.size();
					for(int i = 0; i < nValueSize; ++i) {
						DB_COLUMNS_TYPE_T::const_iterator it(columnsType.find(
							containerData.m_strValueColumns[i]));
						if(columnsType.end() != it) {
							containerData.m_valueColumnsType.push_back(it->second);
						} else {
							containerData.m_valueColumnsType.push_back(DB_TYPE_SIZE);
							OutputError("container name[%s] Can not find the field[%s]"
								, szField0, containerData.m_strValueColumns[i].c_str());
							assert(false);
						}
					}
				}
			} else {
				// This is Stored Procedures.
				containerData.m_nRealColumnSize = 0;
				// key field
				const char* szField3 = pField[3].GetString();
				if(NULL != szField3 && szField3[0] != '\0') {
					CSeparatedStream separated(szField3, strlen(szField3), false,
						m_cSeparator, m_cSeparator);
					std::vector<std::string> dataTypes;
					separated >> dataTypes;

					for(int i = 0; i < (int)dataTypes.size(); ++i) {
						StrToLower(dataTypes[i]);
						containerData.m_keyColumnsType.push_back(GetDBType(dataTypes[i]));
					}
				}
				// value field
				const char* szField4 = pField[4].GetString();
				if(NULL != szField4 && szField4[0] != '\0') {
					CSeparatedStream separated(szField4, strlen(szField4), false,
						m_cSeparator, m_cSeparator);
					std::vector<std::string> dataTypes;
					separated >> dataTypes;

					for(int i = 0; i < (int)dataTypes.size(); ++i) {
						StrToLower(dataTypes[i]);
						containerData.m_valueColumnsType.push_back(GetDBType(dataTypes[i]));
					}
				}
			}

			if(hasField5) {
				containerData.m_strCasColumn = szField5;
			}

			if(hasField6) {
				containerData.m_strUniqueIdxNameOnKey = szField6;
			}

			if(hasField7) {
				containerData.m_strFlagsColumn = szField7;
			}

			bool bKeyInValue;
			for(int i = 0; i < (int)containerData.m_strKeyColumns.size(); ++i) {
				bKeyInValue = false;
				for(int j = 0; j < (int)containerData.m_strValueColumns.size(); ++j) {
					if(containerData.m_strKeyColumns[i] == containerData.m_strValueColumns[j]) {
						bKeyInValue = true;
						break;
					}
				}
				if(bKeyInValue) {
					containerData.m_valueKeyIdxs.push_back(i);
				} else {
					containerData.m_leakyKeyIdxs.push_back(i);
				}
			}

			std::pair<DB_CONTAINERS_T::iterator, bool> pairIB(m_containers.insert(
				DB_CONTAINERS_T::value_type(std::string(szField0), containerData)));
			if(!pairIB.second) {
				OutputError("container Table name = %s already exist !", szField0);
				assert(false);
			}
		} else {
			OutputError("%s NULL == pField", SQL_SP_CONTAINERS);
			assert(false);
		}
    } while(pQueryResult->NextRow());
}

bool CCacheDBManager::DBServerAlreadyExists(uint16_t u16DBId)
{
	thd::CScopedReadLock rLock(m_dbSerRwLock);
	DB_SERVERS_T::const_iterator it(m_dbservers.find(u16DBId));
	if(m_dbservers.end() != it) {
		return true;
	}
	return false;
}

void CCacheDBManager::AddDBServer(uint16_t u16DBId, util::CAutoPointer<db::Database>& pDatabase)
{
	thd::CScopedWriteLock wLock(m_dbSerRwLock);
	m_dbservers.insert(DB_SERVERS_T::value_type(u16DBId, CDBServer(pDatabase)));
}

void CCacheDBManager::LoadDBServers(util::CAutoPointer<db::Database>& pDatabase, const char* szDataDBNames)
{
	CAutoPointer<QueryResult> pQueryResult(pDatabase->Query(SQL_SP_DBSERVERS, szDataDBNames));
	if(pQueryResult.IsInvalid()) {
		OutputError("%s Fail !", SQL_SP_DBSERVERS);
		assert(false);
		return;
	}

	if(pQueryResult->GetFieldCount() != 7) {
		OutputError("%s pQueryResult->GetFieldCount() != 7", SQL_SP_DBSERVERS);
		assert(false);
		return;
	}

	do {
		Field* pField = pQueryResult->Fetch();
		if(NULL != pField) {
			// name field
			uint16_t u16DBId = pField[0].GetUInt16();
			if(0 == u16DBId) {
				OutputError("0 == u16DBId");
				assert(false);
				continue;
			}

			if(DBServerAlreadyExists(u16DBId)) {
				OutputError("db server id = %d already exist !", u16DBId);
				assert(false);
				continue;
			}

			const char* szHost = pField[1].GetString();
			uint32_t nPort = pField[2].GetUInt32();
			const char* szUser = pField[3].GetString();
			const char* szPSW = pField[4].GetString();
			const char* szDb = pField[5].GetString();
			uint16_t nConnection = pField[6].GetUInt16();
			util::CAutoPointer<db::Database> pDB(Database::CreateDatabaseInterface());
			if(pDB->Initialize(szHost, nPort, szUser, szPSW, szDb, nConnection)) {
				AddDBServer(u16DBId, pDB);
			} else {
				OutputError("db server id = %d connect fail.", u16DBId);
				assert(false);
			}
			
		} else {
			OutputError("%s NULL == pField", SQL_SP_DBSERVERS);
			assert(false);
		}
	} while(pQueryResult->NextRow());
}

void CCacheDBManager::SetAllDBServerState(eDBServerState eServerState)
{
	thd::CScopedReadLock rLock(m_dbSerRwLock);
	DB_SERVERS_T::iterator it(m_dbservers.begin());
	for(; m_dbservers.end() != it; ++it) {
		atomic_xchg(&it->second.m_nState, eServerState);
	}
}

void CCacheDBManager::SetDBServerState(uint16_t u16DBId, eDBServerState eServerState)
{
	thd::CScopedReadLock rLock(m_dbSerRwLock);
	DB_SERVERS_T::iterator it(m_dbservers.find(u16DBId));
	if(m_dbservers.end() != it) {
		atomic_xchg(&it->second.m_nState, eServerState);
	}
}

int CCacheDBManager::GetDBType(const std::string& strType)
{
	return GetCacheDBType(strType);
}

void CCacheDBManager::AttachData(std::string& outString, int nDBType, const std::string& inData)
{
	switch(nDBType) {
	case DB_TYPE_TINYINT:
	case DB_TYPE_SMALLINT:
	case DB_TYPE_MEDIUMINT:
	case DB_TYPE_INT:
	case DB_TYPE_INTEGER:
	case DB_TYPE_BIGINT:
	case DB_TYPE_REAL:
	case DB_TYPE_DOUBLE:
	case DB_TYPE_FLOAT:
	case DB_TYPE_DECIMAL:
	case DB_TYPE_NUMERIC:
	case DB_TYPE_DATE:
	case DB_TYPE_TIME:
	case DB_TYPE_YEAR:
	case DB_TYPE_TIMESTAMP:
	case DB_TYPE_DATETIME:
		if(inData.empty()) {
			outString += '0';
		} else {
			outString += inData;
		}
		break;
	default:
		if(inData.empty()) {
			outString += "\'\'";
		} else {
			outString += '\'';
			outString += inData;
			outString += '\'';
		}
		break;
	}
}

void CCacheDBManager::GetColumnsType(
	util::CAutoPointer<db::Database>& pDatabase,
	DB_COLUMNS_TYPE_T& outColumnsType,
	const std::string& strTableName,
	const std::string& strDBName)
{
	CAutoPointer<QueryResult> pQueryColumns(pDatabase->Query(
		SQL_SP_COLUMNS_TYPE, strDBName.c_str(), strTableName.c_str()));
	if(pQueryColumns.IsInvalid()) {
		OutputError("%s Fail !", SQL_SP_COLUMNS_TYPE);
		assert(false);
		return;
	}
	if(pQueryColumns->GetFieldCount() != 2) {
		OutputError("%s pQueryColumns->GetFieldCount() != 2", SQL_SP_COLUMNS_TYPE);
		assert(false);
		return;
	}
	do {
		Field* pField = pQueryColumns->Fetch();
		if(NULL != pField) {
			if(NULL != pField[0].GetString()) {
				if(NULL != pField[1].GetString()) {
					std::string strColumnName(pField[0].GetString());
					std::string strType(pField[1].GetString());
					StrToLower(strType);
					outColumnsType[strColumnName] = GetDBType(strType);
				} else {
					OutputError("%s NULL == pField[1].GetString()", SQL_SP_COLUMNS_TYPE);
					assert(false);
				}
			} else {
				OutputError("%s NULL == pField[0].GetString()", SQL_SP_COLUMNS_TYPE);
				assert(false);
			}
		} else {
			OutputError("%s NULL == pField", SQL_SP_COLUMNS_TYPE);
			assert(false);
		}
	} while(pQueryColumns->NextRow());
}

int CCacheDBManager::DBToStreamDataType(int nDBType)
{
	switch(nDBType) {
	case DB_TYPE_TINYINT:
		return STREAM_DATA_UINT8;
	case DB_TYPE_SMALLINT:
		return STREAM_DATA_UINT16;
	case DB_TYPE_MEDIUMINT:
		return STREAM_DATA_INT32;
	case DB_TYPE_INTEGER:
		return STREAM_DATA_UINT32;
	case DB_TYPE_BIGINT:
		return STREAM_DATA_UINT64;
	case DB_TYPE_INT:
		return STREAM_DATA_UINT32;
	case DB_TYPE_BIT:
		return STREAM_DATA_C_STRING;
	case DB_TYPE_REAL:
		return STREAM_DATA_DOUBLE;
	case DB_TYPE_DOUBLE:
		return STREAM_DATA_DOUBLE;
	case DB_TYPE_FLOAT:
		return STREAM_DATA_FLOAT;
	case DB_TYPE_DECIMAL:
		return STREAM_DATA_C_STRING;
	case DB_TYPE_NUMERIC:
		return STREAM_DATA_C_STRING;
	case DB_TYPE_VARCHAR:
		return STREAM_DATA_C_STRING;
	case DB_TYPE_CHAR:
		return STREAM_DATA_C_STRING;
	case DB_TYPE_TIMESTAMP:
		return STREAM_DATA_UINT32;
	case DB_TYPE_DATETIME:
		return STREAM_DATA_UINT64;
	case DB_TYPE_DATE:
		return STREAM_DATA_INT32;
	case DB_TYPE_TIME:
		return STREAM_DATA_INT32;
	case DB_TYPE_YEAR:
		return STREAM_DATA_UINT8;
	case DB_TYPE_TINYBLOB:
	case DB_TYPE_MEDIUMBLOB:
	case DB_TYPE_LONGBLOB:
	case DB_TYPE_BLOB:
	case DB_TYPE_TINYTEXT:
	case DB_TYPE_MEDIUMTEXT:
	case DB_TYPE_LONGTEXT:
	case DB_TYPE_TEXT:
	case DB_TYPE_ENUM:
	case DB_TYPE_SET:
	case DB_TYPE_VARBINARY:
	case DB_TYPE_BINARY:
		return STREAM_DATA_C_STRING;
	default:
		return STREAM_DATA_NIL;
	}
}

int CCacheDBManager::GetDBNumTypeSize(int nDBType)
{
	switch(nDBType) {
	case DB_TYPE_TINYINT:
		return 1;
	case DB_TYPE_SMALLINT:
		return 2;
	case DB_TYPE_MEDIUMINT:
		return 3;
	case DB_TYPE_INTEGER:
		return 4;
	case DB_TYPE_BIGINT:
		return 8;
	case DB_TYPE_INT:
		return 4;
	case DB_TYPE_REAL:
		return 8;
	case DB_TYPE_DOUBLE:
		return 8;
	case DB_TYPE_FLOAT:
		return 4;
	case DB_TYPE_TIMESTAMP:
		return 4;
	case DB_TYPE_DATETIME:
		return 8;
	case DB_TYPE_DATE:
		return 3;
	case DB_TYPE_TIME:
		return 3;
	case DB_TYPE_YEAR:
		return 1;
	default:
		return 0;
	}
}

uint64_t CCacheDBManager::GetMaskByTypeSize(int nTypeSize)
{
	switch(nTypeSize)
	{
	case 1:
		return 0xFF;
	case 2:
		return 0xFFFF;
	case 3:
		return 0xFFFFFF;
	case 4:
		return 0xFFFFFFFF;
	case 8:
		return 0xFFFFFFFFFFFFFFFF;
	default:
		return 0;
	}
}

bool CCacheDBManager::IsDBString(int nDBType)
{
	switch (nDBType) {
	case DB_TYPE_VARCHAR:
	case DB_TYPE_CHAR:
	case DB_TYPE_TINYBLOB:
	case DB_TYPE_MEDIUMBLOB:
	case DB_TYPE_LONGBLOB:
	case DB_TYPE_BLOB:
	case DB_TYPE_TINYTEXT:
	case DB_TYPE_MEDIUMTEXT:
	case DB_TYPE_LONGTEXT:
	case DB_TYPE_TEXT:
		return true;
	default:
		break;
	}
	return false;
}

void CCacheDBManager::GetValidDbs(std::vector<SDBInfo>& outDbServers)
{
	thd::CScopedReadLock rLock(m_dbSerRwLock);
	DB_SERVERS_T::iterator it(m_dbservers.begin());
	for(; m_dbservers.end() != it; ++it) {
		CDBServer& dbServer = it->second;
		if(DBSERVER_STATE_START != dbServer.GetState()) {
			// 如果不是开启的状态不参与查找
			continue;
		}
		util::CAutoPointer<db::Database> pDatabase(dbServer.GetDatabase());
		if(pDatabase.IsInvalid()) {
			OutputError("pDatabase.IsInvalid()");
			assert(false);
			continue;
		}
		SDBInfo dbInfo;
		dbInfo.m_u16DBId = it->first;
		dbInfo.m_pDatabase = pDatabase;
		outDbServers.push_back(dbInfo);
	}
}

util::CAutoPointer<db::Database> CCacheDBManager::GetValidDb(uint16_t u16DBId)
{
	thd::CScopedReadLock rLock(m_dbSerRwLock);
	DB_SERVERS_T::iterator it(m_dbservers.find(u16DBId));
	if(m_dbservers.end() == it) {
		return util::CAutoPointer<db::Database>();
	}
	if(it->second.GetState() != DBSERVER_STATE_START) {
		assert(false);
		return util::CAutoPointer<db::Database>();
	}
	return it->second.GetDatabase();
}

uint16_t CCacheDBManager::SelectDBId(uint64_t u64Route, const CContainerData& tableInfo)
{
	const std::vector<std::string>& vecKeyColumns = tableInfo.GetKeyColumns();
	if(vecKeyColumns.empty()) {
		OutputError("vecKeyColumns.empty()");
		assert(false);
		return 0;
	}

	const std::vector<int>& vecKeyTypes =
		tableInfo.GetKeyColumnsType();
	
	std::vector<SDBInfo> dbServers;
	GetValidDbs(dbServers);

	size_t nSize = dbServers.size();
	util::CAutoPointer<SCBCtrlDBID> pCBCtrl(new SCBCtrlDBID(nSize));
	for(size_t i = 0; i < nSize; ++i) {
		uint16_t u16DBId = dbServers[i].m_u16DBId;
		util::CAutoPointer<db::Database> pDatabase(dbServers[i].m_pDatabase);
		
		util::CAutoPointer<db::SQLCallbackBase> pCallback(new db::SQLFuncCallbackP2<
			uint16_t, util::CAutoPointer<SCBCtrlDBID> >(SelectDBIdCallback, u16DBId, pCBCtrl));
		util::CAutoPointer<db::AsyncQueryCb> pAsyncQuery(new AsyncQueryCb(pCallback));

		std::string strSQL("SELECT " SQL_FIELD_START);
		strSQL += vecKeyColumns[0].c_str();
		strSQL += SQL_FIELD_END " FROM " SQL_FIELD_START;
		strSQL += tableInfo.GetSchema().c_str();
		strSQL += SQL_FIELD_END "." SQL_FIELD_START;
		strSQL += tableInfo.GetDbTable().c_str();
		strSQL += SQL_FIELD_END " WHERE ";

		int nUseTypeSize = 0;
		int nKeySize = (int)std::min(vecKeyColumns.size(), vecKeyTypes.size());
		int nCurTypeSize = GetDBNumTypeSize(vecKeyTypes[i]);
		if(nCurTypeSize > 0) {
			nUseTypeSize += nCurTypeSize;
			strSQL += SQL_FIELD_START;
			strSQL += vecKeyColumns[0].c_str();
			strSQL += SQL_FIELD_END " = ";
			char szValue[MAX_NUMBER_SIZE] = {0};
			ulltostr(szValue, (u64Route & GetMaskByTypeSize(nCurTypeSize)), 10, 0);
			strSQL += szValue;

			for(int i = 1; i < nKeySize; ++i) {
				nCurTypeSize = GetDBNumTypeSize(vecKeyTypes[i]);
				if(nCurTypeSize < 1) {
					break;
				}
				nUseTypeSize += nCurTypeSize;
				if(nUseTypeSize >= sizeof(u64Route)) {
					break;
				}
				
				strSQL += " AND " SQL_FIELD_START;
				strSQL += vecKeyColumns[i].c_str();
				strSQL += SQL_FIELD_END " = ";
				ulltostr(szValue, ((u64Route >> nUseTypeSize)
					& GetMaskByTypeSize(nCurTypeSize)), 10, 0);
				strSQL += szValue;	
			}
		} else {
			strSQL += SQL_FIELD_START;
			strSQL += vecKeyColumns[0].c_str();
			strSQL += SQL_FIELD_END " = ";
			char szValue[MAX_NUMBER_SIZE] = {0};
			ulltostr(szValue, u64Route, 10, 0);
			strSQL += szValue;
		}
		
		pAsyncQuery->AddQueryNA(strSQL.c_str());
		pDatabase->AddAsyncQuery(pAsyncQuery);
	}
	pCBCtrl->m_event.Wait(DBSERVER_WAIT_TIME);
	return pCBCtrl->m_u16DBId;
}

uint16_t CCacheDBManager::MinLoadDBId(uint64_t u64Route, const CContainerData& tableInfo)
{
	const std::vector<std::string>& vecKeyColumns = tableInfo.GetKeyColumns();
	if(vecKeyColumns.empty()) {
		OutputError("vecKeyColumns.empty()");
		assert(false);
		return 0;
	}

	const std::vector<int>& vecKeyTypes =
		tableInfo.GetKeyColumnsType();

	std::vector<SDBInfo> dbServers;
	GetValidDbs(dbServers);

	size_t nSize = dbServers.size();
	util::CAutoPointer<SCBCtrlMinLoad> pCBCtrl(new SCBCtrlMinLoad(nSize));
	if(nSize > 0) {
		pCBCtrl->m_u16DBId = dbServers[0].m_u16DBId;
	}
	for(size_t i = 0; i < nSize; ++i) {
		uint16_t u16DBId = dbServers[i].m_u16DBId;
		util::CAutoPointer<db::Database> pDatabase(dbServers[i].m_pDatabase);

		util::CAutoPointer<db::SQLCallbackBase> pCallback(new db::SQLFuncCallbackP2<
			uint16_t, util::CAutoPointer<SCBCtrlMinLoad> >(MinLoadDBIdCallback, u16DBId, pCBCtrl));
		util::CAutoPointer<db::AsyncQueryCb> pAsyncQuery(new AsyncQueryCb(pCallback));


		std::string strSQL("SELECT COUNT(" SQL_FIELD_START);
		strSQL += vecKeyColumns[0].c_str();
		strSQL += SQL_FIELD_END ") FROM " SQL_FIELD_START;
		strSQL += tableInfo.GetSchema().c_str();
		strSQL += SQL_FIELD_END "." SQL_FIELD_START;
		strSQL += tableInfo.GetDbTable().c_str();
		strSQL += SQL_FIELD_END " WHERE ";

		int nUseTypeSize = 0;
		int nKeySize = (int)std::min(vecKeyColumns.size(), vecKeyTypes.size());
		int nCurTypeSize = GetDBNumTypeSize(vecKeyTypes[i]);
		if(nCurTypeSize > 0) {
			nUseTypeSize += nCurTypeSize;
			strSQL += SQL_FIELD_START;
			strSQL += vecKeyColumns[0].c_str();
			strSQL += SQL_FIELD_END " = ";
			char szValue[MAX_NUMBER_SIZE] = {0};
			ulltostr(szValue, (u64Route & GetMaskByTypeSize(nCurTypeSize)), 10, 0);
			strSQL += szValue;

			for(int i = 1; i < nKeySize; ++i) {
				nCurTypeSize = GetDBNumTypeSize(vecKeyTypes[i]);
				if(nCurTypeSize < 1) {
					break;
				}
				nUseTypeSize += nCurTypeSize;
				if(nUseTypeSize >= sizeof(u64Route)) {
					break;
				}

				strSQL += " AND " SQL_FIELD_START;
				strSQL += vecKeyColumns[i].c_str();
				strSQL += SQL_FIELD_END " = ";
				ulltostr(szValue, ((u64Route >> nUseTypeSize)
					& GetMaskByTypeSize(nCurTypeSize)), 10, 0);
				strSQL += szValue;	
			}
		} else {
			strSQL += SQL_FIELD_START;
			strSQL += vecKeyColumns[0].c_str();
			strSQL += SQL_FIELD_END " = ";
			char szValue[MAX_NUMBER_SIZE] = {0};
			ulltostr(szValue, u64Route, 10, 0);
			strSQL += szValue;
		}

		pAsyncQuery->AddQueryNA(strSQL.c_str());
		pDatabase->AddAsyncQuery(pAsyncQuery);
	}
	pCBCtrl->m_event.Wait(DBSERVER_WAIT_TIME);
	return pCBCtrl->m_u16DBId;
}

bool CCacheDBManager::SaveRoute(uint16_t u16DBId, const CContainerData& tableInfo, uint64_t u64Route)
{
	const std::vector<std::string>& vecKeyColumns = tableInfo.GetKeyColumns();
	if(vecKeyColumns.empty()) {
		OutputError("vecKeyColumns.empty()");
		assert(false);
		return false;
	}
	util::CAutoPointer<db::Database> pDatabase(GetValidDb(u16DBId));
	if(pDatabase.IsInvalid()) {
		OutputError("pDatabase.IsInvalid()");
		assert(false);
		return false;
	}
	return pDatabase->WaitExecute(
		SQL_INSERT_ROUTE,
		tableInfo.GetSchema().c_str(),
		tableInfo.GetDbTable().c_str(),
		vecKeyColumns[0].c_str(),
		u64Route);
}

bool CCacheDBManager::CheckKeyExist(const std::string& strSQL)
{
	if (strSQL.empty()) {
		OutputError("strSQL.empty()");
		assert(false);
		return false;
	}

	std::vector<SDBInfo> dbServers;
	GetValidDbs(dbServers);

	size_t nSize = dbServers.size();
	util::CAutoPointer<SCheckKeyExist> pCBCtrl(new SCheckKeyExist(nSize));
	for (size_t i = 0; i < nSize; ++i) {
		uint16_t u16DBId = dbServers[i].m_u16DBId;
		util::CAutoPointer<db::Database> pDatabase(dbServers[i].m_pDatabase);

		util::CAutoPointer<db::SQLCallbackBase> pCallback(new db::SQLFuncCallbackP1<
			util::CAutoPointer<SCheckKeyExist> >(SelectKeyExistCallback, pCBCtrl));
		util::CAutoPointer<db::AsyncQueryCb> pAsyncQuery(new AsyncQueryCb(pCallback));


		pAsyncQuery->AddQueryNA(strSQL.c_str());
		pDatabase->AddAsyncQuery(pAsyncQuery);
	}
	pCBCtrl->m_event.Wait(DBSERVER_WAIT_TIME);
	return pCBCtrl->m_bFound;
}

bool CCacheDBManager::QueryFromAllDB(const std::string& strSQL, DB_RESUTLT_VEC_T* pResults)
{
	if (strSQL.empty()) {
		OutputError("strSQL.empty()");
		assert(false);
		return false;
	}

	std::vector<SDBInfo> dbServers;
	GetValidDbs(dbServers);

	size_t nSize = dbServers.size();
	util::CAutoPointer<SSelectResults> pCBCtrl(new SSelectResults(nSize, pResults));
	for (size_t i = 0; i < nSize; ++i) {
		uint16_t u16DBId = dbServers[i].m_u16DBId;
		util::CAutoPointer<db::Database> pDatabase(dbServers[i].m_pDatabase);

		util::CAutoPointer<db::SQLCallbackBase> pCallback(new db::SQLFuncCallbackP1<
			util::CAutoPointer<SSelectResults> >(SelectResultsCallback, pCBCtrl));
		util::CAutoPointer<db::AsyncQueryCb> pAsyncQuery(new AsyncQueryCb(pCallback));


		pAsyncQuery->AddQueryNA(strSQL.c_str());
		pDatabase->AddAsyncQuery(pAsyncQuery);
	}
	pCBCtrl->m_event.Wait(DBSERVER_WAIT_TIME);
	return true;
}

void CCacheDBManager::DisposeAllDBServer()
{
	thd::CScopedReadLock rLock(m_dbSerRwLock);
	DB_SERVERS_T::iterator it(m_dbservers.begin());
	for(; m_dbservers.end() != it; ++it) {
		util::CAutoPointer<db::Database> pDatabase(it->second.GetDatabase());
		if(pDatabase.IsInvalid()) {
			continue;
		}
		pDatabase->Dispose();
	}
}

