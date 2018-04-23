/* 
 * File:   XqxTable2.h
 * Author: Jehu Shaw
 * 
 * Created on 2017_7_5, 23:15
 */
#ifndef __XQXTABLE2_H__
#define __XQXTABLE2_H__

#include <assert.h>
#include <stdexcept>

namespace util {

#define XQXTABLE2_ALLOCATE_SIZE 8
#define XQXTABLE2_INDEX_NIL (size_t)-1

template<class T>
class CXqxTable2
{
	struct SXqxItem {
		T* pObject;
		size_t nIndex;

		SXqxItem(size_t idx)
			: pObject(NULL)
			, nIndex(idx)
		{}

		SXqxItem(size_t idx, T* pObj)
			: pObject(pObj)
			, nIndex(idx)
		{}
	};
	class CXqxTableIterator {
	public:
		CXqxTableIterator(const CXqxTable2<T>* pXqxTable, size_t nIndex)
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

		//T& operator*() const {
		//	if(m_nIndex >= m_pXqxTable->m_nAllcSize) {
		//		throw std::out_of_range("The m_nIndex is out of range.");
		//	}
		//	size_t nCurIdx = m_pXqxTable->m_arrItems[m_nIndex].nIndex;
		//	if(nCurIdx >= m_pXqxTable->m_nAllcSize) {
		//		throw std::out_of_range("The nCurIdx is out of range.");
		//	}
		//	return *m_pXqxTable->m_arrItems[nCurIdx].pObject;
		//}

		//T* operator->() const {
		//	if(m_nIndex >= m_pXqxTable->m_nAllcSize) {
		//		assert(false);
		//		return NULL;
		//	}
		//	size_t nCurIdx = m_pXqxTable->m_arrItems[m_nIndex].nIndex;
		//	if(nCurIdx >= m_pXqxTable->m_nAllcSize) {
		//		assert(false);
		//		return NULL;
		//	}
		//	return m_pXqxTable->m_arrItems[nCurIdx].pObject;
		//}

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

		SXqxItem GetValue() const {
			if(m_nIndex >= m_pXqxTable->m_nAllcSize) {
				throw std::out_of_range("The m_nIndex is out of range.");
			}
			size_t nCurIdx = m_pXqxTable->m_arrItems[m_nIndex].nIndex;
			if(nCurIdx >= m_pXqxTable->m_nAllcSize) {
				throw std::out_of_range("The nCurIdx is out of range.");
			}
			if(m_pXqxTable->m_arrItems[nCurIdx].nIndex >= m_pXqxTable->m_nCurSize) {
				return SXqxItem(XQXTABLE2_INDEX_NIL);
			}
			return SXqxItem(nCurIdx, m_pXqxTable->m_arrItems[nCurIdx].pObject);
		}

	private:
		const CXqxTable2<T>* m_pXqxTable;
		size_t m_nIndex;
		bool m_bEnd;
	};

public:
	typedef CXqxTableIterator iterator;
	typedef struct SXqxItem value_t;

public:
	CXqxTable2(size_t nAllcSize = 0)
		: m_arrItems(NULL)
		, m_nAllcSize(0)
		, m_nCurSize(0)
	{
		if(0 == nAllcSize) {
			nAllcSize = XQXTABLE2_ALLOCATE_SIZE;
		}
		AllcoTable(nAllcSize);
	}

