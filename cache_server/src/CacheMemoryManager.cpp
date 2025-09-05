#include "CacheMemoryManager.h"
#include "CacheOperateHelper.h"
#include "TimerManager.h"
#include "LocalIDFactory.h"
#include "CallBack.h"
#include "Log.h"

using namespace util;
using namespace evt;
using namespace thd;

void CCacheMemoryManager::Init() {
	if(m_bInit) {
        OutputError("m_bInit");
		return;
	}
	m_bInit = true;

    m_timerId = CLocalIDFactory::Pointer()->GenerateID();
    CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
    CAutoPointer<CallbackMFnP0<CCacheMemoryManager> > pMethod(
        new CallbackMFnP0<CCacheMemoryManager>(m_pThis(), &CCacheMemoryManager::CheckAndUpdate));
    pTMgr->SetInterval(m_timerId, CACHE_RECORD_UPDATE_INTERVAL, pMethod);
}

void CCacheMemoryManager::Dispose(bool bUpdateToDb/* = true*/) throw() {
    if(!m_bInit) {
        OutputError("!m_bInit");
        return;
    }
    m_bInit = false;

    CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
    pTMgr->Remove(m_timerId, true);

    if(bUpdateToDb) {
		CScopedWriteLock wrLock(m_rwTicket);
		
		if (!m_records.empty()) {
			CACHE_RECORDS_SET_T::iterator itI = m_records.begin();
			while (m_records.end() != itI) {
				CheckUpdateAndReloadRecord(*itI);
				++itI;
			}
			m_arrRecords.clear();
			m_records.clear();
		}
    }
}

int CCacheMemoryManager::LoadCacheRecord(
	util::CAutoPointer<ICacheMemory>& outCacheRecord,
	uint16_t u16DBID, const std::string& strKey,
	bool bDBCas /*= true*/, uint64_t n64Cas/* = 0*/)
{
	CAutoPointer<ICacheMemory> pCacheRecord(InsertMemoryRecord(u16DBID, strKey, n64Cas));
	int nResult = LoadRecord(pCacheRecord, bDBCas);
	if(MCERR_OK == nResult) {
		outCacheRecord = pCacheRecord;
	}
	return nResult;
}

bool CCacheMemoryManager::AddRecord(CAutoPointer<ICacheMemory> pCacheRecord)
{
    if(pCacheRecord.IsInvalid()) {
        OutputError("pCacheRecord.IsInvalid()");
        return false;
    }

    MCResult nResult = pCacheRecord->AddToDB();
    if(MCERR_OK != nResult) {
        nResult = pCacheRecord->LoadFromDB();
        if(MCERR_OK != nResult) {
            OutputError("MCERR_OK != pCacheRecord->AddToDB() nResult = %d ", nResult);
            return false;
        }
    }
    return true;
}

int CCacheMemoryManager::LoadRecord(
	CAutoPointer<ICacheMemory> pCacheRecord,
	bool bDBCas /*= true*/)
{
    if(pCacheRecord.IsInvalid()) {
        OutputError("pCacheRecord.IsInvalid()");
        return false;
    }
    MCResult nResult = pCacheRecord->LoadFromDB(bDBCas);
    if(MCERR_OK != nResult) {
        if(MCERR_NOTFOUND != nResult) {
            OutputError("MCERR_OK != pCacheRecord->LoadFromDB()1 nResult = %d", nResult);
        }
        return nResult;
    }
    return nResult;
}

bool CCacheMemoryManager::DeleteRecord(CAutoPointer<ICacheMemory> pCacheRecord)
{
    if(pCacheRecord.IsInvalid()) {
        OutputError("pCacheRecord.IsInvalid()");
        return false;
    }
    MCResult nResult = pCacheRecord->DeleteFromDB();
    if(MCERR_OK != nResult) {
        OutputError("MCERR_OK != pCacheRecord->DeleteFromDB() nResult = %d", nResult);
        return false;
    }
    return true;
}

bool CCacheMemoryManager::DeleteRecord(uint16_t u16DBID, const std::string& strKey)
{
    CCacheMemory cacheMemory(u16DBID, strKey);
    MCResult nResult = cacheMemory.DeleteFromDB();
    if(MCERR_OK != nResult) {
        OutputError("MCERR_OK != cacheMemory.DeleteFromDB() nResult = %d", nResult);
        return false;
    }
    return true;
}

bool CCacheMemoryManager::CheckUpdateAndReloadRecord(
	CAutoPointer<ICacheMemory> pCacheRecord) throw()
{
	if(pCacheRecord.IsInvalid()) {
        OutputError("pCacheRecord.IsInvalid()");
		return false;
	}

	if(atomic_cmpxchg8(&pCacheRecord->m_bLock, false, true) == false) {

		uint8_t u8ChgType = pCacheRecord->ChangeType();
		if(MCCHANGE_NIL == u8ChgType) {
			atomic_xchg8(&pCacheRecord->m_bLock, false);
			return false;
		}

		MCResult nResult = pCacheRecord->UpdateToDB(true);
        if(MCERR_NOTFOUND == nResult) {
            nResult = pCacheRecord->AddToDB();
            if(MCERR_OK != nResult) {
                OutputError("MCERR_OK != pCacheRecord->AddToDB() nResult = %d key = %s ", nResult, pCacheRecord->GetKey().c_str());
            }
        } else if(MCERR_EXISTS == nResult) {
			OutputError("MCERR_EXISTS == UpdateToDB()");
            nResult = pCacheRecord->LoadCasFromDB();
			if (MCERR_OK == nResult) {
				nResult = pCacheRecord->UpdateToDB(true);
				if (MCERR_OK != nResult) {
					OutputError("2 MCERR_OK == pCacheRecord->UpdateToDB nResult = %d key = %s ", nResult, pCacheRecord->GetKey().c_str());
				}
			} else {
                OutputError("MCERR_OK != pCacheRecord->LoadCasFromDB() nResult = %d key = %s ", nResult, pCacheRecord->GetKey().c_str());
            }
        } else if(MCERR_OK != nResult) {
			OutputError("1 MCERR_OK != pCacheRecord->UpdateToDB() nResult = %d key = %s ", nResult, pCacheRecord->GetKey().c_str());
		}

		atomic_xchg8(&pCacheRecord->m_bLock, false);
	}
	return true;
}

void CCacheMemoryManager::CheckAndUpdate()
{
    long nSize = GetArrRecordSize();
    if(nSize < 1) {
        return;
    }
    unsigned long idxStart = m_index;
    unsigned long idxEnd = idxStart + nSize;
	for(long i = 0; i < nSize; ++i) {
        if(idxStart < idxEnd) {
            if(m_index < idxStart
                || m_index >= idxEnd)
            {
                break;
            }
        } else if(idxStart > idxEnd) {
            if(m_index < idxStart
                && m_index >= idxEnd)
            {
                break;
            }
        }

        CAutoPointer<ICacheMemory> pCacheRecord(GetArrRecord());
        if(pCacheRecord.IsInvalid()) {
			continue;
		}

        CTimestampManager::PTR_T pTsMgr(CTimestampManager::Pointer());
        int64_t nDiffTime = (int64_t)pTsMgr->DiffTime(
			pTsMgr->GetTimestamp(), pCacheRecord->LastActivityTime());
        if(nDiffTime > g_recordExpireSec) {
        	if(RemoveCacheRecord(pCacheRecord->GetKey(), true)) {
            	break;
            }
        } else {
        	if(CheckUpdateAndReloadRecord(pCacheRecord)) {
            	break;
        	}
      	}

	}
}


