/*
 * File:   XqxTable1StoreS.h
 * Author: Jehu Shaw
 *
 * Created on 2017_7_5, 23:15
 */
#ifndef XQXTABLE1STORES_H
#define XQXTABLE1STORES_H

#include <assert.h>
#include <stdexcept>
#include <stdint.h>
#include "SpinRWLock.h"
#include "ScopedRWLock.h"

namespace util {

#define XQXTABLE1STORES_ALLOCATE_SIZE 8

#ifndef XQXTABLE_INDEX_NIL
#define XQXTABLE_INDEX_NIL -1
#endif

#ifndef XQXTABLE_MAX_SIZE
#define XQXTABLE_MAX_SIZE 0x7FFFFFFF
#endif

template<class T>
class CXqxTable1StoreS
{
private:
	class CXqxItem {
	public:
		CXqxItem(uint32_t idx)
			: m_nIndex(idx)
			, m_object()
			, m_nCount(0)
		{}

		CXqxItem(uint32_t idx, const CXqxItem& orig)
			: m_nIndex(idx)
			, m_object(orig.m_object)
			, m_nCount(orig.m_nCount)
		{}

		const T& GetValue() const {
			return m_object;
		}

		uint64_t GetKey() const {
			uint64_t nKey = m_nCount;
			nKey <<= 32;
			nKey |= m_nIndex;
			return nKey;
		}

	private:
		friend class CXqxTable1StoreS<T>;
		friend class CIteratorBase;

	private:
		T m_object;
		uint32_t m_nIndex;
		uint32_t m_nCount;
	};

public:
	typedef CXqxItem pair_t;

private:
	class CIteratorBase {
	public:
		CIteratorBase(const CXqxTable1StoreS<T>* pXqxTable, uint32_t nIndex)
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

		pair_t GetPair() const {
			thd::CScopedReadLock rLock(m_pXqxTable->m_rwLock);
			if(m_nIndex >= m_pXqxTable->m_nAllcSize) {
				throw std::out_of_range("The m_nIndex is out of range.");
			}
			uint32_t nCurIdx = m_pXqxTable->m_arrItems[m_nIndex].m_nIndex;
			if(nCurIdx >= m_pXqxTable->m_nAllcSize) {
				throw std::out_of_range("The nCurIdx is out of range.");
			}
			if(m_pXqxTable->m_arrItems[nCurIdx].m_nIndex >= m_pXqxTable->m_nCurSize) {
				return pair_t(XQXTABLE_INDEX_NIL);
			}
			return pair_t(nCurIdx, m_pXqxTable->m_arrItems[nCurIdx]);
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

		const CXqxTable1StoreS<T>* m_pXqxTable;

	private:
		volatile uint32_t m_nIndex;
	};

	class CIterator : public CIteratorBase {
	public:
		typedef CIteratorBase super;

		CIterator(const CXqxTable1StoreS<T>* pXqxTable, uint32_t nIndex)
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

		CReverseIterator(const CXqxTable1StoreS<T>* pXqxTable, uint32_t nIndex)
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

	CXqxTable1StoreS(uint32_t nAllcSize = 0)
		: m_arrItems(NULL)
		, m_nAllcSize(0)
		, m_nCurSize(0)
	{
		if(0 == nAllcSize) {
			nAllcSize = XQXTABLE1STORES_ALLOCATE_SIZE;
		}
		thd::CScopedWriteLock wLock(m_rwLock);
		AllcoTable(nAllcSize);
	}

	CXqxTable1StoreS(const CXqxTable1StoreS& orig)
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
			AllcoTable(XQXTABLE1STORES_ALLOCATE_SIZE);
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

					new(&m_arrItems[nCurIdx].m_object)T(
						orig.m_arrItems[nCurIdx].m_object);

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

	~CXqxTable1StoreS()
	{
		thd::CScopedWriteLock wLock(m_rwLock);
		DestoryTable();
	}

