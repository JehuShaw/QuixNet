#include "CacheMemoryManager.h"
#include "NodeDefines.h"
#include "CacheDBManager.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "SeparatedStream.h"


using namespace util;
using namespace db;
using namespace thd;


MCResult CCacheMemory::AddToDB(bool bResetFlag /*= false*/)
{
	CTransferStream tranKey(GetKey(), false);
	CTypeString strTableName;
	tranKey.ReadToTypeString(strTableName);
    std::vector<CTypeString> strKeys;
	tranKey.ReadToTypeString(strKeys);

	if(!IsStringType(strTableName.GetType())) {
		OutputError("strTableName.empty()");
		return MCERR_NOTSTORED;
	}

    if(strKeys.empty()) {
        OutputError("strFieldKeys.empty()");
        return MCERR_NOTSTORED;
    }

	CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
    const CContainerData* pContainerData = pCacheDBMgr->GetContainer(strTableName.GetString());
    if(NULL == pContainerData) {
        OutputError("NULL == pContainerData");
        return MCERR_NOTSTORED;
    }

    CAutoPointer<Database> pDatabase(pCacheDBMgr->GetDatabase(m_u16DBID));
    if(pDatabase.IsInvalid()) {
        OutputError("pDatabase.IsInvalid()");
        return MCERR_NOTSTORED;
    }

	std::vector<CTypeString> strValues;
	uint8_t oldChgType = MCCHANGE_NIL;
	GetValueAndChgType(strValues, oldChgType);

    const std::vector<std::string>& valueColumns = pContainerData->GetValueColumns();
	const std::vector<int>& valueColumnsType = pContainerData->GetValueColumnsType();
	int nValueColumnSize = (int)valueColumns.size();
    int nValueSize = std::min((int)strValues.size(), nValueColumnSize);
	if(nValueSize < 1 && 1 == nValueColumnSize) {
		strValues.resize(nValueColumnSize);
		nValueSize = nValueColumnSize;
	}

    const std::vector<std::string>& keyColumns = pContainerData->GetKeyColumns();
	const std::vector<int>& keyColumnsType = pContainerData->GetKeyColumnsType();
    const std::vector<int>& leakyKeyIdxs = pContainerData->GetLeakyKeyIdxs();
    int nKeySize = (int)std::min(leakyKeyIdxs.size(), strKeys.size());

    if(nValueSize < 1 && nKeySize < 1) {
		if(MCCHANGE_NIL != oldChgType) {
			RecoverChgType(strValues, oldChgType);
		}
        OutputError("nValueSize < 1 && nKeySize < 1  nValueSize = %d nKeySize = %d", nValueSize, nKeySize);
        return MCERR_NOTSTORED;
    }

    std::string strSQL("INSERT INTO "SQL_FIELD_START);
	strSQL += pContainerData->GetSchema();
	strSQL += SQL_FIELD_END"."SQL_FIELD_START;
    strSQL += pContainerData->GetDbTable();
    strSQL += SQL_FIELD_END" (";
    for(int k = 0; k < nKeySize; ++k) {
        strSQL += SQL_FIELD_START;
        strSQL += keyColumns[leakyKeyIdxs[k]];
        strSQL += SQL_FIELD_END",";
    }
    for(int i = 0; i < nValueSize; ++i) {
        strSQL += SQL_FIELD_START;
        strSQL += valueColumns[i];
        strSQL += SQL_FIELD_END",";
    }
    strSQL += SQL_FIELD_START;
    strSQL += pContainerData->GetCasColumn();
    strSQL += SQL_FIELD_END;
	if(bResetFlag) {
		strSQL += ","SQL_FIELD_START;
		strSQL += pContainerData->GetFlagsColumn();
		strSQL += SQL_FIELD_END;
	}
    strSQL += ") VALUES (";
    for(int k = 0; k < nKeySize; ++k) {
		int nIndex = leakyKeyIdxs[k];
		pCacheDBMgr->AttachData(strSQL, keyColumnsType[nIndex], strKeys[k].GetString());
		strSQL += ',';
    }
    for(int j = 0; j < nValueSize; ++j) {
		pCacheDBMgr->AttachData(strSQL, valueColumnsType[j], strValues[j].GetString());
		strSQL += ',';
    }
    strSQL += CastToString(FlushDBCas());
	if(bResetFlag) {
		strSQL += ",0";
	}
    strSQL += ");";

    if(!pDatabase->WaitExecuteNA(strSQL.c_str())) {
		if(MCCHANGE_NIL != oldChgType) {
			RecoverChgType(strValues, oldChgType);
		}
        OutputError("!pDatabase->WaitExecuteNA");
        return MCERR_NOTSTORED;
    }
    return MCERR_OK;
}

