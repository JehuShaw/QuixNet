/* 
 * File:   XqxTable0.h
 * Author: Jehu Shaw
 * 
 * Created on 2017_7_5, 23:15
 */
#ifndef XQXTABLE0_H
#define XQXTABLE0_H

#include <assert.h>
#include <stdexcept>
#include <stdint.h>

namespace util {

#ifndef XQXTABLE_INDEX_NIL
#define XQXTABLE_INDEX_NIL -1
#endif

#ifndef XQXTABLE_MAX_SIZE
#define XQXTABLE_MAX_SIZE 0x7FFFFFFF
#endif

template<class T>
class CXqxTable0
{
private:
	struct SXqxItem {
		T m_object;
		uint32_t m_nIndex;
	};
	class CXqxResult {
	public:
		CXqxResult(uint32_t idx)
			: m_pObject(NULL)
			, m_nIndex(idx)
		{}

		CXqxResult(uint32_t idx, T* pObj)
			: m_pObject(pObj)
			, m_nIndex(idx)
		{}

		uint32_t GetIndex() const {
			return m_nIndex;
		}

		T* GetValue() const {
			return m_pObject;
		}

	private:
		T* m_pObject;
		uint32_t m_nIndex;
		uint32_t m_nCount;
	};

public:
	typedef CXqxResult pair_t;

private:
	class CIteratorBase {
	public:
		CIteratorBase(const CXqxTable0<T>* pXqxTable, uint32_t nIndex)
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
			return pair_t(nCurIdx, &m_pXqxTable->m_arrItems[nCurIdx].m_object);
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
		const CXqxTable0<T>* m_pXqxTable;
		uint32_t m_nIndex;
	};

	class CIterator : public CIteratorBase {
	public:
		typedef CIteratorBase super;

		CIterator(const CXqxTable0<T>* pXqxTable, uint32_t nIndex)
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

		CReverseIterator(const CXqxTable0<T>* pXqxTable, uint32_t nIndex)
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

	CXqxTable0(uint32_t nAllcSize)
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
				for(uint32_t i = 0; i < m_nAllcSize; ++i) {
					m_arrItems[i].m_nIndex = XQXTABLE_INDEX_NIL;
					new(&m_arrItems[i].m_object)T;
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

		if(NULL == orig.m_arrItems) {
			return;
		}

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
				}
				m_nCurSize = orig.m_nCurSize;
				for(uint32_t j = orig.m_nCurSize; j < m_nAllcSize; ++j) {
					m_arrItems[j].m_nIndex = XQXTABLE_INDEX_NIL;
					new(&m_arrItems[j].m_object)T;
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

	T* AddSetIndex(const T& object, uint32_t& outIndex)
	{
		if(m_nCurSize >= XQXTABLE_MAX_SIZE) {
			assert(false);
			return NULL;
		}
		if(NULL == m_arrItems) {
			assert(false);
			return NULL;
		}
		uint32_t nSubIdx = m_nCurSize;
		if(nSubIdx >= m_nAllcSize) {
			return NULL;
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
		struct SXqxItem& xqxItem = m_arrItems[nCurIdx];
		xqxItem.m_nIndex = nCurIdx;
		new(&xqxItem.m_object)T(object, nCurIdx);

		++m_nCurSize;

		intptr_t nOffset = (intptr_t)&outIndex - (intptr_t)&object;
		assert(nOffset >= 0 && nOffset < sizeof(T));
		uint32_t* pOutIndex = (uint32_t*)((intptr_t)&xqxItem.m_object + nOffset);
		*pOutIndex = nCurIdx;
		outIndex = nCurIdx;
		return &xqxItem.m_object;
	}

	bool Remove(uint32_t nIndex, void(T::*pClearFun)() = NULL)
	{
		if(nIndex >= m_nAllcSize) {
			assert(false);
			return false;
		}
		if(NULL == m_arrItems) {
			assert(false);
			return false;
		}

		if(m_nCurSize < 1) {
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
		if (NULL != pClearFun) {
			(m_arrItems[nIndex].m_object.*pClearFun)();
		}
		--m_nCurSize;
		return true;
	}

	bool Change(uint32_t nIndex, const T& object) 
	{
		if(NULL == m_arrItems) {
			assert(false);
			return false;
		}

		if(m_nCurSize < 1) {
			return false;
		}

		if(nIndex >= m_nAllcSize) {
			return false;
		}
		if(m_arrItems[nIndex].m_nIndex >= m_nCurSize) {
			return false;
		}
		m_arrItems[nIndex].m_object = object;
		return true;
	}

	const T* Find(uint32_t nIndex) const 
	{
		if(NULL == m_arrItems) {
			assert(false);
			return NULL;
		}

		if(m_nCurSize < 1) {
			return NULL;
		}

		if(nIndex >= m_nAllcSize) {
			return NULL;
		}
		if(m_arrItems[nIndex].m_nIndex >= m_nCurSize) {
			return NULL;
		}
		return &m_arrItems[nIndex].m_object;
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
		
		if(NULL == right.m_arrItems) {
			return *this;
		}

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
				}
				m_nCurSize = right.m_nCurSize;
				for(uint32_t j = right.m_nCurSize; j < m_nAllcSize; ++j) {
					m_arrItems[j].m_nIndex = XQXTABLE_INDEX_NIL;
					new(&m_arrItems[j].m_object)T;
				}
			}
		}
		return *this;
	}

private:
	void InnerClear() {
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
		for(uint32_t j = nSize; j < m_nAllcSize; ++j) {
			m_arrItems[j].m_object.~T();
		}
	}

private:
	struct SXqxItem* m_arrItems;
	uint32_t m_nAllcSize;
	uint32_t m_nCurSize;
};

}

#endif // XQXTABLE0_H