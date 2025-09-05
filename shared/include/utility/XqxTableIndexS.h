/* 
 * File:   XqxTableIndexS.h
 * Author: Jehu Shaw
 * 
 * Created on 2017_7_5, 23:15
 */
#ifndef XQXTABLEINDEXS_H
#define XQXTABLEINDEXS_H

#include <assert.h>
#include <stdexcept>
#include <stdint.h>
#include "SpinRWLock.h"
#include "ScopedRWLock.h"

namespace util {

#define XQXTABLEINDEXS_ALLOCATE_SIZE 8

#ifndef XQXTABLE_INDEX_NIL
#define XQXTABLE_INDEX_NIL -1
#endif

#ifndef XQXTABLE_MAX_SIZE
#define XQXTABLE_MAX_SIZE 0x7FFFFFFF
#endif

class CXqxTableIndexS
{
private:
	class CXqxItem {
	public:
		CXqxItem(uint32_t idx)
		: m_nIndex(idx)
		, m_nCount(0)
		{}

		CXqxItem(uint32_t idx, const CXqxItem& orig)
		: m_nIndex(idx)
		, m_nCount(orig.m_nCount)
		{}


		uint64_t GetKey() const {
			uint64_t nKey = m_nCount;
			nKey <<= 32;
			nKey |= m_nIndex;
			return nKey;
		}

	private:
		friend class CXqxTableIndexS;
		friend class CIteratorBase;

	private:
		uint32_t m_nIndex;
		uint32_t m_nCount;
	};

private:
	class CIteratorBase {
	public:
		CIteratorBase(const CXqxTableIndexS* pXqxTable, uint32_t nIndex)
			: m_pXqxTable(pXqxTable)
			, m_nIndex(nIndex)
		{
			assert(m_pXqxTable);
		}

		bool operator==(const CIteratorBase& x) const {
			thd::CScopedReadLock rrLock(m_pXqxTable->m_rwLock);
			thd::CScopedReadLock rlLock(x.m_pXqxTable->m_rwLock);

			if(m_pXqxTable != x.m_pXqxTable) {
				throw std::logic_error("The table is not the same.");
			}
			return m_nIndex == x.m_nIndex;
		}

		bool operator!=(const CIteratorBase& x) const {
			thd::CScopedReadLock rrLock(m_pXqxTable->m_rwLock);
			thd::CScopedReadLock rlLock(x.m_pXqxTable->m_rwLock);

			if(m_pXqxTable != x.m_pXqxTable) {
				throw std::logic_error("The table is not the same.");
			}
			return m_nIndex != x.m_nIndex;
		}


		uint64_t GetKey() const {
			thd::CScopedReadLock rLock(m_pXqxTable->m_rwLock);
			return m_pXqxTable->m_arrItems[m_nIndex].GetKey();
		}

	protected:
		inline void IncreaseIndex() {
			uint32_t nIndex;
			uint32_t nSize;
			do {
				nIndex = (uint32_t)m_nIndex;
				nSize = m_pXqxTable->m_nCurSize;
				if(nIndex >= nSize && (uint32_t)-1 != nIndex) {
					return;
				}
			} while (atomic_cmpxchg(&m_nIndex, nIndex, nIndex + 1) != nIndex);
		}

		inline void DecreaseIndex() {
			uint32_t nIndex;
			uint32_t nSize;
			do {
				nIndex = (uint32_t)m_nIndex;
				nSize = m_pXqxTable->m_nCurSize;
				if(nIndex >= nSize) {
					return;
				}
			} while (atomic_cmpxchg(&m_nIndex, nIndex, nIndex - 1) != nIndex);
		}

		const CXqxTableIndexS* m_pXqxTable;

	private:
		volatile uint32_t m_nIndex;
	};

	class CIterator : public CIteratorBase {
	public:
		typedef CIteratorBase super;

		CIterator(const CXqxTableIndexS* pXqxTable, uint32_t nIndex)
			: CIteratorBase(pXqxTable, nIndex) {
		}

		CIterator& operator++() {
			thd::CScopedReadLock rLock(super::m_pXqxTable->m_rwLock);
			super::IncreaseIndex();
			return *this;
		}

