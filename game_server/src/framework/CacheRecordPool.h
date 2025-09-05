/*
 * File:   CacheRecordPool.h
 * Author: Jehu Shaw
 *
 * Created on 2014_8_4, 16:00
 */

#ifndef CACHERECORDPOOL_H
#define	CACHERECORDPOOL_H

#include "NodeDefines.h"
#include "AutoPointer.h"
#include "ICacheRecord.h"
#include "ICacheValue.h"
#include "CacheRecord.h"
#include "SpinRWLock.h"
#include "ReferObject.h"
#include "CacheOperateHelper.h"
#include "TimerManager.h"
#include "LocalIDFactory.h"
#include "CallBack.h"
#include "Log.h"

#define CACHE_RECORD_UPDATE_INTERVAL 10

enum eAddRecordResult {
	ADD_RECORD_FAIL,
	ADD_RECORD_SUCCESS,
	ADD_RECORD_EXIST,
};

struct CacheRecordCompare
{
    bool operator()(const util::CAutoPointer<ICacheRecord>& pRecord1
        , const util::CAutoPointer<ICacheRecord>& pRecord2)const
    {
        return pRecord1->ObjectId() < pRecord2->ObjectId();
    }
};

class CCacheRecordFind : public ICacheRecord
{
public:
    CCacheRecordFind(uint64_t objectId): m_objectId(objectId) {
    }

    virtual MCResult AddToCache() {
        return (MCResult)0;
    }

    virtual MCResult LoadFromCache() {
        return (MCResult)0;
    }

    virtual MCResult StoreToCache() {
        return (MCResult)0;
    }

    virtual MCResult GetsFromCache() {
        return (MCResult)0;
    }

    virtual MCResult CasToCache() {
        return (MCResult)0;
    }

    virtual MCResult DelFromCache() {
        return (MCResult)0;
    }

    virtual uint8_t ChangeType() const {
        return MCCHANGE_NIL;
    }

    virtual int Index() const {
        return -1;
    }

    virtual void Index(int nIndex) {
    }

    virtual uint64_t Route() const {
        return 0;
    }

    virtual uint64_t ObjectId() const {
        return m_objectId;
    }

	virtual uint64_t Cas() const {
		return 0;
	}

	virtual const char* CacheKeyName() const {
		return "";
	}

private:
    uint64_t m_objectId;
};

template<typename ID_TYPE>
class CCacheRecordPool
{
public:
	CCacheRecordPool() : m_bInit(false), m_timerId(0), m_index(-1) {
		m_pThis(this);
	}

	void Init() {
		if (m_bInit) {
			return;
		}
		m_bInit = true;

		m_timerId = util::CLocalIDFactory::Pointer()->GenerateID();
		evt::CTimerManager::PTR_T pTMgr(evt::CTimerManager::Pointer());
		util::CAutoPointer<util::CallbackMFnP0<CCacheRecordPool> > pMethod(
			new util::CallbackMFnP0<CCacheRecordPool>(m_pThis(), &CCacheRecordPool::CheckAndUpdate));
		pTMgr->SetInterval(m_timerId, CACHE_RECORD_UPDATE_INTERVAL, pMethod);
	}

	void Dispose(bool bUpdateToCache = true) throw() {
		if (!m_bInit) {
			return;
		}
		m_bInit = false;

		evt::CTimerManager::PTR_T pTMgr(evt::CTimerManager::Pointer());
		pTMgr->Remove(m_timerId, true);

		if (bUpdateToCache) {
			thd::CScopedWriteLock wrLock(m_rwTicket);

			if (!m_idToRecords.empty()) {
				typename ID_TO_RECORDS_T::const_iterator itU(m_idToRecords.begin());
				while (m_idToRecords.end() != itU) {

					CACHE_RECORDS_SET_T::iterator itI(itU->second.begin());
					while (itU->second.end() != itI) {
						CheckAndUpdateRecord(*itI);
						++itI;
					}
					++itU;
				}
				m_cacheRecords.clear();
				m_idToRecords.clear();
			}
		}
	}

