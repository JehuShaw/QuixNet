/*
 * File:   CacheMemoryManager.h
 * Author: Jehu Shaw
 *
 * Created on 2014_8_4, 16:00
 */

#ifndef _CACHEMEMORYMANAGER_H
#define	_CACHEMEMORYMANAGER_H

#include "NodeDefines.h"
#include "AutoPointer.h"
#include "CacheMemory.h"
#include "SpinRWLock.h"
#include "Log.h"
#include "TimestampManager.h"
#include "ReferObject.h"
#include "Singleton.h"

#define CACHE_RECORD_UPDATE_INTERVAL 1


struct CacheMemoryCompare
{
    bool operator()(const util::CAutoPointer<ICacheMemory>& pRecord1
        , const util::CAutoPointer<ICacheMemory>& pRecord2)const
    {
        return pRecord1->GetKey() < pRecord2->GetKey();
    }
};

class CCacheMemoryFind : public ICacheMemory
{
public:
    CCacheMemoryFind(const std::string& strKey)
		: m_strKey(strKey) {
    }

    virtual MCResult AddToDB(
		bool bResetFlag = false)
	{
        return (MCResult)0;
    }

	virtual MCResult LoadFromDB(
		bool bDBCas = true,
		const int32_t* pInFlag = NULL,
		int32_t* pOutFlag = NULL)
	{
        return (MCResult)0;
    }

    virtual MCResult UpdateToDB(
		bool bDBCas = true,
		bool bResetFlag = false)
	{
        return (MCResult)0;
    }

    virtual MCResult DeleteFromDB() {
        return (MCResult)0;
    }

    virtual const std::string& GetKey() const {
        return m_strKey;
    }

	virtual void SetValue(util::CTransferStream& inValue) {
    }

protected:
	virtual int Index() const {
		return -1;
	}

	virtual void Index(int nIndex) {
	}

    virtual uint64_t LastActivityTime() const {
        return 0;
    }

    virtual void LastActivityTime(uint64_t n64Timeout) {
    }

	virtual uint8_t ChangeType() const {
		return MCCHANGE_NIL;
	}

	virtual void ChangeValue(util::CTransferStream& inValue) {
	}

private:
    std::string m_strKey;
};