		CIterator operator++(int) {
			thd::CScopedReadLock rLock(super::m_pXqxTable->m_rwLock);
			CIterator tmp(*this);
			super::IncreaseIndex();
			return tmp;
		}

		CIterator& operator--() {
			thd::CScopedReadLock rLock(super::m_pXqxTable->m_rwLock);
			super::DecreaseIndex();
			return *this;
		}

		CIterator operator--(int) {
			thd::CScopedReadLock rLock(super::m_pXqxTable->m_rwLock);
			CIterator tmp(*this);
			super::DecreaseIndex();
			return tmp;
		}
	};

	class CReverseIterator : public CIteratorBase {
	public:
		typedef CIteratorBase super;

		CReverseIterator(const CXqxTableIndexS* pXqxTable, uint32_t nIndex)
			: CIteratorBase(pXqxTable, nIndex) {
		}

		CReverseIterator& operator++() {
			thd::CScopedReadLock rLock(super::m_pXqxTable->m_rwLock);
			super::DecreaseIndex();
			return *this;
		}

		CReverseIterator operator++(int) {
			thd::CScopedReadLock rLock(super::m_pXqxTable->m_rwLock);
			CReverseIterator tmp(*this);
			super::DecreaseIndex();
			return tmp;
		}

		CReverseIterator& operator--() {
			thd::CScopedReadLock rLock(super::m_pXqxTable->m_rwLock);
			super::IncreaseIndex();
			return *this;
		}

		CReverseIterator operator--(int) {
			thd::CScopedReadLock rLock(super::m_pXqxTable->m_rwLock);
			CReverseIterator tmp(*this);
			super::IncreaseIndex();
			return tmp;
		}
	};

public:
	typedef CIterator iterator;
	typedef CReverseIterator reverse_iterator;

	CXqxTableIndexS(uint32_t nAllcSize = 0)
		: m_arrItems(NULL)
		, m_nAllcSize(0)
		, m_nCurSize(0)
	{
		if(0 == nAllcSize) {
			nAllcSize = XQXTABLEINDEXS_ALLOCATE_SIZE;
		}
		thd::CScopedWriteLock wLock(m_rwLock);
		AllcoTable(nAllcSize);
	}

	CXqxTableIndexS(const CXqxTableIndexS& orig)
	{
		if(this == &orig) {
			return;
		}

		thd::CScopedReadLock rLock(orig.m_rwLock);
		thd::CScopedWriteLock wLock(m_rwLock);

		m_arrItems = NULL;
		m_nAllcSize = 0;
		m_nCurSize = 0;

		if(NULL == orig.m_arrItems) {
			return;
		}

		if(0 == orig.m_nAllcSize) {
			AllcoTable(XQXTABLEINDEXS_ALLOCATE_SIZE);
		} else {
			AllcoTable(orig.m_nAllcSize);
		}

		if(NULL != m_arrItems) {
			if(orig.m_nCurSize > m_nAllcSize) {
				assert(false);
			} else {
				for(uint32_t i = 0; i < orig.m_nCurSize; ++i) {
					uint32_t nCurIdx = orig.m_arrItems[i].m_nIndex;
					if(nCurIdx >= m_nAllcSize) {
						assert(false);
						continue;
					}

					if(nCurIdx != i) {
						m_arrItems[i].m_nIndex = nCurIdx;
						m_arrItems[nCurIdx].m_nIndex = i;
					} else {
						m_arrItems[i].m_nIndex = i;
					}

					m_arrItems[nCurIdx].m_nCount = orig.m_arrItems[nCurIdx].m_nCount;
				}
				m_nCurSize = orig.m_nCurSize;
			}
		}
	}

	~CXqxTableIndexS()
	{
		thd::CScopedWriteLock wLock(m_rwLock);
		DestoryTable();
	}

