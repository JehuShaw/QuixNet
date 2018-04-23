/* 
 * File:   XqxTable0.h
 * Author: Jehu Shaw
 * 
 * Created on 2017_7_5, 23:15
 */
#ifndef __XQXTABLE0_H__
#define __XQXTABLE0_H__

#include <assert.h>
#include <stdexcept>

namespace util {

#define XQXTABLE0_INDEX_NIL (size_t)-1

template<class T>
class CXqxTable0
{
	struct SXqxItem {
		T object;
		size_t nIndex;
	};
	struct SXqxResult {
		T* pObject;
		size_t nIndex;

		SXqxResult(size_t idx)
			: pObject(NULL)
			, nIndex(idx)
		{}

		SXqxResult(size_t idx, T* pObj)
			: pObject(pObj)
			, nIndex(idx)
		{}
	};
	class CXqxTableIterator {
	public:
		CXqxTableIterator(const CXqxTable0<T>* pXqxTable, size_t nIndex)
			: m_pXqxTable(pXqxTable)
			, m_nIndex(nIndex)
			, m_bEnd(false)
		{
			assert(m_pXqxTable);
			if(pXqxTable && pXqxTable->m_nCurSize == nIndex) {
				m_bEnd = true;
			}
		}

		bool operator==(const CXqxTableIterator& x) const {
			if(m_pXqxTable != x.m_pXqxTable) {
				throw std::logic_error("The table is not the same.");
			}
			if(m_bEnd && m_pXqxTable->m_nCurSize != m_nIndex) {
				throw std::logic_error("The end iterator of this is invalid !");
			}
			if(x.m_bEnd && x.m_pXqxTable->m_nCurSize != x.m_nIndex) {
				throw std::logic_error("The end iterator of x is invalid !");
			}
			return m_nIndex == x.m_nIndex;
		}

		bool operator!=(const CXqxTableIterator& x) const {
			if(m_pXqxTable != x.m_pXqxTable) {
				throw std::logic_error("The table is not the same.");
			}
			if(m_bEnd && m_pXqxTable->m_nCurSize != m_nIndex) {
				throw std::logic_error("The end iterator of this is invalid !");
			}
			if(x.m_bEnd && x.m_pXqxTable->m_nCurSize != x.m_nIndex) {
				throw std::logic_error("The end iterator of x is invalid !");
			}
			return m_nIndex != x.m_nIndex;
		}

		CXqxTableIterator& operator++() {
			if(m_nIndex >= m_pXqxTable->m_nCurSize) {
				m_bEnd = true;
				m_nIndex = m_pXqxTable->m_nCurSize;
				return *this;
			}
			if(++m_nIndex == m_pXqxTable->m_nCurSize) {
				m_bEnd = true;
			}
			return *this;
		}

		CXqxTableIterator operator++(int) {
			CXqxTableIterator tmp(*this);
			++*this;
			return tmp;
		}

		CXqxTableIterator& operator--() {
			if(0 == m_nIndex) {
				m_bEnd = false;
				return *this;
			}
			if(m_nIndex-- == m_pXqxTable->m_nCurSize) {
				m_bEnd = false;
			}
			return *this;
		}

		CXqxTableIterator operator--(int) {
			CXqxTableIterator tmp(*this);
			--*this;
			return tmp;
		}

		SXqxResult GetValue() const {
			if(m_nIndex >= m_pXqxTable->m_nAllcSize) {
				throw std::out_of_range("The m_nIndex is out of range.");
			}
			size_t nCurIdx = m_pXqxTable->m_arrItems[m_nIndex].nIndex;
			if(nCurIdx >= m_pXqxTable->m_nAllcSize) {
				throw std::out_of_range("The nCurIdx is out of range.");
			}
			if(m_pXqxTable->m_arrItems[nCurIdx].nIndex >= m_pXqxTable->m_nCurSize) {
				return SXqxResult(XQXTABLE0_INDEX_NIL);
			}

			return SXqxResult(nCurIdx, &m_pXqxTable->m_arrItems[nCurIdx].object);
		}

	private:
		const CXqxTable0<T>* m_pXqxTable;
		size_t m_nIndex;
		bool m_bEnd;
	};

public:
	typedef CXqxTableIterator iterator;
	typedef struct SXqxResult value_t;
	
public:
	CXqxTable0(size_t nAllcSize)
		: m_arrItems(NULL)
		, m_nAllcSize(0)
		, m_nCurSize(0)
	{
		if(0 == nAllcSize) {
			throw std::logic_error("The allocation size can't be zero.");
		} else {
			size_t nByteSize = sizeof(struct SXqxItem) * nAllcSize;
			m_arrItems = (struct SXqxItem*)malloc(nByteSize);
			m_nAllcSize = nAllcSize;
			if(NULL != m_arrItems) {
				for(size_t i = 0; i < m_nAllcSize; ++i) {
					m_arrItems[i].nIndex = XQXTABLE0_INDEX_NIL;
					new(&m_arrItems[i].object)T;
				}
			} else {
				assert(m_arrItems);
			}
		}
	}