MCResult CCacheMemory::LoadFromDB(
	bool bDBCas /*= true*/,
	const int32_t* pInFlag /*= NULL*/,
	int32_t* pOutFlag /*= NULL*/)
{
	CTransferStream tranKey(GetKey(), false);
	CTypeString strTableName;
	tranKey.ReadToTypeString(strTableName);
	std::vector<CTypeString> strKeys;
	tranKey.ReadToTypeString(strKeys);

	if(!IsStringType(strTableName.GetType())) {
		OutputError("strTableName.empty()");
		return MCERR_NOREPLY;
	}

    if(strKeys.empty()) {
        OutputError("strFieldKeys.empty()");
        return MCERR_NOREPLY;
    }

	CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
    const CContainerData* pContainerData = pCacheDBMgr->GetContainer(strTableName.GetString());
    if(NULL == pContainerData) {
        OutputError("NULL == pContainerData");
        return MCERR_NOREPLY;
    }

    CAutoPointer<Database> pDatabase(pCacheDBMgr->GetDatabase(m_u16DBID));
    if(pDatabase.IsInvalid()) {
        OutputError("pDatabase.IsInvalid()");
        return MCERR_NOREPLY;
    }

    const std::vector<std::string>& valueColumns = pContainerData->GetValueColumns();
	const std::vector<int>& valueColumnsType = pContainerData->GetValueColumnsType();
    int nValueSize = (int)valueColumns.size();
    if(nValueSize < 1) {
        OutputError("nValueSize < 1 nValueSize = %d", nValueSize);
        return MCERR_NOREPLY;
    }

    const std::vector<std::string>& keyColumns = pContainerData->GetKeyColumns();
	const std::vector<int>& keyColumnsType = pContainerData->GetKeyColumnsType();
    const std::vector<int>& leakyKeyIdxs = pContainerData->GetLeakyKeyIdxs();
	int nKeySize = (int)std::min(leakyKeyIdxs.size(), strKeys.size());

    std::string strSQL;
	if(NULL != pInFlag) {
		strSQL += SQL_BEGIN_TRAN"; ";
	}
	strSQL += "SELECT ";
    if(bDBCas) {
		if(NULL != pInFlag) {
			char szFlag[MAX_NUMBER_SIZE] = {0};
			ltostr(szFlag, *pInFlag, 10, 0);

			for(int i = 0; i < nValueSize; ++i) {
				strSQL += "CASE WHEN "SQL_FIELD_START;
				strSQL += pContainerData->GetFlagsColumn();
				strSQL += SQL_FIELD_END" = 0 OR "SQL_FIELD_START;
				strSQL += pContainerData->GetFlagsColumn();
				strSQL += SQL_FIELD_END" = ";
				strSQL += szFlag;
				strSQL += " THEN "SQL_FIELD_START;
				strSQL += valueColumns[i];
				strSQL += SQL_FIELD_END" ELSE NULL END,";
			}
		} else {
			for(int i = 0; i < nValueSize; ++i) {
				strSQL += SQL_FIELD_START;
				strSQL += valueColumns[i];
				strSQL += SQL_FIELD_END",";
			}
		}
        strSQL += SQL_FIELD_START;
        strSQL += pContainerData->GetCasColumn();
        strSQL += SQL_FIELD_END;
    } else {
        if(nValueSize > 0) {
			if(NULL != pInFlag) {
				char szFlag[MAX_NUMBER_SIZE] = {0};
				ltostr(szFlag, *pInFlag, 10, 0);

				for(int i = 0; i < nValueSize - 1; ++i) {
					strSQL += "CASE WHEN "SQL_FIELD_START;
					strSQL += pContainerData->GetFlagsColumn();
					strSQL += SQL_FIELD_END" = 0 OR "SQL_FIELD_START;
					strSQL += pContainerData->GetFlagsColumn();
					strSQL += SQL_FIELD_END" = ";
					strSQL += szFlag;
					strSQL += " THEN "SQL_FIELD_START;
					strSQL += valueColumns[i];
					strSQL += SQL_FIELD_END" ELSE NULL END,";
				}
				strSQL += "CASE WHEN "SQL_FIELD_START;
				strSQL += pContainerData->GetFlagsColumn();
				strSQL += SQL_FIELD_END" = 0 OR "SQL_FIELD_START;
				strSQL += pContainerData->GetFlagsColumn();
				strSQL += SQL_FIELD_END" = ";
				strSQL += szFlag;
				strSQL += " THEN "SQL_FIELD_START;
				strSQL += valueColumns[nValueSize - 1];
				strSQL += SQL_FIELD_END" ELSE NULL END";

			} else {
				for(int i = 0; i < nValueSize - 1; ++i) {
					strSQL += SQL_FIELD_START;
					strSQL += valueColumns[i];
					strSQL += SQL_FIELD_END",";
				}
				strSQL += SQL_FIELD_START;
				strSQL += valueColumns[nValueSize - 1];
				strSQL += SQL_FIELD_END;
			}
        }
    }
	if(NULL != pOutFlag) {
		strSQL += ","SQL_FIELD_START;
		strSQL += pContainerData->GetFlagsColumn();
		strSQL += SQL_FIELD_END;
	}
    strSQL += " FROM "SQL_FIELD_START;
	strSQL += pContainerData->GetSchema();
	strSQL += SQL_FIELD_END"."SQL_FIELD_START;
    strSQL += pContainerData->GetDbTable();
    strSQL += SQL_FIELD_END;
    if(nKeySize > 0) {
        strSQL += " WHERE ";
        for(int k = 0; k < nKeySize - 1; ++k) {
			int nIndex = leakyKeyIdxs[k];
            strSQL += SQL_FIELD_START;
            strSQL += keyColumns[nIndex];
			strSQL += SQL_FIELD_END" = ";
			pCacheDBMgr->AttachData(strSQL, keyColumnsType[nIndex], strKeys[k].GetString());
			strSQL += " AND ";
        }
		int nIndex = leakyKeyIdxs[nKeySize - 1];
        strSQL += SQL_FIELD_START;
        strSQL += keyColumns[nIndex];
		strSQL += SQL_FIELD_END" = ";
		pCacheDBMgr->AttachData(strSQL, keyColumnsType[nIndex], strKeys[nKeySize - 1].GetString());
    }
    strSQL += " LIMIT 1;";

    CAutoPointer<QueryResult> pQueryResult(pDatabase->QueryNA(strSQL.c_str()));
    if(pQueryResult.IsInvalid()) {
        return MCERR_NOTFOUND;
    }

    Field* pField = pQueryResult->Fetch();
    if(NULL == pField) {
        OutputError("NULL == pField");
        return MCERR_NOTFOUND;
    }

	bool bChange = true;
	if(NULL != pOutFlag) {
		if(bDBCas) {
			*pOutFlag = pField[nValueSize + 1].GetInt32();
		} else {
			*pOutFlag = pField[nValueSize].GetInt32();
		}
		if(0 != *pOutFlag && NULL != pInFlag && *pInFlag != *pOutFlag) {
			bChange = false;
		}
	}

	if(bChange) {
		util::CTransferStream inValues;

		for(int i = 0; i < nValueSize; ++i) {

			uint8_t u8Type = pCacheDBMgr->DBToStreamDataType(valueColumnsType[i]);
			const char * pValues = pField[i].GetString();
			if(NULL != pValues) {
				inValues.WriteFromTypeString(pValues, strlen(pValues), u8Type);
			} else {
				inValues.WriteFromTypeString(NULL, 0, u8Type);
			}
		}

		if(bDBCas) {
			uint64_t n64Cas = pField[nValueSize].GetUInt64();
			ChangeValueAndCas(inValues, n64Cas);
		} else {
			ChangeValue(inValues);
		}
	}

    return MCERR_OK;
}