class CCacheMemoryManager
	: public util::Singleton<CCacheMemoryManager>
{
public:
	CCacheMemoryManager() : m_bInit(false), m_timerId(0), m_index(-1) {
		m_pThis(this);
	}

    void Init();

    void Dispose(bool bUpdateToDb = true) throw();

    bool AddCacheRecord(uint16_t u16DBID, const std::string& strKey, const std::string& strValue
        , bool bAddToDb = true)
    {
        util::CAutoPointer<ICacheMemory> pCacheRecord(InsertMemoryRecord(u16DBID, strKey, strValue));

        if(bAddToDb) {
            return AddRecord(pCacheRecord);
        }

        return true;
    }

    int LoadCacheRecord(util::CAutoPointer<ICacheMemory>& outCacheRecord,
		uint16_t u16DBID, const std::string& strKey,
		bool bDBCas = true, uint64_t n64Cas = 0,
		const int32_t* pInFlag = NULL,
		int32_t* pOutFlag = NULL);

    bool RemoveCacheRecord(const std::string& strKey, bool bUpdateToDb = true, bool bResetFlag = true)
    {
        util::CAutoPointer<ICacheMemory> pCacheRecord(RemoveMemoryRecord(strKey));
        if(pCacheRecord.IsInvalid()) {
            return false;
        }
        if(bUpdateToDb) {
            return CheckUpdateAndReloadRecord(pCacheRecord, bResetFlag);
        }
        return true;
    }

	bool RemoveAndDeleteFromDb(uint16_t u16DBID, const std::string& strKey)
	{
		util::CAutoPointer<ICacheMemory> pCacheRecord(RemoveMemoryRecord(strKey));
        if(pCacheRecord.IsInvalid()) {
            return DeleteRecord(u16DBID, strKey);
        } else {
		    return DeleteRecord(pCacheRecord);
        }
	}

    bool UpdateCacheRecord(const std::string& strKey)
    {
        util::CAutoPointer<ICacheMemory> pCacheMemory(FindMemoryRecord(strKey));
        if(pCacheMemory.IsInvalid()) {
            return false;
        }
        return CheckUpdateAndReloadRecord(pCacheMemory);
    }

    util::CAutoPointer<ICacheMemory> FindMemoryRecord(const std::string& strKey)
    {
        thd::CScopedReadLock rdLock(m_rwTicket);

        CCacheMemoryFind findKey(strKey);
        util::CAutoPointer<ICacheMemory> pFindKey((ICacheMemory*)&findKey, false);
        CACHE_RECORDS_SET_T::iterator itI = m_records.find(pFindKey);
        if(m_records.end() == itI) {
            return util::CAutoPointer<ICacheMemory>();
        }

        if(itI->IsInvalid()) {
			return *itI;
		}
        evt::CTimestampManager::PTR_T pTsMgr(evt::CTimestampManager::Pointer());
        const_cast<util::CAutoPointer<ICacheMemory>&>(*itI)->LastActivityTime(pTsMgr->GetTimestamp());

        return *itI;
    }

	util::CAutoPointer<ICacheMemory> RemoveMemoryRecord(const std::string& strKey)
	{
		thd::CScopedWriteLock wrLock(m_rwTicket);

		CCacheMemoryFind findKey(strKey);
		util::CAutoPointer<ICacheMemory> pFindKey((ICacheMemory*)&findKey, false);
		CACHE_RECORDS_SET_T::iterator itI = m_records.find(pFindKey);
		if(m_records.end() == itI) {
			return util::CAutoPointer<ICacheMemory>();
		}

		util::CAutoPointer<ICacheMemory> pCacheRecord(*itI);
		if(pCacheRecord.IsInvalid()) {
			return pCacheRecord;
		}
		EraseCacheRecord(pCacheRecord->Index());
		m_records.erase(itI);
		return pCacheRecord;
	}
	
	static bool CheckUpdateAndReloadRecord(
		util::CAutoPointer<ICacheMemory> pCacheRecord,
		bool bResetFlag = false) throw();

private:
	friend class CCacheMemory;

    inline util::CAutoPointer<ICacheMemory> InsertMemoryRecord(
		uint16_t u16DBID,
		const std::string& strKey,
        const std::string& strValue)
    {
        thd::CScopedWriteLock wrLock(m_rwTicket);

		util::CTransferStream tsValues(strValue, false);
        util::CAutoPointer<ICacheMemory> pCacheRecord(
			new CCacheMemory(u16DBID, strKey, tsValues));
		if(pCacheRecord.IsInvalid()) {
			return pCacheRecord;
		}
        std::pair<CACHE_RECORDS_SET_T::iterator, bool> pairIB(m_records.insert(pCacheRecord));
        if(pairIB.second) {

			evt::CTimestampManager::PTR_T pTsMgr(evt::CTimestampManager::Pointer());
			pCacheRecord->LastActivityTime(pTsMgr->GetTimestamp());

            int32_t nIndex = m_arrRecords.size();
            m_arrRecords.push_back(pCacheRecord);
            pCacheRecord->Index(nIndex);
        } else {
			pCacheRecord = *pairIB.first;
			if(pCacheRecord.IsInvalid()) {
				return pCacheRecord;
			}

			evt::CTimestampManager::PTR_T pTsMgr(evt::CTimestampManager::Pointer());
			pCacheRecord->LastActivityTime(pTsMgr->GetTimestamp());

            pCacheRecord->SetValue(tsValues);
        }
        return pCacheRecord;
    }

    inline util::CAutoPointer<ICacheMemory> InsertMemoryRecord(
		uint16_t u16DBID,
		const std::string& strKey,
		uint64_t n64Cas)
    {
        thd::CScopedWriteLock wrLock(m_rwTicket);

        util::CAutoPointer<ICacheMemory> pCacheRecord(new CCacheMemory(u16DBID, strKey, n64Cas));
		if(pCacheRecord.IsInvalid()) {
			return pCacheRecord;
		}
        std::pair<CACHE_RECORDS_SET_T::iterator, bool> pairIB(m_records.insert(pCacheRecord));
        if(pairIB.second) {

			evt::CTimestampManager::PTR_T pTsMgr(evt::CTimestampManager::Pointer());
			pCacheRecord->LastActivityTime(pTsMgr->GetTimestamp());

            int32_t nIndex = m_arrRecords.size();
            m_arrRecords.push_back(pCacheRecord);
            pCacheRecord->Index(nIndex);
        } else {
			pCacheRecord = *pairIB.first;
			if(pCacheRecord.IsInvalid()) {
				return pCacheRecord;
			}

			evt::CTimestampManager::PTR_T pTsMgr(evt::CTimestampManager::Pointer());
			pCacheRecord->LastActivityTime(pTsMgr->GetTimestamp());

		}
        return pCacheRecord;
    }

    inline long GetArrRecordSize() {
        thd::CScopedReadLock rdLock(m_rwTicket);
        return (long)m_arrRecords.size();
    }

    inline util::CAutoPointer<ICacheMemory> GetArrRecord() {
        thd::CScopedReadLock rdLock(m_rwTicket);
        unsigned long nSize = (unsigned long)m_arrRecords.size();
        if(nSize < 1) {
            return util::CAutoPointer<ICacheMemory>();
        }
        unsigned long nIndex = (unsigned long)atomic_inc(&m_index) % nSize;
        return m_arrRecords[nIndex];
    }

private:
    inline bool EraseCacheRecord(int32_t index) {

        if(m_arrRecords.empty()) {
            return false;
        }
        if(index < 0 || index >= (int32_t)m_arrRecords.size()) {
            return false;
        }
		if(!m_arrRecords[index].IsInvalid()) {
			m_arrRecords[index]->Index(-1);
		}
        if(index != (int32_t) m_arrRecords.size() - 1) {
            m_arrRecords[index] = m_arrRecords[m_arrRecords.size() - 1];
			m_arrRecords[index]->Index(index);
        }
        m_arrRecords.pop_back();
        return true;
    }

	static bool AddRecord(util::CAutoPointer<ICacheMemory> pCacheRecord);

	static int LoadRecord(
		util::CAutoPointer<ICacheMemory> pCacheRecord,
		bool bDBCas = true,
		const int32_t* pInFlag = NULL,
		int32_t* pOutFlag = NULL);

	static bool DeleteRecord(util::CAutoPointer<ICacheMemory> pCacheRecord);

	static bool DeleteRecord(uint16_t u16DBID, const std::string& strKey);

    void CheckAndUpdate();

private:
    typedef std::set<util::CAutoPointer<ICacheMemory>, CacheMemoryCompare> CACHE_RECORDS_SET_T;
    typedef std::vector<util::CAutoPointer<ICacheMemory> > CACHE_RECORDS_ARRAY_T;

	util::CReferObject<CCacheMemoryManager> m_pThis;
    CACHE_RECORDS_SET_T m_records;
    CACHE_RECORDS_ARRAY_T m_arrRecords;
    thd::CSpinRWLock m_rwTicket;
    uint64_t m_timerId;
	volatile unsigned long m_index;
	bool m_bInit;
};

#endif /* _CACHEMEMORYMANAGER_H */



