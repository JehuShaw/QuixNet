/*
 * File:   CacheUserManager.h
 * Author: Jehu Shaw
 *
 * Created on 2014_8_4, 16:00
 */

#ifndef CACHEUSERMANAGER_H
#define	CACHEUSERMANAGER_H

#include "CacheRecordPool.h"


class CCacheUserManager
	: public util::Singleton<CCacheUserManager>
{
public:
	CCacheUserManager() : m_crPool() {
	}

	void Init() {
		m_crPool.Init();
	}

	void Dispose(bool bUpdateToCache = true) throw() {
		m_crPool.Dispose(bUpdateToCache);
	}

    eAddRecordResult AddCacheRecord(uint64_t userId, const util::CValueStream& strKeys,
		util::CAutoPointer<ICacheValue> pValue, bool bLoadFromCache = true)
    {
		return m_crPool.AddCacheRecord(userId, strKeys, pValue, bLoadFromCache);
    }

    bool RemoveCacheRecord(uint64_t userId, bool bUpdateToCache = true)
    {
		return m_crPool.RemoveCacheRecord(userId, bUpdateToCache);
    }

    bool RemoveCacheRecord(uint64_t userId, uint64_t objectId, bool bUpdateToCache = true)
    {
		return m_crPool.RemoveCacheRecord(userId, objectId, bUpdateToCache);
    }

	bool RemoveAndDeleteFromCache(uint64_t userId)
	{
		return m_crPool.RemoveAndDeleteFromCache(userId);
	}

	bool RemoveAndDeleteFromCache(uint64_t userId, uint64_t objectId)
	{
		return m_crPool.RemoveAndDeleteFromCache(userId, objectId);
	}

    bool UpdateCacheRecord(uint64_t userId, uint64_t objectId)
    {
		return m_crPool.UpdateCacheRecord(userId, objectId);
    }

private:
	CCacheRecordPool<uint64_t> m_crPool;
};

#endif /* CACHEUSERMANAGER_H */



