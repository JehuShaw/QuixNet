/*
 * File:   CacheRecordManager.h
 * Author: Jehu Shaw
 *
 * Created on 2014_8_4, 16:00
 */

#ifndef _CACHERECORDMANAGER_H
#define	_CACHERECORDMANAGER_H

#include "NodeDefines.h"
#include "AutoPointer.h"
#include "ICacheRecord.h"
#include "ICacheValue.h"
#include "CacheRecord.h"
#include "SpinRWLock.h"
#include "ReferObject.h"
#include "Singleton.h"

#define CACHE_RECORD_UPDATE_INTERVAL 1

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

private:
    uint64_t m_objectId;
};

class CCacheRecordManager
	: public util::Singleton<CCacheRecordManager>
{
public:
	CCacheRecordManager() : m_bInit(false), m_timerId(0), m_index(-1) {
		m_pThis(this);
	}

    void Init();

    void Dispose(bool bUpdateToCache = true) throw();

    eAddRecordResult AddCacheRecord(uint64_t userId, const util::CValueStream& strKeys,
		util::CAutoPointer<ICacheValue> pValue, bool bLoadFromCache = true)
    {
        if(pValue.IsInvalid()) {
            return ADD_RECORD_FAIL;
        }

        thd::CScopedWriteLock wrLock(m_rwTicket);

        util::CAutoPointer<ICacheRecord> pCacheRecord(new CCacheRecord(
            userId, strKeys, pValue));

        std::pair<USERID_TO_RECORDS_T::iterator, bool> pairIBU(m_userIdToRecords.insert(
            USERID_TO_RECORDS_T::value_type(userId, CACHE_RECORDS_SET_T())));
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

    eAddRecordResult AddCacheRecordEx(uint64_t userId,
        uint64_t u64Route, const util::CValueStream& strKeys,
        util::CAutoPointer<ICacheValue> pValue,
        bool bLoadFromCache = true)
    {
        if(pValue.IsInvalid()) {
            return ADD_RECORD_FAIL;
        }

        thd::CScopedWriteLock wrLock(m_rwTicket);

        util::CAutoPointer<ICacheRecord> pCacheRecord(new CCacheRecord(
            u64Route, strKeys, pValue));

        std::pair<USERID_TO_RECORDS_T::iterator, bool> pairIBU(m_userIdToRecords.insert(
            USERID_TO_RECORDS_T::value_type(userId, CACHE_RECORDS_SET_T())));
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
        int32_t nIndex = m_cacheRecords.size();
        m_cacheRecords.push_back(pCacheRecord);
        pCacheRecord->Index(nIndex);

		if(bRet) {
			return ADD_RECORD_SUCCESS;
		} else {
			return ADD_RECORD_FAIL;
		}
    }

    bool RemoveCacheRecord(uint64_t userId, bool bUpdateToCache = true)
    {
        thd::CScopedWriteLock wrLock(m_rwTicket);

        USERID_TO_RECORDS_T::const_iterator itU(m_userIdToRecords.find(userId));
        if(m_userIdToRecords.end() == itU) {
            return false;
        }
        CACHE_RECORDS_SET_T::const_iterator itI(itU->second.begin());
        while(itU->second.end() != itI) {

            EraseCacheRecord((*itI)->Index());
            if(bUpdateToCache) {
                ClearCacheAndStoreRecord(*itI);
            }
            ++itI;
        }
        m_userIdToRecords.erase(itU);
        return true;
    }

    bool RemoveCacheRecord(uint64_t userId, uint64_t objectId, bool bUpdateToCache = true)
    {
        thd::CScopedWriteLock wrLock(m_rwTicket);

        USERID_TO_RECORDS_T::iterator itU(m_userIdToRecords.find(userId));
        if(m_userIdToRecords.end() == itU) {
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
            m_userIdToRecords.erase(itU);
        } else {
            itU->second.erase(itI);
        }

        if(bUpdateToCache) {
            return ClearCacheAndStoreRecord(pCacheRecord);
        }
        return true;
    }

	bool RemoveAndDeleteFromCache(uint64_t userId)
	{
		thd::CScopedWriteLock wrLock(m_rwTicket);

		USERID_TO_RECORDS_T::const_iterator itU(m_userIdToRecords.find(userId));
		if(m_userIdToRecords.end() == itU) {
			return false;
		}
		CACHE_RECORDS_SET_T::const_iterator itI(itU->second.begin());
		while(itU->second.end() != itI) {

			EraseCacheRecord((*itI)->Index());
			DeleteRecord(*itI);

			++itI;
		}
		m_userIdToRecords.erase(itU);
	}

	bool RemoveAndDeleteFromCache(uint64_t userId, uint64_t objectId)
	{
		thd::CScopedWriteLock wrLock(m_rwTicket);

		USERID_TO_RECORDS_T::iterator itU(m_userIdToRecords.find(userId));
		if(m_userIdToRecords.end() == itU) {
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
			m_userIdToRecords.erase(itU);
		} else {
			itU->second.erase(itI);
		}
		return DeleteRecord(pCacheRecord);
	}

    bool UpdateCacheRecord(uint64_t userId, uint64_t objectId)
    {
        thd::CScopedReadLock rdLock(m_rwTicket);

        USERID_TO_RECORDS_T::const_iterator itU(m_userIdToRecords.find(userId));
        if(m_userIdToRecords.end() == itU) {
            return false;
        }
        CCacheRecordFind findKey(objectId);
        util::CAutoPointer<ICacheRecord> pFindKey((ICacheRecord*)&findKey, false);
        CACHE_RECORDS_SET_T::const_iterator itI(itU->second.find(pFindKey));
        if(itU->second.end() == itI) {
            return false;
        }

        return CheckUpdateAndReloadRecord(*itI);
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

    static bool CheckAndLoadRecord(util::CAutoPointer<ICacheRecord> pCacheRecord);

    static bool DeleteRecord(util::CAutoPointer<ICacheRecord> pCacheRecord);

	static bool CheckUpdateAndReloadRecord(util::CAutoPointer<ICacheRecord> pCacheRecord) throw();

    static bool ClearCacheAndStoreRecord(util::CAutoPointer<ICacheRecord> pCacheRecord) throw();

    void CheckAndUpdate();


private:
    typedef std::set<util::CAutoPointer<ICacheRecord>, CacheRecordCompare> CACHE_RECORDS_SET_T;
    typedef std::map<uint64_t, CACHE_RECORDS_SET_T> USERID_TO_RECORDS_T;
    typedef std::vector<util::CAutoPointer<ICacheRecord> > CACHE_RECORDS_ARRAY_T;

    USERID_TO_RECORDS_T m_userIdToRecords;
    CACHE_RECORDS_ARRAY_T m_cacheRecords;
    thd::CSpinRWLock m_rwTicket;
    uint64_t m_timerId;
    volatile unsigned long m_index;
	bool m_bInit;

	util::CReferObject<CCacheRecordManager> m_pThis;
};

#endif /* _CACHERECORDMANAGER_H */