MCResult CCacheMemory::UpdateToDB(
	bool bDBCas /*= true*/,
	bool bResetFlag /*= false*/)
{
	CTransferStream tranKey(GetKey(), false);
	CTypeString strTableName;
	tranKey.ReadToTypeString(strTableName);
	std::vector<CTypeString> strKeys;
	tranKey.ReadToTypeString(strKeys);

	if(!IsStringType(strTableName.GetType())) {
		OutputError("strTableName.empty()");
		return MCERR_NOTSTORED;
	}

    if(strKeys.empty()) {
        OutputError("strFieldKeys.empty()");
        return MCERR_NOTSTORED;
    }

	CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
    const CContainerData* pContainerData = pCacheDBMgr->GetContainer(strTableName.GetString());
    if(NULL == pContainerData) {
        OutputError("NULL == pContainerData");
        return MCERR_NOTSTORED;
    }

    CAutoPointer<Database> pDatabase(pCacheDBMgr->GetDatabase(m_u16DBID));
    if(pDatabase.IsInvalid()) {
        OutputError("pDatabase.IsInvalid()");
        return MCERR_NOTSTORED;
    }

    const std::vector<std::string>& keyColumns = pContainerData->GetKeyColumns();
	const std::vector<int>& keyColumnsType = pContainerData->GetKeyColumnsType();
	const std::vector<int>& leakyKeyIdxs = pContainerData->GetLeakyKeyIdxs();
	int nKeySize = (int)std::min(leakyKeyIdxs.size(), strKeys.size());

	uint8_t oldChgType = MCCHANGE_NIL;
	std::vector<CTypeString> strValues;
	GetValueAndChgType(strValues, oldChgType);

    const std::vector<std::string>& valueColumns = pContainerData->GetValueColumns();
	const std::vector<int>& valueColumnsType = pContainerData->GetValueColumnsType();
    int nValueColumnSize = (int)valueColumns.size();
    int nValueSize = (int)std::min((int)strValues.size(), nValueColumnSize);
    if(nValueSize < 1) {
        if(1 == nValueColumnSize) {
			strValues.resize(nValueColumnSize);
			nValueSize = nValueColumnSize;
		} else {
			if(MCCHANGE_NIL != oldChgType) {
				RecoverChgType(strValues, oldChgType);
			}
            OutputError("nValueSize < 1 && nValueColumnSize != 1");
            return MCERR_NOTSTORED;
        }
    }

    if(bDBCas)
    {
		std::string strSQL(SQL_UPDATE_GETCAS_BEFORE_UPDATE" UPDATE "SQL_FIELD_START);
		strSQL += pContainerData->GetSchema();
		strSQL += SQL_FIELD_END"."SQL_FIELD_START;
		strSQL += pContainerData->GetDbTable();
		strSQL += SQL_FIELD_END" SET ";
		uint64_t n64MemCas = GetDBCas();
        if(nValueSize > 0) {
            for(int i = 0; i < nValueSize; ++i) {
				CTypeString& typeString = strValues[i];
				if(typeString.IsIgnore()) {
					continue;
				}
                strSQL += SQL_FIELD_START;
                strSQL += valueColumns[i];
				strSQL += SQL_FIELD_END" = ";
				pCacheDBMgr->AttachData(strSQL, valueColumnsType[i], typeString.GetString());
				strSQL += ',';
            }
        } else {
            for(int i = 0; i < nValueColumnSize; ++i) {
                strSQL += SQL_FIELD_START;
                strSQL += valueColumns[i];
				strSQL += SQL_FIELD_END" = ";
				pCacheDBMgr->AttachData(strSQL, valueColumnsType[i], std::string());
				strSQL += ',';
            }
        }
        strSQL += SQL_FIELD_START;
        strSQL += pContainerData->GetCasColumn();
        strSQL += SQL_FIELD_END" = ";
        strSQL += CastToString(FlushDBCas());

		if(bResetFlag) {
			strSQL += ","SQL_FIELD_START;
			strSQL += pContainerData->GetFlagsColumn();
			strSQL += SQL_FIELD_END" = 0";
		}

		strSQL += SQL_UPDATE_GETCAS_IN_SET;

		if(nKeySize > 0) {
			strSQL += " WHERE ";
			for(int k = 0; k < nKeySize; ++k) {
				int nIndex = leakyKeyIdxs[k];
				strSQL += SQL_FIELD_START;
				strSQL += keyColumns[nIndex];
				strSQL += SQL_FIELD_END" = ";
				pCacheDBMgr->AttachData(strSQL, keyColumnsType[nIndex], strKeys[k].GetString());
				strSQL += " AND ";
			}
			strSQL += SQL_UPDATE_GETCAS_IN_WHERE_BEFORE_CAS_FIELD SQL_FIELD_START;
			strSQL += pContainerData->GetCasColumn();
			strSQL += SQL_FIELD_END SQL_UPDATE_GETCAS_IN_WHERE_AFTER_CAS_FIELD" = ";
			strSQL += CastToString(n64MemCas);
		}
		strSQL += " LIMIT 1; "SQL_UPDATE_GETCAS_AFTER_UPDATE;

		CAutoPointer<QueryResult> pQueryResult(pDatabase->QueryNA(strSQL.c_str()));
		if(pQueryResult.IsInvalid()) {
			if(MCCHANGE_NIL != oldChgType) {
				RecoverChgType(strValues, oldChgType);
			}
			OutputError("pQueryResult.IsInvalid()");
			return MCERR_NOTSTORED;
		}

		Field* pField = pQueryResult->Fetch();
		if(NULL == pField) {
			if(MCCHANGE_NIL != oldChgType) {
				RecoverChgType(strValues, oldChgType);
			}
			OutputError("NULL == pField");
			return MCERR_NOTSTORED;
		}

		if(NULL == pField[0].GetString()) {
			if(MCCHANGE_NIL != oldChgType) {
				RecoverChgType(strValues, oldChgType);
			}
			return MCERR_NOTFOUND;
		}

		uint64_t n64DBCas = pField[0].GetUInt64();
		if(n64DBCas != n64MemCas) {
			if(MCCHANGE_NIL != oldChgType) {
				RecoverChgType(strValues, oldChgType);
			}
			OutputError("n64DBCas != n64MemCas n64DBCas = "I64FMTD
				" n64MemCas = "I64FMTD, n64DBCas, n64MemCas);
			return MCERR_EXISTS;
		}
    } else {
		std::string strSQL("UPDATE "SQL_FIELD_START);
		strSQL += pContainerData->GetSchema();
		strSQL += SQL_FIELD_END"."SQL_FIELD_START;
		strSQL += pContainerData->GetDbTable();
		strSQL += SQL_FIELD_END" SET ";
        if(nValueSize > 0) {
			bool bSetField = false;
            for(int i = 0; i < nValueSize - 1; ++i) {
				CTypeString& typeString = strValues[i];
				if(typeString.IsIgnore()) {
					continue;
				}
                strSQL += SQL_FIELD_START;
                strSQL += valueColumns[i];
				strSQL += SQL_FIELD_END" = ";
				pCacheDBMgr->AttachData(strSQL, valueColumnsType[i], typeString.GetString());
				strSQL += ',';
				bSetField = true;
            }
			CTypeString& typeString = strValues[nValueSize - 1];
			if(!typeString.IsIgnore()) {
				strSQL += SQL_FIELD_START;
				strSQL += valueColumns[nValueSize - 1];
				strSQL += SQL_FIELD_END" = ";
				pCacheDBMgr->AttachData(strSQL, valueColumnsType[nValueSize - 1], typeString.GetString());
				
				if(bResetFlag) {
					strSQL += ","SQL_FIELD_START;
					strSQL += pContainerData->GetFlagsColumn();
					strSQL += SQL_FIELD_END" = 0";
				}
			} else {		
				if(bResetFlag) {
					strSQL += SQL_FIELD_START;
					strSQL += pContainerData->GetFlagsColumn();
					strSQL += SQL_FIELD_END" = 0";
				} else if(bSetField) {
					strSQL.erase(strSQL.size() - 1, 1);
				}
			}
        } else {
            if(nValueColumnSize > 0) {
                for(int i = 0; i < nValueColumnSize - 1; ++i) {
                    strSQL += SQL_FIELD_START;
                    strSQL += valueColumns[i];
					strSQL += SQL_FIELD_END" = ";
					pCacheDBMgr->AttachData(strSQL, valueColumnsType[i], std::string());
					strSQL += ',';
                }
                strSQL += SQL_FIELD_START;
                strSQL += valueColumns[nValueColumnSize - 1];
                strSQL += SQL_FIELD_END" = ";
				pCacheDBMgr->AttachData(strSQL, valueColumnsType[nValueColumnSize - 1], std::string());
				if(bResetFlag) {
					strSQL += ","SQL_FIELD_START;
					strSQL += pContainerData->GetFlagsColumn();
					strSQL += SQL_FIELD_END" = 0";
				}
            } else {
				if(bResetFlag) {
					strSQL += SQL_FIELD_START;
					strSQL += pContainerData->GetFlagsColumn();
					strSQL += SQL_FIELD_END" = 0";
				}
			}
        }

		if(nKeySize > 0) {
			strSQL += " WHERE ";
			for(int k = 0; k < nKeySize - 1; ++k) {
				int nIndex = leakyKeyIdxs[k];
				strSQL += SQL_FIELD_START;
				strSQL += keyColumns[nIndex];
				strSQL += SQL_FIELD_END" = ";
				pCacheDBMgr->AttachData(strSQL, keyColumnsType[nIndex], strKeys[k].GetString());
				strSQL += " AND ";
			}
			int nIndex = leakyKeyIdxs[nKeySize - 1];
			strSQL += SQL_FIELD_START;
			strSQL += keyColumns[nIndex];
			strSQL += SQL_FIELD_END" = ";
			pCacheDBMgr->AttachData(strSQL, keyColumnsType[nIndex], strKeys[nKeySize - 1].GetString());
		}
		strSQL += " LIMIT 1;";

		if(!pDatabase->WaitExecuteNA(strSQL.c_str())) {
			if(MCCHANGE_NIL != oldChgType) {
				RecoverChgType(strValues, oldChgType);
			}
			OutputError("!pDatabase->WaitExecuteNA");
			return MCERR_NOTSTORED;
		}
    }
    return MCERR_OK;
}