	CXqxTable2(const CXqxTable2& orig)
	{
		if(this == &orig) {
			return;
		}

		m_arrItems = NULL;
		m_nAllcSize = 0;
		m_nCurSize = 0;

		if(0 == orig.m_nAllcSize) {
			AllcoTable(XQXTABLE2_ALLOCATE_SIZE);
		} else {
			AllcoTable(orig.m_nAllcSize);
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

					if(NULL != orig.m_arrItems[nCurIdx].pObject) {
						m_arrItems[nCurIdx].pObject = AllcoObject(
							*orig.m_arrItems[nCurIdx].pObject);
					} else {
						assert(false);
					}

					if(nCurIdx != i) {
						m_arrItems[i].nIndex = nCurIdx;
						m_arrItems[nCurIdx].nIndex = i;
					} else {
						m_arrItems[i].nIndex = i;
					}	
				}
				m_nCurSize = orig.m_nCurSize;
			}
		}
	}

	~CXqxTable2()
	{
		DestoryTable();
	}

	size_t Add(const T& object)
	{
		AllcoTable(m_nCurSize + 1);
		if(NULL == m_arrItems) {
			assert(false);
			return XQXTABLE2_INDEX_NIL;
		}
		size_t nSubIdx = m_nCurSize;
		if(nSubIdx >= m_nAllcSize) {
			return XQXTABLE2_INDEX_NIL;
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
		xqxItem.pObject = AllcoObject(object);

		++m_nCurSize;
		return nCurIdx;
	}

	value_t Add()
	{
		AllcoTable(m_nCurSize + 1);
		if(NULL == m_arrItems) {
			assert(false);
			return value_t(XQXTABLE2_INDEX_NIL);
		}
		size_t nSubIdx = m_nCurSize;
		if(nSubIdx >= m_nAllcSize) {
			return value_t(XQXTABLE2_INDEX_NIL);
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
		xqxItem.pObject = AllcoObject();

		++m_nCurSize;
		return value_t(nCurIdx, xqxItem.pObject);
	}

	bool Remove(size_t nIndex)
	{
		if(NULL == m_arrItems) {
			assert(false);
			return false;
		}
		if(nIndex >= m_nAllcSize) {
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
		DestroyObject(m_arrItems[nIndex].pObject);
		--m_nCurSize;
		return true;
	}

	void Clear() 
	{
		if(NULL == m_arrItems) {
			assert(false);
			return;
		}
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
			DestroyObject(m_arrItems[nCurIdx].pObject);
		}
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
		if(NULL != m_arrItems[nIndex].pObject) {
			if(m_arrItems[nIndex].pObject == &object) {
				return false;
			}
			*m_arrItems[nIndex].pObject = object;
		} else {
			assert(false);
		}
		return true;
	}

	value_t Find(size_t nIndex) const 
	{
		if(NULL == m_arrItems) {
			assert(false);
			return value_t(XQXTABLE2_INDEX_NIL);
		}
		if(nIndex >= m_nAllcSize) {
			return value_t(XQXTABLE2_INDEX_NIL);
		}
		if(m_arrItems[nIndex].nIndex >= m_nCurSize) {
			return value_t(XQXTABLE2_INDEX_NIL);
		}

		return value_t(nIndex, m_arrItems[nIndex].pObject);
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

	CXqxTable2& operator = (const CXqxTable2& right) {
		if(this == &right) {
			return *this;
		}

		DestoryTable();

		if(0 == right.m_nAllcSize) {
			AllcoTable(XQXTABLE2_ALLOCATE_SIZE);
		} else {
			AllcoTable(right.m_nAllcSize);
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

					if(NULL != right.m_arrItems[nCurIdx].pObject) {
						m_arrItems[nCurIdx].pObject = AllcoObject(
							*right.m_arrItems[nCurIdx].pObject);
					} else {
						assert(false);
					}

					if(nCurIdx != i) {
						m_arrItems[i].nIndex = nCurIdx;
						m_arrItems[nCurIdx].nIndex = i;
					} else {
						m_arrItems[i].nIndex = i;
					}	
				}
				m_nCurSize = right.m_nCurSize;
			}
		}
		return *this;
	}

private:
	T* AllcoObject(const T& object) {
		return new T(object);
	}

	T* AllcoObject() {
		return new T;
	}

	void DestroyObject(T*& pObject) {
		delete pObject;
		pObject = NULL;
	}

	void AllcoTable(size_t nNeedSize) {
		if(NULL == m_arrItems) {
			size_t nByteSize = sizeof(struct SXqxItem) * nNeedSize;
			m_arrItems = (struct SXqxItem*)malloc(nByteSize);
			m_nAllcSize = nNeedSize;
			if(NULL != m_arrItems) {
				for(size_t i = 0; i < m_nAllcSize; ++i) {
					m_arrItems[i].nIndex = XQXTABLE2_INDEX_NIL;
				}
			} else {
				assert(m_arrItems);
			}
		} else {
			if(nNeedSize > m_nAllcSize) {
				size_t nOldSize = m_nAllcSize;
				size_t nNewSize = m_nAllcSize * 2;
				if(nNeedSize > nNewSize) {
					nNewSize = nNeedSize;
				}
				size_t nByteSize = sizeof(struct SXqxItem) * nNewSize;
				struct SXqxItem* pNew = (struct SXqxItem*)realloc(m_arrItems, nByteSize);
				if(NULL == pNew) {
					if(nNeedSize < nNewSize) {
						nByteSize = sizeof(struct SXqxItem) * nNeedSize;
						pNew = (struct SXqxItem*)realloc(m_arrItems, nByteSize);
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
					for(size_t i = nOldSize; i < m_nAllcSize; ++i) {
						m_arrItems[i].nIndex = XQXTABLE2_INDEX_NIL;
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

private:
	struct SXqxItem* m_arrItems;
	size_t m_nAllcSize;
	size_t m_nCurSize;
};

}

#endif // __XQXTABLE2_H__