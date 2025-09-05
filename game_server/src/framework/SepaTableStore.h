/* 
 * File:   SepaTableStore.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_6_25, 21:20
 */

#ifndef SEPATABLESTORE_H
#define SEPATABLESTORE_H

#include <vector>
#include "Common.h"
#include "SpinRWLock.h"
#include "ScopedRWLock.h"

class CSepaTableStore
{
public:
	CSepaTableStore(uint32_t uCacheId);
	~CSepaTableStore();

	static int LoadMaxTableSize(uint32_t uCacheId, const char* szSPMaxTableSize);

	bool LoadTables(long nMaxTableSize, const char* szSPExistTables);

	bool CheckAndStore(const char* szSPCreate, const char* szSPStore, uint64_t userId, const char* szContext, int nLength);

	bool Store(const char* szSPStore, uint64_t userId, const char* szContext, int nLength);

	inline uint32_t GetCacheId() const {
		return m_uCacheId;
	}

private:
	inline long GetMaxTableSize() const {
		return m_nMaxTableSize;
	}

	inline long GetTableIndex(uint64_t userId) {
		if(0 == m_nMaxTableSize) {
			return 0;
		}
		return (long)(userId % m_nMaxTableSize);
	}
private:
	inline bool Empty() {
		thd::CScopedReadLock scopedLock(m_rwLock);
		return m_tableStatus.empty();
	}
	inline void Clear() {
		thd::CScopedWriteLock scopedLock(m_rwLock);
		m_tableStatus.clear();
	}
	inline bool CheckReassign(int nIndex) {
		thd::CScopedReadLock scopedLock(m_rwLock);
		return (int)m_tableStatus.size() <= nIndex;
	}
	inline void Reassign(int nIndex) {
		thd::CScopedWriteLock scopedLock(m_rwLock);
		m_tableStatus.resize(nIndex + 1, false);
	}
	inline bool CheckNonexistent(int nIndex) {
		thd::CScopedReadLock scopedLock(m_rwLock);
		return m_tableStatus[nIndex] == false;
	}
	inline void SetExistence(int nIndex) {
		thd::CScopedWriteLock scopedLock(m_rwLock);
		m_tableStatus[nIndex] = true;
	}
private:
	typedef std::vector<bool> TABLE_STATUS_SET_T;
	TABLE_STATUS_SET_T m_tableStatus;
	thd::CSpinRWLock m_rwLock;
	uint32_t m_uCacheId;
	volatile long m_nMaxTableSize;
};

#endif  // _SEPATABLESTORE_H