MCResult CCacheMemory::DeleteFromDB()
{
	CTransferStream tranKey(GetKey(), false);
	CTypeString strTableName;
	tranKey.ReadToTypeString(strTableName);
	std::vector<CTypeString> strKeys;
	tranKey.ReadToTypeString(strKeys);

	if(!IsStringType(strTableName.GetType())) {
		OutputError("strTableName.empty()");
		return MCERR_NOTSTORED;
	}

    if(strKeys.empty()) {
        OutputError("strFieldKeys.empty()");
        return MCERR_NOTSTORED;
    }

	CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
    const CContainerData* pContainerData = pCacheDBMgr->GetContainer(strTableName.GetString());
    if(NULL == pContainerData) {
        OutputError("NULL == pContainerData");
        return MCERR_NOTSTORED;
    }

    CAutoPointer<Database> pDatabase(pCacheDBMgr->GetDatabase(m_u16DBID));
    if(pDatabase.IsInvalid()) {
        OutputError("pDatabase.IsInvalid()");
        return MCERR_NOTSTORED;
    }

    const std::vector<std::string>& keyColumns = pContainerData->GetKeyColumns();
	const std::vector<int>& keyColumnsType = pContainerData->GetKeyColumnsType();
	const std::vector<int>& leakyKeyIdxs = pContainerData->GetLeakyKeyIdxs();
	int nKeySize = (int)std::min(leakyKeyIdxs.size(), strKeys.size());

    std::string strSQL("DELETE FROM "SQL_FIELD_START);
	strSQL += pContainerData->GetSchema();
	strSQL += SQL_FIELD_END"."SQL_FIELD_START;
    strSQL += pContainerData->GetDbTable();
    strSQL += SQL_FIELD_END;
    if(nKeySize > 0) {
        strSQL += " WHERE ";
        for(int k = 0; k < nKeySize - 1; ++k) {
			int nIndex = leakyKeyIdxs[k];
            strSQL += SQL_FIELD_START;
            strSQL += keyColumns[nIndex];
			strSQL += SQL_FIELD_END" = ";
			pCacheDBMgr->AttachData(strSQL, keyColumnsType[nIndex], strKeys[k].GetString());
			strSQL += " AND ";
        }
		int nIndex = leakyKeyIdxs[nKeySize - 1];
        strSQL += SQL_FIELD_START;
        strSQL += keyColumns[nIndex];
		strSQL += SQL_FIELD_END" = ";
		pCacheDBMgr->AttachData(strSQL, keyColumnsType[nIndex], strKeys[nKeySize - 1].GetString());
    }
    strSQL += " LIMIT 1;";

    if(!pDatabase->WaitExecuteNA(strSQL.c_str())) {
        OutputError("!pDatabase->WaitExecuteNA");
        return MCERR_NOTSTORED;
    }

    ResetChgType();
    return MCERR_OK;
}

