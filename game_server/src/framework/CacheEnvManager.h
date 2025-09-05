/*
 * File:   CacheEnvManager.h
 * Author: Jehu Shaw
 *
 * Created on 2014_8_4, 16:00
 */

#ifndef CACHEENVMANAGER_H
#define	CACHEENVMANAGER_H

#include "CacheRecordPool.h"


class CCacheEnvManager
	: public util::Singleton<CCacheEnvManager>
{
public:
	CCacheEnvManager() : m_crPool() {
	}

	void Init() {
		m_crPool.Init();
	}

	void Dispose(bool bUpdateToCache = true) throw() {
		m_crPool.Dispose(bUpdateToCache);
	}

	eAddRecordResult AddCacheRecord(uint32_t envId, const util::CValueStream& strKeys,
		util::CAutoPointer<ICacheValue> pValue, bool bLoadFromCache = true)
	{
		return m_crPool.AddCacheRecord(envId, strKeys, pValue, bLoadFromCache);
	}

	bool RemoveCacheRecord(uint32_t envId, bool bUpdateToCache = true)
	{
		return m_crPool.RemoveCacheRecord(envId, bUpdateToCache);
	}

	bool RemoveCacheRecord(uint32_t envId, uint64_t objectId, bool bUpdateToCache = true)
	{
		return m_crPool.RemoveCacheRecord(envId, objectId, bUpdateToCache);
	}

	bool RemoveAndDeleteFromCache(uint32_t envId)
	{
		return m_crPool.RemoveAndDeleteFromCache(envId);
	}

	bool RemoveAndDeleteFromCache(uint32_t envId, uint64_t objectId)
	{
		return m_crPool.RemoveAndDeleteFromCache(envId, objectId);
	}

	bool UpdateCacheRecord(uint32_t envId, uint64_t objectId)
	{
		return m_crPool.UpdateCacheRecord(envId, objectId);
	}

private:
	CCacheRecordPool<uint32_t> m_crPool;
};

#endif /* CACHEENVMANAGER_H */