    eAddRecordResult AddCacheRecord(ID_TYPE id, const util::CValueStream& strKeys,
		util::CAutoPointer<ICacheValue> pValue, bool bLoadFromCache = true)
    {
        if(pValue.IsInvalid()) {
            return ADD_RECORD_FAIL;
        }

        thd::CScopedWriteLock wrLock(m_rwTicket);

        util::CAutoPointer<ICacheRecord> pCacheRecord(new CCacheRecord(
            id, strKeys, pValue));

        std::pair<typename ID_TO_RECORDS_T::iterator, bool> pairIBU(m_idToRecords.insert(
            typename ID_TO_RECORDS_T::value_type(id, CACHE_RECORDS_SET_T())));
        CACHE_RECORDS_SET_T& indexSet = pairIBU.first->second;
        std::pair<CACHE_RECORDS_SET_T::iterator, bool> pairIBI(indexSet.insert(pCacheRecord));
        if(!pairIBI.second) {
            // have inserted this record
            return ADD_RECORD_EXIST;
        }

		bool bRet = true;
		if(bLoadFromCache) {
			bRet = CheckAndLoadRecord(pCacheRecord);
		}

		// iterate the item
        int32_t nIndex = (int32_t)m_cacheRecords.size();
        m_cacheRecords.push_back(pCacheRecord);
		pCacheRecord->Index(nIndex);

		if(bRet) {
			return ADD_RECORD_SUCCESS;
		} else {
			return ADD_RECORD_FAIL;
		}
    }

    bool RemoveCacheRecord(ID_TYPE id, bool bUpdateToCache = true)
    {
        thd::CScopedWriteLock wrLock(m_rwTicket);

        typename ID_TO_RECORDS_T::const_iterator itU(m_idToRecords.find(id));
        if(m_idToRecords.end() == itU) {
            return false;
        }
        CACHE_RECORDS_SET_T::const_iterator itI(itU->second.begin());
        while(itU->second.end() != itI) {

            EraseCacheRecord((*itI)->Index());
            if(bUpdateToCache) {
				CheckAndUpdateRecord(*itI);
            }
            ++itI;
        }
        m_idToRecords.erase(itU);
        return true;
    }

    bool RemoveCacheRecord(ID_TYPE id, uint64_t objectId, bool bUpdateToCache = true)
    {
        thd::CScopedWriteLock wrLock(m_rwTicket);

        typename ID_TO_RECORDS_T::iterator itU(m_idToRecords.find(id));
        if(m_idToRecords.end() == itU) {
            return false;
        }
        CCacheRecordFind findKey(objectId);
        util::CAutoPointer<ICacheRecord> pFindKey((ICacheRecord*)&findKey, false);
        CACHE_RECORDS_SET_T::const_iterator itI(itU->second.find(pFindKey));
        if(itU->second.end() == itI) {
            return false;
        }

		util::CAutoPointer<ICacheRecord> pCacheRecord(*itI);
        EraseCacheRecord(pCacheRecord->Index());
        if((int)itU->second.size() < 2) {
            m_idToRecords.erase(itU);
        } else {
            itU->second.erase(itI);
        }

        if(bUpdateToCache) {
            return CheckAndUpdateRecord(pCacheRecord);
        }
        return true;
    }

	bool RemoveAndDeleteFromCache(ID_TYPE id)
	{
		thd::CScopedWriteLock wrLock(m_rwTicket);

		typename ID_TO_RECORDS_T::const_iterator itU(m_idToRecords.find(id));
		if(m_idToRecords.end() == itU) {
			return false;
		}
		CACHE_RECORDS_SET_T::const_iterator itI(itU->second.begin());
		while(itU->second.end() != itI) {

			EraseCacheRecord((*itI)->Index());
			DeleteRecord(*itI);

			++itI;
		}
		m_idToRecords.erase(itU);
		return true;
	}

