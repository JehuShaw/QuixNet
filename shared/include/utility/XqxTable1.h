/* 
 * File:   XqxTable1.h
 * Author: Jehu Shaw
 * 
 * Created on 2017_7_5, 23:15
 */
#ifndef XQXTABLE1_H
#define XQXTABLE1_H

#include <assert.h>
#include <stdexcept>
#include <stdint.h>

namespace util {

#define XQXTABLE1_ALLOCATE_SIZE 8

#ifndef XQXTABLE_INDEX_NIL
#define XQXTABLE_INDEX_NIL -1
#endif

#ifndef XQXTABLE_MAX_SIZE
#define XQXTABLE_MAX_SIZE 0x7FFFFFFF
#endif

template<class T>
class CXqxTable1
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
		friend class CXqxTable1<T>;
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
		CIteratorBase(const CXqxTable1<T>* pXqxTable, uint32_t nIndex)
			: m_pXqxTable(pXqxTable)
			, m_nIndex(nIndex)
		{
			assert(m_pXqxTable);
		}

		bool operator==(const CIteratorBase& x) const {
			if(m_pXqxTable != x.m_pXqxTable) {
				throw std::logic_error("The table is not the same.");
			}
			return m_nIndex == x.m_nIndex;
		}

		bool operator!=(const CIteratorBase& x) const {
			if(m_pXqxTable != x.m_pXqxTable) {
				throw std::logic_error("The table is not the same.");
			}
			return m_nIndex != x.m_nIndex;
		}

		pair_t GetPair() const {
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
			if(m_nIndex >= m_pXqxTable->m_nCurSize
				&& (uint32_t)-1 != m_nIndex) {
				return;
			}
			++m_nIndex;
		}

		inline void DecreaseIndex() {
			if(m_nIndex >= m_pXqxTable->m_nCurSize) {
				return;
			}
			--m_nIndex;
		}

	private:
		const CXqxTable1<T>* m_pXqxTable;
		uint32_t m_nIndex;
	};

	class CIterator : public CIteratorBase {
	public:
		typedef CIteratorBase super;

		CIterator(const CXqxTable1<T>* pXqxTable, uint32_t nIndex)
			: CIteratorBase(pXqxTable, nIndex) {
		}

		CIterator& operator++() {
			super::IncreaseIndex();
			return *this;
		}

		CIterator operator++(int) {
			CIterator tmp(*this);
			super::IncreaseIndex();
			return tmp;
		}

		CIterator& operator--() {
			super::DecreaseIndex();
			return *this;
		}

		CIterator operator--(int) {
			CIterator tmp(*this);
			super::DecreaseIndex();
			return tmp;
		}
	};

	class CReverseIterator : public CIteratorBase {
	public:
		typedef CIteratorBase super;

		CReverseIterator(const CXqxTable1<T>* pXqxTable, uint32_t nIndex)
			: CIteratorBase(pXqxTable, nIndex) {
		}

		CReverseIterator& operator++() {
			super::DecreaseIndex();
			return *this;
		}

		CReverseIterator operator++(int) {
			CReverseIterator tmp(*this);
			super::DecreaseIndex();
			return tmp;
		}

		CReverseIterator& operator--() {
			super::IncreaseIndex();
			return *this;
		}

		CReverseIterator operator--(int) {
			CReverseIterator tmp(*this);
			super::IncreaseIndex();
			return tmp;
		}
	};

public:
	typedef CIterator iterator;
	typedef CReverseIterator reverse_iterator;

	CXqxTable1(uint32_t nAllcSize = 0)
		: m_arrItems(NULL)
		, m_nAllcSize(0)
		, m_nCurSize(0)
	{
		if(0 == nAllcSize) {
			nAllcSize = XQXTABLE1_ALLOCATE_SIZE;
		}
		AllcoTable(nAllcSize);
	}

	CXqxTable1(const CXqxTable1& orig)
	{
		if(this == &orig) {
			return;
		}

		m_arrItems = NULL;
		m_nAllcSize = 0;
		m_nCurSize = 0;

		if(NULL == orig.m_arrItems) {
			return;
		}

		if(0 == orig.m_nAllcSize) {
			AllcoTable(XQXTABLE1_ALLOCATE_SIZE);
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

	~CXqxTable1()
	{
		DestoryTable();
	}

	uint64_t Add(T object)
	{
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
		new(&xqxItem.m_object)T(object);

		++m_nCurSize;
		return xqxItem.GetKey();
	}

	uint64_t Add()
	{
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
		new(&xqxItem.m_object)T();

		++m_nCurSize;
		return xqxItem.GetKey();
	}

	bool Remove(uint64_t nKey)
	{
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
		m_arrItems[nIndex].m_object.~T();
		--m_nCurSize;
		return true;
	}

	void Clear()
	{
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

	bool Change(uint64_t nKey, T object) 
	{
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

	void Print()
	{
		if(NULL == m_arrItems) {
			assert(false);
			return;
		}
		printf("/////////////////////////////////////////////////////////////////\n");
		printf("m_nCurSize = %d m_nAllcSize= %d\n", m_nCurSize, m_nAllcSize);
		for(uint32_t i = 0; i < m_nAllcSize; ++i) {
			printf("m_arrItems[%u].m_nIndex = %u\n",i, m_arrItems[i].m_nIndex);
		}
		printf("m_nCurSize = %d m_nAllcSize= %d\n", m_nCurSize, m_nAllcSize);
	}

	uint32_t Size() const {
		return m_nCurSize;
	}

	bool Empty() const {
		return m_nCurSize == 0;
	}

	iterator Begin() const {
		return iterator(this, 0);
	}

	iterator End() const {
		return iterator(this, m_nCurSize);
	}

	reverse_iterator RBegin() const {
		return reverse_iterator(this, m_nCurSize - 1);
	}

	reverse_iterator REnd() const {
		return reverse_iterator(this, -1);
	}

	CXqxTable1& operator = (const CXqxTable1& right) {
		if(this == &right) {
			return *this;
		}

		DestoryTable();

		if(NULL == right.m_arrItems) {
			return *this;
		}

		if(0 == right.m_nAllcSize) {
			AllcoTable(XQXTABLE1_ALLOCATE_SIZE);
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

	void DestoryTable() {
		if(NULL != m_arrItems) {
			Clear();
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
};

}

#endif // XQXTABLE1_H