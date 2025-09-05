/* 
 * File:   CacheDBManager.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_8_4, 16:00
 */

#ifndef CACHEDBMANAGER_H
#define	CACHEDBMANAGER_H

#include "NodeDefines.h"
#include "AutoPointer.h"
#include "SpinRWLock.h"
#include "Database.h"
#include "Singleton.h"

#if defined(ENABLE_DATABASE_MYSQL)
#include "CacheMySqlAdpt.h"
#endif

typedef std::vector<util::CAutoPointer<db::QueryResult> > DB_RESUTLT_VEC_T;

class CContainerData {
public:
	const std::string& GetSchema() const {
		return m_strSchema;
	}
    const std::string& GetDbTable() const {
        return m_strDbTable;
    }
    const std::vector<std::string>& GetKeyColumns() const {
        return m_strKeyColumns;
    }
    const std::vector<std::string>& GetValueColumns() const {
        return m_strValueColumns;
    }
	const std::string& GetFlagsColumn() const {
		return m_strFlagsColumn;
	}
    const std::string& GetCasColumn() const {
        return m_strCasColumn;
    }
    const std::string& GetUniqueIdxNameOnKey() const {
        return m_strUniqueIdxNameOnKey;
    }
    const std::vector<int>& GetLeakyKeyIdxs() const {
        return m_leakyKeyIdxs;
    }
	const std::vector<int>& GetValueKeyIdxs() const {
		return m_valueKeyIdxs;
	}
	const std::vector<int>& GetKeyColumnsType() const {
		return m_keyColumnsType;
	}
	const std::vector<int>& GetValueColumnsType() const {
		return m_valueColumnsType;
	}
	int GetRealColumnSize() const {
		return m_nRealColumnSize;
	}
private:
    friend class CCacheDBManager;
	std::string m_strSchema;
    std::string m_strDbTable;
    std::vector<std::string> m_strKeyColumns;
    std::vector<std::string> m_strValueColumns;
	std::string m_strFlagsColumn;
    std::string m_strCasColumn;
    std::string m_strUniqueIdxNameOnKey;

    std::vector<int> m_leakyKeyIdxs;
	std::vector<int> m_valueKeyIdxs;
	std::vector<int> m_keyColumnsType;
	std::vector<int> m_valueColumnsType;
	int m_nRealColumnSize;
};

typedef std::map<std::string, CContainerData> DB_CONTAINERS_T;
typedef std::map<std::string, int> DB_COLUMNS_TYPE_T;

enum eDBServerState {
	DBSERVER_STATE_WAIT,
	DBSERVER_STATE_START,
};

class CDBServer{
public:
	CDBServer() : m_pDatabase(), m_nState(DBSERVER_STATE_WAIT)
	{
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(int32_t) < sizeof(long));
#endif
	}

	CDBServer(util::CAutoPointer<db::Database>& pDataBase)
		: m_pDatabase(pDataBase), m_nState(DBSERVER_STATE_WAIT)
	{
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(int32_t) < sizeof(long));
#endif
	}

	util::CAutoPointer<db::Database> GetDatabase() const {
		return m_pDatabase;
	}
	int32_t GetState() const {
		return m_nState;
	}

private:
	friend class CCacheDBManager;
	util::CAutoPointer<db::Database> m_pDatabase;
	volatile int32_t m_nState;
};

typedef std::map<uint16_t, CDBServer> DB_SERVERS_T;