MCResult CCacheMemory::SelectAllFromDB(
	uint16_t u16DBID,
    const std::string& strKey,
    uint32_t nOffset,
    uint32_t nCount,
    mc_record_set_t* pRecordSet,
	bool bCacheData,
    bool bDBCas /*= true*/)
{
	if(NULL == pRecordSet) {
		OutputError("NULL == pRecordSet");
		return MCERR_NOREPLY;
	}

	CTransferStream tsKey(strKey, false);
	CTypeString strTableName;
	tsKey.ReadToTypeString(strTableName);
	std::vector<CTypeString> strKeys;
	tsKey.ReadToTypeString(strKeys);

	if(!IsStringType(strTableName.GetType())) {
		OutputError("strTableName.empty()");
		return MCERR_NOREPLY;
	}

	CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
	const CContainerData* pContainerData = pCacheDBMgr->GetContainer(strTableName.GetString());
	if(NULL == pContainerData) {
		OutputError("NULL == pContainerData");
		return MCERR_NOREPLY;
	}

	CAutoPointer<Database> pDatabase(pCacheDBMgr->GetDatabase(u16DBID));
	if(pDatabase.IsInvalid()) {
		OutputError("pDatabase.IsInvalid()");
		return MCERR_NOREPLY;
	}

	const std::vector<std::string>& valueColumns = pContainerData->GetValueColumns();
	const std::vector<int>& valueColumnsType = pContainerData->GetValueColumnsType();
	int nValueColSize = (int)valueColumns.size();

	const std::vector<std::string>& keyColumns = pContainerData->GetKeyColumns();
	const std::vector<int>& keyColumnsType = pContainerData->GetKeyColumnsType();
	const std::vector<int>& leakyKeyIdxs = pContainerData->GetLeakyKeyIdxs();
	int nKeyColSize = (int)leakyKeyIdxs.size();

	if(nValueColSize < 1 && nKeyColSize < 1) {
		OutputError("nValueColSize < 1 && nKeyColSize < 1"
			" nValueColSize = %d nKeyColSize = %d", nValueColSize, nKeyColSize);
		return MCERR_NOREPLY;
	}

	std::string strSQL("SELECT ");
	if(bDBCas)
	{
		for(int i = 0; i < nKeyColSize; ++i) {
			int nIndex = leakyKeyIdxs[i];
			strSQL += SQL_FIELD_START;
			strSQL += keyColumns[nIndex];
			strSQL += SQL_FIELD_END",";
		}
		for(int i = 0; i < nValueColSize; ++i) {
			strSQL += SQL_FIELD_START;
			strSQL += valueColumns[i];
			strSQL += SQL_FIELD_END",";
		}
		strSQL += SQL_FIELD_START;
		strSQL += pContainerData->GetCasColumn();
		strSQL += SQL_FIELD_END;
	} else {

		if(nValueColSize > 0) {
			for(int i = 0; i < nKeyColSize; ++i) {
				int nIndex = leakyKeyIdxs[i];
				strSQL += SQL_FIELD_START;
				strSQL += keyColumns[nIndex];
				strSQL += SQL_FIELD_END",";
			}
			for(int i = 0; i < nValueColSize - 1; ++i) {
				strSQL += SQL_FIELD_START;
				strSQL += valueColumns[i];
				strSQL += SQL_FIELD_END",";
			}
			strSQL += SQL_FIELD_START;
			strSQL += valueColumns[nValueColSize - 1];
			strSQL += SQL_FIELD_END;
		} else {
			if(nKeyColSize > 0) {
				for(int i = 0; i < nKeyColSize - 1; ++i) {
					int nIndex = leakyKeyIdxs[i];
					strSQL += SQL_FIELD_START;
					strSQL += keyColumns[nIndex];
					strSQL += SQL_FIELD_END",";
				}
				int nIndex = leakyKeyIdxs[nKeyColSize - 1];
				strSQL += SQL_FIELD_START;
				strSQL += keyColumns[nIndex];
				strSQL += SQL_FIELD_END;
			}
		}
	}
	strSQL += " FROM "SQL_FIELD_START;
	strSQL += pContainerData->GetSchema();
	strSQL += SQL_FIELD_END"."SQL_FIELD_START;
	strSQL += pContainerData->GetDbTable();
	strSQL += SQL_FIELD_END;
	if(!strKeys.empty()) {

		const std::vector<int>& valueKeyIdxs = pContainerData->GetValueKeyIdxs();
		bool bValueKeyIdxsEmpty = valueKeyIdxs.empty();
		int nKeySize = 0;
		if(bValueKeyIdxsEmpty) {
			nKeySize = (int)std::min(keyColumns.size(), strKeys.size());
		} else {
			nKeySize = (int)std::min(valueKeyIdxs.size(), strKeys.size());
		}

		if(nKeySize > 0) {
			strSQL += " WHERE ";
			if(bValueKeyIdxsEmpty) {
				for(int k = 0; k < nKeySize - 1; ++k) {
					strSQL += SQL_FIELD_START;
					strSQL += keyColumns[k];
					strSQL += SQL_FIELD_END" = ";
					pCacheDBMgr->AttachData(strSQL, keyColumnsType[k], strKeys[k].GetString());
					strSQL += " AND ";
				}
				strSQL += SQL_FIELD_START;
				strSQL += keyColumns[nKeySize - 1];
				strSQL += SQL_FIELD_END" = ";
				pCacheDBMgr->AttachData(strSQL, keyColumnsType[nKeySize - 1], strKeys[nKeySize - 1].GetString());
			} else {
				for(int k = 0; k < nKeySize - 1; ++k) {
					int nIndex = valueKeyIdxs[k];
					strSQL += SQL_FIELD_START;
					strSQL += keyColumns[nIndex];
					strSQL += SQL_FIELD_END" = ";
					pCacheDBMgr->AttachData(strSQL, keyColumnsType[nIndex], strKeys[k].GetString());
					strSQL += " AND ";
				}
				int nIndex = valueKeyIdxs[nKeySize - 1];
				strSQL += SQL_FIELD_START;
				strSQL += keyColumns[nIndex];
				strSQL += SQL_FIELD_END" = ";
				pCacheDBMgr->AttachData(strSQL, keyColumnsType[nIndex], strKeys[nKeySize - 1].GetString());
			}
		}
	}
	if(nCount > 0) {
		if(nOffset > 0) {
			strSQL += " LIMIT ";
			strSQL += CastToString(nOffset);
			strSQL += ',';
			strSQL += CastToString(nCount);
		} else {
			strSQL += " LIMIT ";
			strSQL += CastToString(nCount);
		}
	}
	strSQL += " ;";

	CAutoPointer<QueryResult> pQueryResult(pDatabase->QueryNA(strSQL.c_str()));
	if(pQueryResult.IsInvalid()) {
		return MCERR_NOTFOUND;
	}

	int nFieldSize = pQueryResult->GetFieldCount();
	if(nFieldSize < 1) {
		return MCERR_OK;
	}

	if(nFieldSize < (nKeyColSize + nValueColSize)) {
		OutputError("nFieldSize < (nKeyColSize + nValueColSize)"
			" nFieldSize = %d nKeyColSize = %d nValueColSize = %d",
			nFieldSize, nKeyColSize, nValueColSize);
		return MCERR_NOREPLY;
	}

	do {
		Field* pField = pQueryResult->Fetch();
		if(NULL != pField) {
			mc_record_t* pMcRecord = RecordSetAdd(pRecordSet);

			CTransferStream tranKeys;
			tranKeys.Serialize(strTableName.GetString(), true);
			uint32_t u32KeyOffset = tranKeys.GetWriteOffset();

			if(nKeyColSize > 0) {
				for(int i = 0; i < nKeyColSize; ++i) {
					int nIndex = leakyKeyIdxs[i];
					uint8_t u8Type = pCacheDBMgr->DBToStreamDataType(keyColumnsType[nIndex]);
					const char * pKey = pField[i].GetString();
					if(NULL != pKey) {
						tranKeys.WriteFromTypeString(pKey, strlen(pKey), u8Type);
					} else {
						tranKeys.WriteFromTypeString(NULL, 0, u8Type);
					}
				}
			}
			CTransferStream tranValues;
			if(nValueColSize > 0) {
				for(int i = 0; i < nValueColSize; ++i) {
					uint8_t u8Type = pCacheDBMgr->DBToStreamDataType(valueColumnsType[i]);
					const char * pValue = pField[nKeyColSize + i].GetString();
					if(NULL != pValue) {
						tranValues.WriteFromTypeString(pValue, strlen(pValue), u8Type);
					} else {
						tranValues.WriteFromTypeString(NULL, 0, u8Type);
					}
				}
			}
			if(bDBCas) {
				uint64_t n64Cas = pField[nKeyColSize + nValueColSize].GetUInt64();
				const char* pKey = tranKeys.GetData() + TS_BITS_TO_BYTES(u32KeyOffset);
				int32_t nKeyLength = tranKeys.GetWriteOffset() - u32KeyOffset;
				if(nKeyLength < 0) {
					nKeyLength = 0;
				}
				nKeyLength = TS_BITS_TO_BYTES(nKeyLength);
				SetRecordNKey(pMcRecord, pKey, nKeyLength);
				SetRecordNValue(pMcRecord, tranValues.GetData(), tranValues.GetNumberOfBytesUsed());
				SetRecordCas(pMcRecord, n64Cas);
				if(bCacheData) {
					std::string strKey(tranKeys.GetData(), tranKeys.GetNumberOfBytesUsed());
					InsertCacheRecord(u16DBID, strKey, tranValues, true, n64Cas);
				}
			} else {
				const char* pKey = tranKeys.GetData() + TS_BITS_TO_BYTES(u32KeyOffset);
				int32_t nKeyLength = tranKeys.GetWriteOffset() - u32KeyOffset;
				if(nKeyLength < 0) {
					nKeyLength = 0;
				}
				nKeyLength = TS_BITS_TO_BYTES(nKeyLength);
				SetRecordNKey(pMcRecord, pKey, nKeyLength);
				SetRecordNValue(pMcRecord, tranValues.GetData(), tranValues.GetNumberOfBytesUsed());
				if(bCacheData) {
					std::string strKey(tranKeys.GetData(), tranKeys.GetNumberOfBytesUsed());
					InsertCacheRecord(u16DBID, strKey, tranValues, false, 0);
				}
			}
		} else {
			OutputError("NULL == pField");
		}
	} while (pQueryResult->NextRow());
	return MCERR_OK;
}