	bool RemoveAndDeleteFromCache(ID_TYPE id, uint64_t objectId)
	{
		thd::CScopedWriteLock wrLock(m_rwTicket);

		typename ID_TO_RECORDS_T::iterator itU(m_idToRecords.find(id));
		if(m_idToRecords.end() == itU) {
			return false;
		}

        CCacheRecordFind findKey(objectId);
        util::CAutoPointer<ICacheRecord> pFindKey((ICacheRecord*)&findKey, false);
		CACHE_RECORDS_SET_T::const_iterator itI(itU->second.find(pFindKey));
		if(itU->second.end() == itI) {
			return false;
		}

		util::CAutoPointer<ICacheRecord> pCacheRecord(*itI);
		EraseCacheRecord(pCacheRecord->Index());

		if((int)itU->second.size() < 2) {
			m_idToRecords.erase(itU);
		} else {
			itU->second.erase(itI);
		}
		return DeleteRecord(pCacheRecord);
	}

    bool UpdateCacheRecord(ID_TYPE id, uint64_t objectId)
    {
        thd::CScopedReadLock rdLock(m_rwTicket);

        typename ID_TO_RECORDS_T::const_iterator itU(m_idToRecords.find(id));
        if(m_idToRecords.end() == itU) {
            return false;
        }
        CCacheRecordFind findKey(objectId);
        util::CAutoPointer<ICacheRecord> pFindKey((ICacheRecord*)&findKey, false);
        CACHE_RECORDS_SET_T::const_iterator itI(itU->second.find(pFindKey));
        if(itU->second.end() == itI) {
            return false;
        }

        return CheckAndUpdateRecord(*itI);
    }

private:
    inline bool EraseCacheRecord(int32_t index) {

        if(m_cacheRecords.empty()) {
            return false;
        }
        if(index < 0 || index >= (int32_t)m_cacheRecords.size()) {
            return false;
        }
		if(!m_cacheRecords[index].IsInvalid()) {
			m_cacheRecords[index]->Index(-1);
		}
        if(index != (int32_t) m_cacheRecords.size() - 1) {
            m_cacheRecords[index] = m_cacheRecords[m_cacheRecords.size() - 1];
			m_cacheRecords[index]->Index(index);
        }
        m_cacheRecords.pop_back();
        return true;
    }

    inline long GetArrCacheRecordSize() {
        thd::CScopedReadLock rdLock(m_rwTicket);
        return (long)m_cacheRecords.size();
    }

    inline util::CAutoPointer<ICacheRecord> GetArrCacheRecord() {
        thd::CScopedReadLock rdLock(m_rwTicket);
        unsigned long nSize = (unsigned long)m_cacheRecords.size();
        if(nSize < 1) {
            return util::CAutoPointer<ICacheRecord>();
        }
        unsigned long nIndex = (unsigned long)atomic_inc(&m_index) % nSize;
        return m_cacheRecords[nIndex];
    }

	static bool CheckAndLoadRecord(util::CAutoPointer<ICacheRecord> pCacheRecord) {
		if (pCacheRecord.IsInvalid()) {
			OutputError("pCacheRecord.IsInvalid()");
			return false;
		}
		MCResult nResult = pCacheRecord->LoadFromCache();
		if (MCERR_NOTFOUND == nResult) {

			nResult = pCacheRecord->AddToCache();
			if (MCERR_OK != nResult) {
				OutputError("MCERR_OK != pCacheRecord->AddToCache()"
					" nResult = %d route = " I64FMTD " cacheKeyName = %s ",
					nResult, pCacheRecord->Route(), pCacheRecord->CacheKeyName());
				return false;
			}
		} else if (MCERR_OK != nResult) {
			OutputError("MCERR_OK != pCacheRecord->LoadFromCache() nResult = %d route = " I64FMTD 
				" cacheKeyName = %s ", nResult, pCacheRecord->Route(), pCacheRecord->CacheKeyName());
			return false;
		}
		return true;
	}

