#include "CacheMemoryManager.h"
#include "CacheOperateHelper.h"
#include "TimerManager.h"
#include "GuidFactory.h"
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

    m_timerId = CGuidFactory::Pointer()->CreateGuid();
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

            CACHE_RECORDS_SET_T::iterator itI = m_records.begin();
            while(m_records.end() != itI) {
                util::CAutoPointer<ICacheMemory>& pCacheMem =
                    const_cast<util::CAutoPointer<ICacheMemory>&>(*itI);
                if(atomic_cmpxchg8(&pCacheMem->m_bLock,true,false) == false) {

					uint8_t u8ChgType = pCacheMem->ChangeType();
                    if(MCCHANGE_UPDATE == u8ChgType) {
                        MCResult nResult = pCacheMem->UpdateToDB();
                        if(MCERR_OK != nResult) {
                            OutputError("MCERR_OK != pCacheMem->UpdateToDB() nResult = %d ", nResult);
                        }
                    }

                    atomic_xchg8(&pCacheMem->m_bLock, false);
                }
                ++itI;
            }
    }
}

int CCacheMemoryManager::LoadCacheRecord(
	util::CAutoPointer<ICacheMemory>& outCacheRecord,
	uint16_t u16DBID, const std::string& strKey,
	bool bDBCas /*= true*/, uint64_t n64Cas/* = 0*/,
	const int32_t* pInFlag /*= NULL*/,
	int32_t* pOutFlag /*= NULL*/)
{
	CAutoPointer<ICacheMemory> pCacheRecord(InsertMemoryRecord(u16DBID, strKey, n64Cas));
	int nResult = LoadRecord(pCacheRecord, bDBCas, pInFlag, pOutFlag);
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
	bool bDBCas /*= true*/,
	const int32_t* pInFlag /*= NULL*/,
	int32_t* pOutFlag /*= NULL*/)
{
    if(pCacheRecord.IsInvalid()) {
        OutputError("pCacheRecord.IsInvalid()");
        return false;
    }
    MCResult nResult = pCacheRecord->LoadFromDB(bDBCas, pInFlag, pOutFlag);
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
	CAutoPointer<ICacheMemory> pCacheRecord,
	bool bResetFlag /*= false*/) throw()
{
	if(pCacheRecord.IsInvalid()) {
        OutputError("pCacheRecord.IsInvalid()");
		return false;
	}

	if(atomic_cmpxchg8(&pCacheRecord->m_bLock,true,false) == false) {

		uint8_t u8ChgType = pCacheRecord->ChangeType();
		if(MCCHANGE_NIL == u8ChgType) {
			atomic_xchg8(&pCacheRecord->m_bLock, false);
			return false;
		}

		MCResult nResult = pCacheRecord->UpdateToDB(true, bResetFlag);
        if(MCERR_NOTFOUND == nResult) {
            nResult = pCacheRecord->AddToDB(bResetFlag);
            if(MCERR_OK != nResult) {
                OutputError("MCERR_OK != pCacheRecord->AddToDB() nResult = %d", nResult);
            }
        } else if(MCERR_EXISTS == nResult) {
			OutputError("MCERR_EXISTS == UpdateToDB()");
            nResult = pCacheRecord->LoadFromDB();
            if(MCERR_OK != nResult) {
                OutputError("MCERR_OK != pCacheRecord->LoadFromDB() nResult = %d", nResult);
            }
        } else if(MCERR_OK != nResult) {
			OutputError("MCERR_OK != pCacheRecord->UpdateToDB() nResult = %d", nResult);
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
        	if(RemoveCacheRecord(pCacheRecord->GetKey(), true, true)) {
            	break;
            }
        } else {
        	if(CheckUpdateAndReloadRecord(pCacheRecord)) {
            	break;
        	}
      	}

	}
}