class CCacheDBManager
	: public util::Singleton<CCacheDBManager>
{
public:
	CCacheDBManager();
	~CCacheDBManager();

    void Init();

    void Dispose();

    char GetSeparator() const {
        return m_cSeparator;
    }

    char GetTableMapDelimiter() const {
        return m_cTableMapDelimiter;
    }

    const CContainerData* GetContainer(std::string strName) {
        DB_CONTAINERS_T::iterator it = m_containers.find(strName);
        if(m_containers.end() != it) {
            return &it->second;
        }
        return NULL;
    }

    util::CAutoPointer<db::Database> GetDatabase(uint16_t u16DBId) {
		thd::CScopedReadLock rLock(m_dbSerRwLock);
        DB_SERVERS_T::const_iterator it(m_dbservers.find(u16DBId));
		if(m_dbservers.end() == it) {
			return util::CAutoPointer<db::Database>();
		}
		return it->second.GetDatabase();
    }

	util::CAutoPointer<db::Database> GetRandomDatabase() {
		thd::CScopedReadLock rLock(m_dbSerRwLock);
		DB_SERVERS_T::const_iterator it(m_dbservers.begin());
		if (m_dbservers.end() == it) {
			return util::CAutoPointer<db::Database>();
		}
		return it->second.GetDatabase();
	}

	void SetAllDBServerState(eDBServerState eServerState);

	void SetDBServerState(uint16_t u16DBId, eDBServerState eServerState);

	uint16_t GetDBIdByUserId(uint64_t userId) {

		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strUserIdTable(pConfig->GetString(APPCONFIG_BALUSERCNTRNAME));
		if(strUserIdTable.empty()) {
			return 0;
		}
		DB_CONTAINERS_T::const_iterator it(m_containers.find(strUserIdTable));
		if(m_containers.end() == it) {
			return 0;
		}
		return SelectDBId(userId, it->second);
	}

	uint16_t GetDBIdByDirServId(uint32_t serverId) {

		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strServIdTable(pConfig->GetString(APPCONFIG_DIRSERVCNTRNAME));
		if(strServIdTable.empty()) {
			return 0;
		}
		DB_CONTAINERS_T::const_iterator it(m_containers.find(strServIdTable));
		if(m_containers.end() == it) {
			return 0;
		}
		return SelectDBId(serverId, it->second);
	}

	uint16_t GetDBIdByBalServId(uint32_t serverId) {

		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strServIdTable(pConfig->GetString(APPCONFIG_BALSERVCNTRNAME));
		if(strServIdTable.empty()) {
			return 0;
		}
		DB_CONTAINERS_T::const_iterator it(m_containers.find(strServIdTable));
		if(m_containers.end() == it) {
			return 0;
		}
		return SelectDBId(serverId, it->second);
	}

	uint16_t GetMinLoadDBIdByBalUserId(uint64_t userId) {

		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strUserIdTable(pConfig->GetString(APPCONFIG_BALUSERCNTRNAME));
		if(strUserIdTable.empty()) {
			return 0;
		}
		DB_CONTAINERS_T::const_iterator it(m_containers.find(strUserIdTable));
		if(m_containers.end() == it) {
			return 0;
		}
		uint16_t u16DBId = MinLoadDBId(userId, it->second);
		if(0 == u16DBId) {
			return 0;
		}
		// 说明是专门创建一张表用来存储路由信息,这样的表帮它存储信息.
		if(it->second.GetRealColumnSize() == 1) {
			if(!SaveRoute(u16DBId, it->second, userId)) {
				return 0;
			}
		}
		return u16DBId;
	}

	uint16_t GetMinLoadDBIdByBalServId(uint32_t serverId) {

		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strServIdTable(pConfig->GetString(APPCONFIG_BALSERVCNTRNAME));
		if(strServIdTable.empty()) {
			return 0;
		}
		DB_CONTAINERS_T::const_iterator it(m_containers.find(strServIdTable));
		if(m_containers.end() == it) {
			return 0;
		}
		uint16_t u16DBId = MinLoadDBId(serverId, it->second);
		if(0 == u16DBId) {
			return 0;
		}
		// 说明是专门创建一张表用来存储路由信息,这样的表帮它存储信息.
		if(it->second.GetRealColumnSize() == 1) {
			if(!SaveRoute(u16DBId, it->second, serverId)) {
				return 0;
			}
		}
		return u16DBId;
	}

	uint16_t GetMinLoadDBIdByDirServId(uint32_t serverId) {

		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strServIdTable(pConfig->GetString(APPCONFIG_DIRSERVCNTRNAME));
		if(strServIdTable.empty()) {
			return 0;
		}
		DB_CONTAINERS_T::const_iterator it(m_containers.find(strServIdTable));
		if(m_containers.end() == it) {
			return 0;
		}
		uint16_t u16DBId = MinLoadDBId(serverId, it->second);
		if(0 == u16DBId) {
			return 0;
		}
		// 说明是专门创建一张表用来存储路由信息,这样的表帮它存储信息.
		if(it->second.GetRealColumnSize() == 1) {
			if(!SaveRoute(u16DBId, it->second, serverId)) {
				return 0;
			}
		}
		return u16DBId;
	}

	void AttachData(std::string& outString, int nDBType, const std::string& inData);

	static int DBToStreamDataType(int nDBType);

	static int GetDBNumTypeSize(int nDBType);

	static uint64_t GetMaskByTypeSize(int nTypeSize);

	static bool IsDBString(int nDBType);

	bool CheckKeyExist(const std::string& strSQL);

	bool QueryFromAllDB(const std::string& strSQL, DB_RESUTLT_VEC_T* pResults);

private:
    void LoadConfigOptions(util::CAutoPointer<db::Database>& pDatabase);
    void LoadContainers(util::CAutoPointer<db::Database>& pDatabase, const char* szDataDBName);
	bool DBServerAlreadyExists(uint16_t u16DBId);
	void AddDBServer(uint16_t u16DBId, util::CAutoPointer<db::Database>& pDatabase);
	void LoadDBServers(util::CAutoPointer<db::Database>& pDatabase, const char* szDataDBNames);
	void GetColumnsType(
		util::CAutoPointer<db::Database>& pDatabase,
		DB_COLUMNS_TYPE_T& outColumnsType, 
		const std::string& strTableName, 
		const std::string& strDBName);
	int GetDBType(const std::string& strType);

	struct SDBInfo
	{
		uint16_t m_u16DBId;
		util::CAutoPointer<db::Database> m_pDatabase;

		SDBInfo(): m_u16DBId(0), m_pDatabase() {}
	};
	void GetValidDbs(std::vector<SDBInfo>& outDbServers);

	util::CAutoPointer<db::Database> GetValidDb(uint16_t u16DBId);

	uint16_t SelectDBId(uint64_t u64Route, const CContainerData& tableInfo); 
	
	uint16_t MinLoadDBId(uint64_t u64Route, const CContainerData& tableInfo);

	bool SaveRoute(uint16_t u16DBId, const CContainerData& tableInfo, uint64_t u64Route);

	void DisposeAllDBServer();

private:
	bool m_bInit;
    
    char m_cSeparator;
    char m_cTableMapDelimiter;
    DB_CONTAINERS_T m_containers;
	DB_SERVERS_T m_dbservers;
	thd::CSpinRWLock m_dbSerRwLock;
};

#endif /* CACHEDBMANAGER_H */