void CCacheMemory::RecoverChgType(const std::vector<util::CTypeString>& inValues, uint8_t oldChgType) {
	thd::CScopedWriteLock wrLock(m_rwTicket);
	m_strValue.RecoverUpdate(inValues);

	if(MCCHANGE_UPDATE == oldChgType) {
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}
}

void CCacheMemory::GetValueAndChgType(std::vector<CTypeString>& outValues, uint8_t& outChgType) {
	thd::CScopedReadLock rdLock(m_rwTicket);
	m_strValue.SerializeResetUpdate(outValues);
	outChgType = atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
}

MCResult CCacheMemory::GetsValue(CTransferStream& outValue, uint64_t& n64Cas) const {
    CScopedReadLock rdLock(m_rwTicket);
    m_strValue.Serialize(outValue);
    n64Cas = m_n64CasIncre + m_n64Cas;
    return MCERR_OK;
}

MCResult CCacheMemory::CheckAndSetValue(CTransferStream& inValue, uint64_t n64Cas) {
	CScopedWriteLock wrLock(m_rwTicket);
	if(n64Cas != (m_n64CasIncre + m_n64Cas)) {
		return MCERR_EXISTS;
	}
	m_strValue.ParseSetUpdate(inValue);
	++m_n64CasIncre;
    atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
    return MCERR_OK;
}