	CXqxTable0(const CXqxTable0& orig)
	{
		if(this == &orig) {
			return;
		}
		m_arrItems = NULL;
		m_nAllcSize = 0;
		m_nCurSize = 0;

		if(0 == orig.m_nAllcSize) {
			throw std::logic_error("The allocation size can't be zero.");
		} else {
			size_t nByteSize = sizeof(struct SXqxItem) * orig.m_nAllcSize;
			m_arrItems = (struct SXqxItem*)malloc(nByteSize);
			m_nAllcSize = orig.m_nAllcSize;
			assert(m_arrItems);
		}

		if(NULL != m_arrItems) {
			if(orig.m_nCurSize > m_nAllcSize) {
				assert(false);
			} else {
				for(size_t i = 0; i < orig.m_nCurSize; ++i) {
					size_t nCurIdx = orig.m_arrItems[i].nIndex;
					if(nCurIdx >= m_nAllcSize) {
						assert(false);
						continue;
					}

					new(&m_arrItems[nCurIdx].object)T(
						orig.m_arrItems[nCurIdx].object);

					if(nCurIdx != i) {
						m_arrItems[i].nIndex = nCurIdx;
						m_arrItems[nCurIdx].nIndex = i;
					} else {
						m_arrItems[i].nIndex = i;
					}	
				}
				m_nCurSize = orig.m_nCurSize;
				for(size_t j = orig.m_nCurSize; j < m_nAllcSize; ++j) {
					m_arrItems[j].nIndex = XQXTABLE0_INDEX_NIL;;
					new(&m_arrItems[j].object)T;
				}
			}
		}
	}

	~CXqxTable0()
	{
		if(NULL != m_arrItems) {
			InnerClear();
			free(m_arrItems);
			m_arrItems = NULL;
		}
		m_nAllcSize = 0;
	}

	value_t Add()
	{
		if(NULL == m_arrItems) {
			assert(false);
			return value_t(XQXTABLE0_INDEX_NIL);
		}
		size_t nSubIdx = m_nCurSize;
		if(nSubIdx >= m_nAllcSize) {
			return value_t(XQXTABLE0_INDEX_NIL);
		}
		size_t nCurIdx = nSubIdx;
		if(m_nCurSize > 0) {
			size_t nPreIdx = m_arrItems[nCurIdx].nIndex;
			if(nCurIdx > nPreIdx) {
				assert(nCurIdx == m_arrItems[nPreIdx].nIndex);
				nCurIdx = nPreIdx;
				m_arrItems[nSubIdx].nIndex = nSubIdx;
			}
		}
		struct SXqxItem& xqxItem = m_arrItems[nCurIdx];
		xqxItem.nIndex = nCurIdx;

		++m_nCurSize;
		return value_t(nCurIdx, &xqxItem.object);
	}

	bool Remove(size_t nIndex)
	{
		if(nIndex >= m_nAllcSize) {
			assert(false);
			return false;
		}
		if(NULL == m_arrItems) {
			assert(false);
			return false;
		}

		size_t nCurIdx = nIndex;
		size_t nSubIdx = m_arrItems[nCurIdx].nIndex;
		size_t nLastIdx = m_nCurSize - 1;
		if(nSubIdx > nLastIdx) {
			return false;
		}

		if(nCurIdx > nLastIdx) {
			m_arrItems[nCurIdx].nIndex = nCurIdx;
			m_arrItems[nSubIdx].nIndex = nSubIdx;
			nCurIdx = nSubIdx;	
		}

		size_t nSubIdx2 = m_arrItems[nLastIdx].nIndex;
		if(nSubIdx2 != nLastIdx) {
			m_arrItems[nLastIdx].nIndex = nLastIdx;
			m_arrItems[nSubIdx2].nIndex = nCurIdx;
			m_arrItems[nCurIdx].nIndex = nSubIdx2;
		} else if(nCurIdx != nLastIdx) {
			m_arrItems[nCurIdx].nIndex = nLastIdx;
			m_arrItems[nLastIdx].nIndex = nCurIdx;
		}
		--m_nCurSize;
		return true;
	}