	uint64_t Add()
	{
		thd::CScopedWriteLock wLock(m_rwLock);
		if(m_nCurSize >= XQXTABLE_MAX_SIZE) {
			assert(false);
			return XQXTABLE_INDEX_NIL;
		}
		AllcoTable(m_nCurSize + 1);
		if(NULL == m_arrItems) {
			assert(false);
			return XQXTABLE_INDEX_NIL;
		}
		uint32_t nSubIdx = m_nCurSize;
		if(nSubIdx >= m_nAllcSize) {
			return XQXTABLE_INDEX_NIL;
		}
		uint32_t nCurIdx = nSubIdx;
		if(m_nCurSize > 0) {
			uint32_t nPreIdx = m_arrItems[nCurIdx].m_nIndex;
			if(nCurIdx > nPreIdx) {
				assert(nCurIdx == m_arrItems[nPreIdx].m_nIndex);
				nCurIdx = nPreIdx;
				m_arrItems[nSubIdx].m_nIndex = nSubIdx;
			}
		}
		CXqxItem& xqxItem = m_arrItems[nCurIdx];
		xqxItem.m_nIndex = nCurIdx;
		++xqxItem.m_nCount;

		++m_nCurSize;
		return xqxItem.GetKey();
	}

	bool Remove(uint64_t nKey)
	{
		thd::CScopedWriteLock wLock(m_rwLock);
		if(NULL == m_arrItems) {
			assert(false);
			return false;
		}

		if(m_nCurSize < 1) {
			return false;
		}

		uint32_t nIndex = GetIndex(nKey);
		if(nIndex >= m_nAllcSize) {
			assert(false);
			return false;
		}

		uint32_t nCount = GetCount(nKey);
		if(m_arrItems[nIndex].m_nCount != nCount) {
			return false;
		}

		uint32_t nCurIdx = nIndex;
		uint32_t nSubIdx = m_arrItems[nCurIdx].m_nIndex;
		uint32_t nLastIdx = m_nCurSize - 1;
		if(nSubIdx > nLastIdx) {
			return false;
		}

		if(nCurIdx > nLastIdx) {
			m_arrItems[nCurIdx].m_nIndex = nCurIdx;
			m_arrItems[nSubIdx].m_nIndex = nSubIdx;
			nCurIdx = nSubIdx;
		}

		uint32_t nSubIdx2 = m_arrItems[nLastIdx].m_nIndex;
		if(nSubIdx2 != nLastIdx) {
			m_arrItems[nLastIdx].m_nIndex = nLastIdx;
			m_arrItems[nSubIdx2].m_nIndex = nCurIdx;
			m_arrItems[nCurIdx].m_nIndex = nSubIdx2;
		} else if(nCurIdx != nLastIdx) {
			m_arrItems[nCurIdx].m_nIndex = nLastIdx;
			m_arrItems[nLastIdx].m_nIndex = nCurIdx;
		}
		--m_nCurSize;
		return true;
	}

	void Clear()
	{
		thd::CScopedWriteLock wLock(m_rwLock);
		InnerClear();
	}

	bool Find(uint64_t nKey) const 
	{
		thd::CScopedReadLock rLock(m_rwLock);
		if(NULL == m_arrItems) {
			assert(false);
			return false;
		}

		if(m_nCurSize < 1) {
			return false;
		}

		uint32_t nIndex = GetIndex(nKey);
		if(nIndex >= m_nAllcSize) {
			return false;
		}

		uint32_t nCount = GetCount(nKey);
		if(m_arrItems[nIndex].m_nCount != nCount) {
			return false;
		}

		if(m_arrItems[nIndex].m_nIndex >= m_nCurSize) {
			return false;
		}

		return true;

	}

	uint32_t Size() const {
		thd::CScopedReadLock rLock(m_rwLock);
		return m_nCurSize;
	}

	bool Empty() const {
		thd::CScopedReadLock rLock(m_rwLock);
		return m_nCurSize == 0;
	}

	iterator Begin() const {
		thd::CScopedReadLock rLock(m_rwLock);
		return iterator(this, 0);
	}

	iterator End() const {
		thd::CScopedReadLock rLock(m_rwLock);
		return iterator(this, m_nCurSize);
	}

	reverse_iterator RBegin() const {
		thd::CScopedReadLock rLock(m_rwLock);
		return reverse_iterator(this, m_nCurSize - 1);
	}

	reverse_iterator REnd() const {
		thd::CScopedReadLock rLock(m_rwLock);
		return reverse_iterator(this, -1);
	}