MCResult CCacheMemory::StoredProcedures(
	uint16_t u16DBID,
    const std::string& strProc,
	const std::string& strParam,
    mc_record_set_t* pRecordSet)
{
	// no reply
	if(NULL == pRecordSet) {
		OutputError("NULL == pRecordSet");
		return MCERR_NOREPLY;
	}

	CTransferStream tsProc(strProc, false);
	CTypeString strProcName;
	tsProc.ReadToTypeString(strProcName);

    if(!IsStringType(strProcName.GetType())) {
        OutputError("strProcName.empty()");
        return MCERR_NOREPLY;
    }

	CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
    const CContainerData* pContainerData = pCacheDBMgr->GetContainer(strProcName.GetString());
    if(NULL == pContainerData) {
		// It's meaning close the function, when empty the name.
		// return Ok for result.
        return MCERR_OK;
    }

	if(pContainerData->GetDbTable().empty()) {
		OutputError("pContainerData->GetDbTable().empty()");
		return MCERR_NOREPLY;
	}

    CAutoPointer<Database> pDatabase(pCacheDBMgr->GetDatabase(u16DBID));
    if(pDatabase.IsInvalid()) {
        OutputError("pDatabase.IsInvalid()");
        return MCERR_NOREPLY;
    }

	CTransferStream tsParams(strParam, false);
    std::vector<CTypeString> strParams;
	tsParams.ReadToTypeString(strParams);

	const std::vector<int>& keyColumnsType = pContainerData->GetKeyColumnsType();
	int nParamSize = (int)std::min(strParams.size(), keyColumnsType.size());

    std::string strSQL;
	strSQL += SQL_PROCEDURE_CALL" "SQL_FIELD_START;
	strSQL += pContainerData->GetSchema();
	strSQL += SQL_FIELD_END".";
    strSQL += pContainerData->GetDbTable();
    strSQL += SQL_PROCEDURE_BEFORE_PARAMS;
    if(nParamSize > 0) {
        int nLoopSize = nParamSize - 1;
        for(int i = 0; i < nLoopSize; ++i) {
			pCacheDBMgr->AttachData(strSQL, keyColumnsType[i], strParams[i].GetString());
			strSQL += ',';
        }
		pCacheDBMgr->AttachData(strSQL, keyColumnsType[nLoopSize], strParams[nLoopSize].GetString());
    }
    strSQL += SQL_PROCEDURE_AFTER_PARAMS;

	CAutoPointer<QueryResult> pQueryResult(pDatabase->QueryNA(strSQL.c_str()));
	if(pQueryResult.IsInvalid()) {
		return MCERR_NOTFOUND;
	}

    int nFieldSize = pQueryResult->GetFieldCount();
    if(nFieldSize < 1) {
        return MCERR_OK;
    }
	const std::vector<int>& valueColumnsType = pContainerData->GetValueColumnsType();

    do {
		Field* pField = pQueryResult->Fetch();
		if(NULL != pField) {
			mc_record_t* pMcRecord = RecordSetAdd(pRecordSet);
			CTransferStream tranValues;
			if(nFieldSize > 0) {
				uint8_t u8Type = STREAM_DATA_NIL;
				for(int i = 0; i < nFieldSize; ++i) {
					if(i < (int)valueColumnsType.size()) {
						u8Type = pCacheDBMgr->DBToStreamDataType(valueColumnsType[i]);
					} else {
						u8Type = STREAM_DATA_C_STRING;
					}
					const char * pValues = pField[i].GetString();
					if(NULL != pValues) {
						tranValues.WriteFromTypeString(pValues, strlen(pValues), u8Type);
					} else {
						tranValues.WriteFromTypeString(NULL, 0, u8Type);
					}
				}
			}
			SetRecordKey(pMcRecord, "");
			SetRecordNValue(pMcRecord, tranValues.GetData(), tranValues.GetNumberOfBytesUsed());
		} else {
			OutputError("NULL == pField");
		}
    } while (pQueryResult->NextRow());
    return MCERR_OK;
}

MCResult CCacheMemory::AsyncStoredProcedures(
	uint16_t u16DBID,
	const std::string& strProc,
	const std::string& strParam,
	mc_record_set_t* pRecordSet)
{
	CTransferStream tsProc(strProc, false);
	CTypeString strProcName;
	tsProc.ReadToTypeString(strProcName);

	if(!IsStringType(strProcName.GetType())) {
		OutputError("strProcName.empty()");
		return MCERR_NOTSTORED;
	}

	CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
	const CContainerData* pContainerData = pCacheDBMgr->GetContainer(strProcName.GetString());
	if(NULL == pContainerData) {
		// It's meaning close the function, when empty the name.
		// return Ok for result.
		return MCERR_OK;
	}

	if(pContainerData->GetDbTable().empty()) {
		OutputError("pContainerData->GetDbTable().empty()");
		return MCERR_NOTSTORED;
	}

	CAutoPointer<Database> pDatabase(pCacheDBMgr->GetDatabase(u16DBID));
	if(pDatabase.IsInvalid()) {
		OutputError("pDatabase.IsInvalid()");
		return MCERR_NOTSTORED;
	}

	CTransferStream tsParams(strParam, false);
	std::vector<CTypeString> strParams;
	tsParams.ReadToTypeString(strParams);

	const std::vector<int>& keyColumnsType = pContainerData->GetKeyColumnsType();
	int nParamSize = (int)std::min(strParams.size(), keyColumnsType.size());

	std::string strSQL;
	strSQL += SQL_PROCEDURE_CALL" "SQL_FIELD_START;
	strSQL += pContainerData->GetSchema();
	strSQL += SQL_FIELD_END".";
	strSQL += pContainerData->GetDbTable();
	strSQL += SQL_PROCEDURE_BEFORE_PARAMS;
	if(nParamSize > 0) {
		int nLoopSize = nParamSize - 1;
		for(int i = 0; i < nLoopSize; ++i) {
			pCacheDBMgr->AttachData(strSQL, keyColumnsType[i], strParams[i].GetString());
			strSQL += ',';
		}
		pCacheDBMgr->AttachData(strSQL, keyColumnsType[nLoopSize], strParams[nLoopSize].GetString());
	}
	strSQL += SQL_PROCEDURE_AFTER_PARAMS;

	if(NULL == pRecordSet) {
		if(!pDatabase->ExecuteNA(strSQL.c_str())) {
			OutputError("!pDatabase->Execute()");
			return MCERR_NOTSTORED;
		}
	} else {

		CAutoPointer<AsyncQueryWait> pAqw(new AsyncQueryWait);
		int nIdx = pAqw->AddQueryNA(strSQL.c_str());
		pDatabase->AddAsyncQuery(pAqw);
		pAqw->Wait();

		CAutoPointer<QueryResult> pQueryResult(pAqw->GetQueryResult(nIdx));
		if(pQueryResult.IsInvalid()) {
			return MCERR_NOTFOUND;
		}

		int nFieldSize = pQueryResult->GetFieldCount();
		if(nFieldSize < 1) {
			return MCERR_OK;
		}
		const std::vector<int>& valueColumnsType = pContainerData->GetValueColumnsType();

		do {
			Field* pField = pQueryResult->Fetch();
			if(NULL != pField) {
				mc_record_t* pMcRecord = RecordSetAdd(pRecordSet);
				CTransferStream tranValues;
				if(nFieldSize > 0) {
					uint8_t u8Type = STREAM_DATA_NIL;
					for(int i = 0; i < nFieldSize; ++i) {
						if(i < (int)valueColumnsType.size()) {
							u8Type = pCacheDBMgr->DBToStreamDataType(valueColumnsType[i]);
						} else {
							u8Type = STREAM_DATA_C_STRING;
						}
						const char * pValues = pField[i].GetString();
						if(NULL != pValues) {
							tranValues.WriteFromTypeString(pValues, strlen(pValues), u8Type);
						} else {
							tranValues.WriteFromTypeString(NULL, 0, u8Type);
						}
					}
				}
				SetRecordKey(pMcRecord, "");
				SetRecordNValue(pMcRecord, tranValues.GetData(), tranValues.GetNumberOfBytesUsed());
			} else {
				OutputError("NULL == pField");
			}
		} while (pQueryResult->NextRow());
	}
	return MCERR_OK;
}

