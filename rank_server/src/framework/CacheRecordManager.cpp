#include "CacheRecordManager.h"
#include "CacheOperateHelper.h"
#include "TimerManager.h"
#include "GuidFactory.h"
#include "CallBack.h"
#include "Log.h"

using namespace util;
using namespace evt;
using namespace thd;

void CCacheRecordManager::Init() {
	if(m_bInit) {
		return;
	}
	m_bInit = true;

    m_timerId = CGuidFactory::Pointer()->CreateGuid();
    CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
    CAutoPointer<CallbackMFnP0<CCacheRecordManager> > pMethod(
        new CallbackMFnP0<CCacheRecordManager>(m_pThis(), &CCacheRecordManager::CheckAndUpdate));
    pTMgr->SetInterval(m_timerId, CACHE_RECORD_UPDATE_INTERVAL, pMethod);
}

void CCacheRecordManager::Dispose(bool bUpdateToCache/* = true*/) throw() {
    if(!m_bInit) {
        return;
    }
    m_bInit = false;

    CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
    pTMgr->Remove(m_timerId, true);

    if(bUpdateToCache) {
        CScopedWriteLock wrLock(m_rwTicket);

        USERID_TO_RECORDS_T::const_iterator itU(m_userIdToRecords.begin());
        while(m_userIdToRecords.end() != itU) {

            CACHE_RECORDS_SET_T::iterator itI(itU->second.begin());
            while(itU->second.end() != itI) {
                util::CAutoPointer<ICacheRecord>& pCacheMem =
                const_cast<util::CAutoPointer<ICacheRecord>&>(*itI);
                if(atomic_cmpxchg8(&pCacheMem->m_bLock,true,false) == false) {
                    if(pCacheMem->ChangeType() != MCCHANGE_NIL) {
                        MCResult nResult = pCacheMem->CasToCache();
                        if(MCERR_OK != nResult) {
                            OutputError("MCERR_OK != pCacheMem->CasToCache()"
                                " nResult = %d route = "I64FMTD, nResult, pCacheMem->Route());
                        }
                    }
                    atomic_xchg8(&pCacheMem->m_bLock, false);
                }
                ++itI;
            }
            ++itU;
        }
    }
}

bool CCacheRecordManager::CheckAndLoadRecord(CAutoPointer<ICacheRecord> pCacheRecord)
{
    if(pCacheRecord.IsInvalid()) {
		OutputError("pCacheRecord.IsInvalid()");
        return false;
    }
    MCResult nResult = pCacheRecord->LoadFromCache();
    if(MCERR_NOTFOUND == nResult) {

		nResult = pCacheRecord->AddToCache();
		if(MCERR_OK != nResult) {
			OutputError("MCERR_OK != pCacheRecord->AddToCache()"
				" nResult = %d route = "I64FMTD, nResult, pCacheRecord->Route());
			return false;
		}
    } else if(MCERR_OK != nResult) {
        OutputError("MCERR_OK != pCacheRecord->LoadFromCache()"
            " nResult = %d route = "I64FMTD, nResult, pCacheRecord->Route());
        return false;
    }
    return true;
}

bool CCacheRecordManager::DeleteRecord(CAutoPointer<ICacheRecord> pCacheRecord)
{
    if(pCacheRecord.IsInvalid()) {
		OutputError("pCacheRecord.IsInvalid()");
        return false;
    }

	MCResult nResult = pCacheRecord->DelFromCache();
	if(MCERR_OK != nResult) {
		OutputError("MCERR_OK != pCacheRecord->DeleteFromCache()"
			" nResult = %d route = "I64FMTD, nResult, pCacheRecord->Route());
		return false;
	}
    return true;
}

bool CCacheRecordManager::CheckUpdateAndReloadRecord(CAutoPointer<ICacheRecord> pCacheRecord) throw()
{
	if(pCacheRecord.IsInvalid()) {
		OutputError("pCacheRecord.IsInvalid()");
		return false;
	}

	if(atomic_cmpxchg8(&pCacheRecord->m_bLock, true, false) == false) {

		if(pCacheRecord->ChangeType() == MCCHANGE_NIL) {
			atomic_xchg8(&pCacheRecord->m_bLock, false);
			return false;
		}

		MCResult nResult = pCacheRecord->CasToCache();
		if(MCERR_OK != nResult) {
			if(MCERR_EXISTS == nResult) {
					OutputError("MCERR_EXISTS == CasToCache()"
						" route = "I64FMTD, pCacheRecord->Route());
			} else {
				OutputError("MCERR_OK != CasToCache() nResult = %d"
					" route = "I64FMTD, nResult, pCacheRecord->Route());
			}
		}

		atomic_xchg8(&pCacheRecord->m_bLock, false);
		return true;
	}
	return false;
}

bool CCacheRecordManager::ClearCacheAndStoreRecord(CAutoPointer<ICacheRecord> pCacheRecord) throw()
{
    if(pCacheRecord.IsInvalid()) {
		OutputError("pCacheRecord.IsInvalid()");
        return false;
    }

    atomic_xchg8(&pCacheRecord->m_bLock, true);
    MCResult nResult = pCacheRecord->StoreToCache();
    if(MCERR_OK != nResult) {
        OutputError("MCERR_OK != pCacheRecord->StoreToCache() nResult "
			"= %d route = "I64FMTD, nResult, pCacheRecord->Route());
    }
    atomic_xchg8(&pCacheRecord->m_bLock, false);
    return true;
}

void CCacheRecordManager::CheckAndUpdate()
{
    long nSize = GetArrCacheRecordSize();
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
		if(CheckUpdateAndReloadRecord(GetArrCacheRecord())) {
            break;
        }
	}
}