	CXqxTableIndexS& operator = (const CXqxTableIndexS& right) {
		if(this == &right) {
			return *this;
		}

		thd::CScopedReadLock rLock(right.m_rwLock);
		thd::CScopedWriteLock wLock(m_rwLock);

		DestoryTable();

		if(NULL == right.m_arrItems) {
			return *this;
		}

		if(0 == right.m_nAllcSize) {
			AllcoTable(XQXTABLEINDEXS_ALLOCATE_SIZE);
		} else {
			AllcoTable(right.m_nAllcSize);
		}

		if(NULL != m_arrItems) {
			if(right.m_nCurSize > m_nAllcSize) {
				assert(false);
			} else {
				for(uint32_t i = 0; i < right.m_nCurSize; ++i) {
					uint32_t nCurIdx = right.m_arrItems[i].m_nIndex;
					if(nCurIdx >= m_nAllcSize) {
						assert(false);
						continue;
					}

					if(nCurIdx != i) {
						m_arrItems[i].m_nIndex = nCurIdx;
						m_arrItems[nCurIdx].m_nIndex = i;
					} else {
						m_arrItems[i].m_nIndex = i;
					}

					m_arrItems[nCurIdx].m_nCount = right.m_arrItems[nCurIdx].m_nCount;
				}
				m_nCurSize = right.m_nCurSize;
			}
		}
		return *this;
	}

private:
	void AllcoTable(uint32_t nNeedSize) {
		if(NULL == m_arrItems) {
			uint32_t nByteSize = sizeof(CXqxItem) * nNeedSize;
			m_arrItems = (CXqxItem*)malloc(nByteSize);
			m_nAllcSize = nNeedSize;
			if(NULL != m_arrItems) {
				for(uint32_t i = 0; i < m_nAllcSize; ++i) {
					m_arrItems[i].m_nIndex = XQXTABLE_INDEX_NIL;
					m_arrItems[i].m_nCount = 0;
				}
			} else {
				assert(m_arrItems);
			}
		} else {
			if(nNeedSize > m_nAllcSize) {
				uint32_t nOldSize = m_nAllcSize;
				uint32_t nNewSize = m_nAllcSize * 2;
				if(nNeedSize > nNewSize) {
					nNewSize = nNeedSize;
				}
				uint32_t nByteSize = sizeof(CXqxItem) * nNewSize;
				CXqxItem* pNew = (CXqxItem*)realloc(m_arrItems, nByteSize);
				if(NULL == pNew) {
					if(nNeedSize < nNewSize) {
						nByteSize = sizeof(CXqxItem) * nNeedSize;
						pNew = (CXqxItem*)realloc(m_arrItems, nByteSize);
						if(NULL != pNew) {
							m_arrItems = pNew;
							m_nAllcSize = nNeedSize;
						}
					}
					assert(pNew);
				} else {
					m_arrItems = pNew;
					m_nAllcSize = nNewSize;
				}
				if(NULL != m_arrItems) {
					for(uint32_t i = nOldSize; i < m_nAllcSize; ++i) {
						m_arrItems[i].m_nIndex = XQXTABLE_INDEX_NIL;
						m_arrItems[i].m_nCount = 0;
					}
				} else {
					assert(m_arrItems);
				}
			}
		}
	}

	void InnerClear() {
		if(NULL == m_arrItems) {
			assert(false);
			return;
		}
		uint32_t nSize = m_nCurSize;
		m_nCurSize = 0;
		for(uint32_t i = 0; i < nSize; ++i) {
			uint32_t nCurIdx = m_arrItems[i].m_nIndex;
			if(nCurIdx >= m_nAllcSize) {
				assert(false);
				continue;
			}
			if(nCurIdx != i) {
				m_arrItems[i].m_nIndex = i;
				m_arrItems[nCurIdx].m_nIndex = nCurIdx;
			}
		}
	}

	void DestoryTable() {
		if(NULL != m_arrItems) {
			InnerClear();
			free(m_arrItems);
			m_arrItems = NULL;
		}
		m_nAllcSize = 0;
	}

	inline static uint32_t GetCount(uint64_t nKey) {
		return nKey >> 32;
	}

	inline static uint32_t GetIndex(uint64_t nKey) {
		return nKey & 0xFFFFFFFF;
	}

private:
	CXqxItem* m_arrItems;
	uint32_t m_nAllcSize;
	uint32_t m_nCurSize;
	thd::CSpinRWLock m_rwLock;
};

}

#endif // XQXTABLEINDEXS_H