	static bool DeleteRecord(util::CAutoPointer<ICacheRecord> pCacheRecord) {
		if (pCacheRecord.IsInvalid()) {
			OutputError("pCacheRecord.IsInvalid()");
			return false;
		}

		MCResult nResult = pCacheRecord->DelFromCache();
		if (MCERR_OK != nResult) {
			OutputError("MCERR_OK != pCacheRecord->DeleteFromCache() nResult = %d route = " I64FMTD 
				" cacheKeyName = %s ", nResult, pCacheRecord->Route(), pCacheRecord->CacheKeyName());
			return false;
		}
		return true;
	}

	static bool CheckAndUpdateRecord(util::CAutoPointer<ICacheRecord> pCacheRecord) throw() {
		if (pCacheRecord.IsInvalid()) {
			OutputError("pCacheRecord.IsInvalid()");
			return false;
		}

		if (atomic_cmpxchg8(&pCacheRecord->m_bLock, false, true) == false) {

			if (pCacheRecord->ChangeType() == MCCHANGE_NIL) {
				atomic_xchg8(&pCacheRecord->m_bLock, false);
				return false;
			}

			MCResult nResult = pCacheRecord->CasToCache();
			if (MCERR_OK != nResult) {
				if (MCERR_EXISTS == nResult) {
					OutputError("MCERR_EXISTS == CasToCache() route = " I64FMTD " cacheKeyName = %s cas = " 
						I64FMTD, pCacheRecord->Route(), pCacheRecord->CacheKeyName(), pCacheRecord->Cas());
				} else {
					OutputError("MCERR_OK != CasToCache() nResult = %d route = " I64FMTD " cacheKeyName = %s cas = " 
						I64FMTD, nResult, pCacheRecord->Route(), pCacheRecord->CacheKeyName(), pCacheRecord->Cas());
				}
			}

			atomic_xchg8(&pCacheRecord->m_bLock, false);
			return true;
		}
		return false;
	}

	static bool ClearCacheAndStoreRecord(util::CAutoPointer<ICacheRecord> pCacheRecord) throw() {
		if (pCacheRecord.IsInvalid()) {
			OutputError("pCacheRecord.IsInvalid()");
			return false;
		}

		atomic_xchg8(&pCacheRecord->m_bLock, true);
		MCResult nResult = pCacheRecord->StoreToCache();
		if (MCERR_OK != nResult) {
			OutputError("MCERR_OK != pCacheRecord->StoreToCache() nResult = %d route = " 
				I64FMTD " cacheKeyName = %s cas = " I64FMTD , nResult, pCacheRecord->Route(),
				pCacheRecord->CacheKeyName(), pCacheRecord->Cas());
		}
		atomic_xchg8(&pCacheRecord->m_bLock, false);
		return true;
	}

	void CheckAndUpdate() {
		long nSize = GetArrCacheRecordSize();
		if (nSize < 1) {
			return;
		}
		unsigned long idxStart = m_index;
		unsigned long idxEnd = idxStart + nSize;
		for (long i = 0; i < nSize; ++i) {
			if (idxStart < idxEnd) {
				if (m_index < idxStart
					|| m_index >= idxEnd)
				{
					break;
				}
			}
			else if (idxStart > idxEnd) {
				if (m_index < idxStart
					&& m_index >= idxEnd)
				{
					break;
				}
			}
			if (CheckAndUpdateRecord(GetArrCacheRecord())) {
				break;
			}
		}
	}

private:
    typedef std::set<util::CAutoPointer<ICacheRecord>, CacheRecordCompare> CACHE_RECORDS_SET_T;
    typedef std::map<ID_TYPE, CACHE_RECORDS_SET_T> ID_TO_RECORDS_T;
    typedef std::vector<util::CAutoPointer<ICacheRecord> > CACHE_RECORDS_ARRAY_T;

    ID_TO_RECORDS_T m_idToRecords;
    CACHE_RECORDS_ARRAY_T m_cacheRecords;
    thd::CSpinRWLock m_rwTicket;
    uint64_t m_timerId;
    volatile unsigned long m_index;
	bool m_bInit;

	util::CReferObject<CCacheRecordPool<ID_TYPE> > m_pThis;
};

#endif /* CACHERECORDPOOL_H */