	bool Add(uint64_t nKey, T object)
	{
		thd::CScopedWriteLock wLock(m_rwLock);
		if(m_nCurSize >= XQXTABLE_MAX_SIZE) {
			assert(false);
			return false;
		}
		uint32_t nIndex = GetIndex(nKey);
		AllcoTable(nIndex + 1);
		if(NULL == m_arrItems) {
			assert(false);
			return false;
		}
		if(nIndex >= m_nAllcSize) {
			return false;
		}

		uint32_t nSubIdx = m_arrItems[nIndex].m_nIndex;
		if(nSubIdx < m_nCurSize) {
			uint32_t nCount = GetCount(nKey);
			CXqxItem& xqxItem = m_arrItems[nIndex];
			if(nCount != xqxItem.m_nCount) {
				InnerRemove(nIndex);
				nSubIdx = m_arrItems[nIndex].m_nIndex;
			} else {
				return false;
			}
		}

		if(nSubIdx >= m_nAllcSize) {
			nSubIdx = nIndex;
		}

		if(nSubIdx > m_nCurSize) {
			if(nIndex >= m_nCurSize) {
				if(nSubIdx != nIndex) {
					m_arrItems[nSubIdx].m_nIndex = nSubIdx;
					m_arrItems[nIndex].m_nIndex = nIndex;
				}
				nSubIdx = nIndex;	
			}
			uint32_t nLastIdx = m_arrItems[m_nCurSize].m_nIndex;
			if(nLastIdx < m_nCurSize) {
				m_arrItems[m_nCurSize].m_nIndex = m_nCurSize;
			} else {
				nLastIdx = m_nCurSize;
			}
			m_arrItems[nLastIdx].m_nIndex = nSubIdx;
			m_arrItems[nSubIdx].m_nIndex = nLastIdx;
			if(nIndex < m_nCurSize) {
				m_arrItems[nIndex].m_nIndex = nIndex;
			}
		} else if(nSubIdx == m_nCurSize) {
			if(nIndex < m_nCurSize) {
				m_arrItems[nIndex].m_nIndex = nIndex;
				m_arrItems[nSubIdx].m_nIndex = nSubIdx;
			} else {
				m_arrItems[nSubIdx].m_nIndex = nIndex;
				m_arrItems[nIndex].m_nIndex = nSubIdx;
			}
		} else {	
			assert(false);
			return false;
		}

		m_arrItems[nIndex].m_nCount = GetCount(nKey);
		new(&m_arrItems[nIndex].m_object)T(object);

		++m_nCurSize;
		return true;
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

		return InnerRemove(nIndex);
	}

	void Clear()
	{
		thd::CScopedWriteLock wLock(m_rwLock);
		InnerClear();
	}

	bool Change(uint64_t nKey, T object) 
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
			return false;
		}

		uint32_t nCount = GetCount(nKey);
		if(m_arrItems[nIndex].m_nCount != nCount) {
			return false;
		}

		if(m_arrItems[nIndex].m_nIndex >= m_nCurSize) {
			return false;
		}

		m_arrItems[nIndex].m_object = object;
		return true;
	}

	bool Has(uint64_t nKey) const
	{
		thd::CScopedReadLock rLock(m_rwLock);
		if(NULL == m_arrItems) {
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

	T Find(uint64_t nKey) const
	{
		thd::CScopedReadLock rLock(m_rwLock);
		if(NULL == m_arrItems) {
			assert(false);
			return T();
		}

		if(m_nCurSize < 1) {
			return T();
		}

		uint32_t nIndex = GetIndex(nKey);
		if(nIndex >= m_nAllcSize) {
			return T();
		}

		uint32_t nCount = GetCount(nKey);
		if(m_arrItems[nIndex].m_nCount != nCount) {
			return T();
		} 

		if(m_arrItems[nIndex].m_nIndex >= m_nCurSize) {
			return T();
		}

		return m_arrItems[nIndex].m_object;
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

	CXqxTable1StoreS& operator = (const CXqxTable1StoreS& right) {

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
			AllcoTable(XQXTABLE1STORES_ALLOCATE_SIZE);
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

					new(&m_arrItems[nCurIdx].m_object)T(
						right.m_arrItems[nCurIdx].m_object);

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

	void Print()
	{
		if(NULL == m_arrItems) {
			assert(false);
			return;
		}
		printf("/////////////////////////////////////////////////////////////////\n");
		printf("m_nCurSize = %d m_nAllcSize= %d\n", m_nCurSize, m_nAllcSize);
		for(uint32_t i = 0; i < m_nAllcSize; ++i) {
			printf("m_arrItems[%u].m_nIndex = %u \n",i, m_arrItems[i].m_nIndex);
			//if( i < 50000) {
				//assert(m_arrItems[i].m_pObject == NULL);
				//int subIdx = m_arrItems[i].m_nIndex;
				//if(i != subIdx) {
					//assert(m_arrItems[subIdx].m_nIndex == i);
					//if(i < m_nCurSize) {
					//	assert(m_arrItems[i].m_pObject == NULL);
					//} else {
					//	assert(m_arrItems[i].m_pObject != NULL);
					//}
				//} else {
					//if(i < m_nCurSize) {
					//	assert(m_arrItems[i].m_pObject != NULL);
					//} else {
					//	assert(m_arrItems[i].m_pObject == NULL);
					//}
				//}

			//}
		}
		//printf("m_nCurSize = %d m_nAllcSize= %d\n", m_nCurSize, m_nAllcSize);
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
			m_arrItems[nCurIdx].m_object.~T();
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

	inline bool InnerRemove(uint32_t nIndex) {
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
		m_arrItems[nIndex].m_object.~T();
		--m_nCurSize;
		return true;
	}

private:
	CXqxItem* m_arrItems;
	uint32_t m_nAllcSize;
	uint32_t m_nCurSize;
	thd::CSpinRWLock m_rwLock;
};

}

#endif // XQXTABLE1STORES_H