	bool Change(size_t nIndex, const T& object) 
	{
		if(NULL == m_arrItems) {
			assert(false);
			return false;
		}
		if(nIndex >= m_nAllcSize) {
			return false;
		}
		if(m_arrItems[nIndex].nIndex >= m_nCurSize) {
			return false;
		}
		m_arrItems[nIndex].object = object;
		return true;
	}

	value_t Find(size_t nIndex) const 
	{
		if(NULL == m_arrItems) {
			assert(false);
			return value_t(XQXTABLE0_INDEX_NIL);
		}
		if(nIndex >= m_nAllcSize) {
			return value_t(XQXTABLE0_INDEX_NIL);
		}
		if(m_arrItems[nIndex].nIndex >= m_nCurSize) {
			return value_t(XQXTABLE0_INDEX_NIL);
		}
		return value_t(nIndex, &m_arrItems[nIndex].object);
	}

	size_t GetIndexByPtr(const T* pObject) const
	{
		struct SXqxItem* pValue = NULL;
		pValue = (struct SXqxItem*)((intptr_t)
			pObject - (intptr_t)&pValue->object);
		size_t nIdx = pValue - m_arrItems;
		if(nIdx >= m_nAllcSize) {
			return XQXTABLE0_INDEX_NIL;
		}
		if((m_arrItems + nIdx) != pValue) {
			return XQXTABLE0_INDEX_NIL;
		}
		return nIdx;
	}

	void Print()
	{
		if(NULL == m_arrItems) {
			assert(false);
			return;
		}
		printf("/////////////////////////////////////////////////////////////////\n");
		printf("m_nCurSize = %d m_nAllcSize= %d\n", m_nCurSize, m_nAllcSize);
		for(size_t i = 0; i < m_nAllcSize; ++i) {
			printf("m_arrItems[%u].index = %u\n",i, m_arrItems[i].nIndex);
		}
		printf("m_nCurSize = %d m_nAllcSize= %d\n", m_nCurSize, m_nAllcSize);
	}

	size_t Size() const {
		return m_nCurSize;
	}

	bool Empty() const {
		return m_nCurSize == 0;
	}

	iterator Begin() {
		return iterator(this, 0);
	}

	iterator End() {
		return iterator(this, m_nCurSize);
	}

	CXqxTable0& operator = (const CXqxTable0& right) {
		if(this == &right) {
			return *this;
		}

		if(NULL != m_arrItems) {
			InnerClear();
			free(m_arrItems);
			m_arrItems = NULL;
		}
		m_nAllcSize = 0;

		if(0 == right.m_nAllcSize) {
			throw std::logic_error("The allocation size can't be zero.");
		} else {
			size_t nByteSize = sizeof(struct SXqxItem) * right.m_nAllcSize;
			m_arrItems = (struct SXqxItem*)malloc(nByteSize);
			m_nAllcSize = right.m_nAllcSize;
			assert(m_arrItems);
		}

		if(NULL != m_arrItems) {
			if(right.m_nCurSize > m_nAllcSize) {
				assert(false);
			} else {
				for(size_t i = 0; i < right.m_nCurSize; ++i) {
					size_t nCurIdx = right.m_arrItems[i].nIndex;
					if(nCurIdx >= m_nAllcSize) {
						assert(false);
						continue;
					}

					new(&m_arrItems[nCurIdx].object)T(
						right.m_arrItems[nCurIdx].object);

					if(nCurIdx != i) {
						m_arrItems[i].nIndex = nCurIdx;
						m_arrItems[nCurIdx].nIndex = i;
					} else {
						m_arrItems[i].nIndex = i;
					}	
				}
				m_nCurSize = right.m_nCurSize;
				for(size_t j = right.m_nCurSize; j < m_nAllcSize; ++j) {
					m_arrItems[j].nIndex = XQXTABLE0_INDEX_NIL;;
					new(&m_arrItems[j].object)T;
				}
			}
		}
		return *this;
	}

private:
	void InnerClear() {
		size_t nSize = m_nCurSize;
		m_nCurSize = 0;
		for(size_t i = 0; i < nSize; ++i) {
			size_t nCurIdx = m_arrItems[i].nIndex;
			if(nCurIdx >= m_nAllcSize) {
				assert(false);
				continue;
			}
			if(nCurIdx != i) {
				m_arrItems[i].nIndex = i;
				m_arrItems[nCurIdx].nIndex = nCurIdx;
			}
			m_arrItems[nCurIdx].object.~T();
		}
		for(size_t j = nSize; j < m_nAllcSize; ++j) {
			m_arrItems[j].object.~T();
		}
	}

private:
	struct SXqxItem* m_arrItems;
	size_t m_nAllcSize;
	size_t m_nCurSize;
};

}

#endif // __XQXTABLE0_H__