MCResult CCacheMemory::ChangeFlag(uint16_t u16DBID, const std::string& strKey, int32_t nFlag)
{
	CTransferStream tsKey(strKey, false);
	CTypeString strTableName;
	tsKey.ReadToTypeString(strTableName);
	std::vector<CTypeString> strKeys;
	tsKey.ReadToTypeString(strKeys);

	if(!IsStringType(strTableName.GetType())) {
		OutputError("strTableName.empty()");
		return MCERR_NOREPLY;
	}

	CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
	const CContainerData* pContainerData = pCacheDBMgr->GetContainer(strTableName.GetString());
	if(NULL == pContainerData) {
		OutputError("NULL == pContainerData");
		return MCERR_NOREPLY;
	}

	CAutoPointer<Database> pDatabase(pCacheDBMgr->GetDatabase(u16DBID));
	if(pDatabase.IsInvalid()) {
		OutputError("pDatabase.IsInvalid()");
		return MCERR_NOREPLY;
	}

	const std::string& flagsColumn = pContainerData->GetFlagsColumn();

	const std::vector<std::string>& keyColumns = pContainerData->GetKeyColumns();
	const std::vector<int>& keyColumnsType = pContainerData->GetKeyColumnsType();
	const std::vector<int>& leakyKeyIdxs = pContainerData->GetLeakyKeyIdxs();
	int nKeySize = (int)std::min(leakyKeyIdxs.size(), strKeys.size());

	if(nKeySize < 1) {
		OutputError("nKeySize < 1  nKeySize = %d", nKeySize);
		return MCERR_NOREPLY;
	}

	std::string strSQL("UPDATE "SQL_FIELD_START);
	strSQL += pContainerData->GetSchema();
	strSQL += SQL_FIELD_END"."SQL_FIELD_START;
	strSQL += pContainerData->GetDbTable();
	strSQL += SQL_FIELD_END" SET "SQL_FIELD_START;
	strSQL += flagsColumn;
	strSQL += SQL_FIELD_END" = ";
	char szBuf[MAX_NUMBER_SIZE] = {0};
	ltostr(szBuf, nFlag, 10, 0);
	strSQL += szBuf;

	if(nKeySize > 0) {
		strSQL += " WHERE ";
		for(int k = 0; k < nKeySize - 1; ++k) {
			int nIndex = leakyKeyIdxs[k];
			strSQL += SQL_FIELD_START;
			strSQL += keyColumns[nIndex];
			strSQL += SQL_FIELD_END" = ";
			pCacheDBMgr->AttachData(strSQL, keyColumnsType[nIndex], strKeys[k].GetString());
			strSQL += " AND ";
		}
		int nIndex = leakyKeyIdxs[nKeySize - 1];
		strSQL += SQL_FIELD_START;
		strSQL += keyColumns[nIndex];
		strSQL += SQL_FIELD_END" = ";
		pCacheDBMgr->AttachData(strSQL, keyColumnsType[nIndex], strKeys[nKeySize - 1].GetString());
	}
	strSQL += " LIMIT 1;";

	if(!pDatabase->WaitExecuteNA(strSQL.c_str())) {
		OutputError("!pDatabase->WaitExecuteNA");
		return MCERR_NOTSTORED;
	}
	return MCERR_OK;
}


bool CCacheMemory::InsertCacheRecord(
	uint16_t u16DBID,
	const std::string& strKey,
	util::CTransferStream& inValue,
	bool bSetCas, uint64_t n64Cas)
{
	CCacheMemoryManager::PTR_T pCacheMemMgr(CCacheMemoryManager::Pointer());
	thd::CScopedWriteLock wrLock(pCacheMemMgr->m_rwTicket);

	util::CAutoPointer<CCacheMemory> pCacheRecord(new CCacheMemory(u16DBID, strKey));
	if(pCacheRecord.IsInvalid()) {
		OutputError("pCacheRecord.IsInvalid()1");
		return false;
	}
	std::pair<CCacheMemoryManager::CACHE_RECORDS_SET_T::iterator, bool>
		pairIB(pCacheMemMgr->m_records.insert(pCacheRecord));
	if(pairIB.second) {

		evt::CTimestampManager::PTR_T pTsMgr(evt::CTimestampManager::Pointer());
		pCacheRecord->LastActivityTime(pTsMgr->GetTimestamp());

		if(bSetCas) {
			pCacheRecord->ChangeValueAndCas(inValue, n64Cas);
		} else {
			pCacheRecord->ChangeValue(inValue);
		}
		// iterate the item
		int32_t nIndex = pCacheMemMgr->m_arrRecords.size();
		pCacheMemMgr->m_arrRecords.push_back(pCacheRecord);
		pCacheRecord->Index(nIndex);
	} else {
		pCacheRecord = *pairIB.first;
		if(pCacheRecord.IsInvalid()) {
			OutputError("pCacheRecord.IsInvalid()2");
			return false;
		}

		evt::CTimestampManager::PTR_T pTsMgr(evt::CTimestampManager::Pointer());
		pCacheRecord->LastActivityTime(pTsMgr->GetTimestamp());

		if(bSetCas) {
			pCacheRecord->ChangeValueAndCas(inValue, n64Cas);
		} else {
			pCacheRecord->ChangeValue(inValue);
		}
	}

	return true;
}

