/*
 * File:   SimpleAllocator.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_23, 10:32
 */

#ifndef SIMPLEALLOCATOR_H_
#define SIMPLEALLOCATOR_H_

#include <vector>
#include <assert.h>
#include "SpinLock.h"
#include "ScopedLock.h"
#include "SysTickCount.h"

namespace util
{

#define ARRAY_DEFAULT_SIZE 8
#define ALLOC_MIN_SIZE 8
#define RECOVER_TIMEOUT_SECOND 3600

	class CXqxAllocator
	{
	public:
		CXqxAllocator(size_t nSize = ARRAY_DEFAULT_SIZE)
			: m_nOffset(-1)
			, m_nCurSize(0)
			, m_arrPtrs(NULL)
			, m_nAllocSize(0)
		{
			if(0 == nSize) {
				nSize = ARRAY_DEFAULT_SIZE;
			}
			thd::CScopedLock lock(m_mutex);
			m_arrPtrs = (void**)malloc(sizeof(void*) * nSize);
			m_nAllocSize = nSize;
		}

		~CXqxAllocator() {

			thd::CScopedLock lock(m_mutex);
			if(NULL != m_arrPtrs) {
				for(size_t i = 0; i < m_nCurSize; ++i) {
					free(m_arrPtrs[i]);
				}
				free(m_arrPtrs);
				m_arrPtrs = NULL;
			}
			m_nCurSize = 0;
			m_nAllocSize = 0;
			m_nOffset = -1;
		}

		void* AllocateBlock(size_t size) {

			thd::CScopedLock lock(m_mutex);
			if((size_t)-1 == (m_nOffset + 1)) {
				assert(false);
				return NULL;
			}
			if(m_nOffset + 1 >= m_nCurSize) {
				// 需要加一个指针
				if(m_nCurSize + 1 > m_nAllocSize) {
					// 需要重新分配内存
					size_t nSize = m_nAllocSize * 2;
					if(nSize <= m_nAllocSize) {
						// 数据溢出
						assert(false);
						return NULL;
					}
					void** arrPtrs = (void**)realloc(m_arrPtrs, sizeof(void*) * nSize);
					if(NULL == arrPtrs) {
						assert(false);
						return NULL;
					}
					m_arrPtrs = arrPtrs;
					m_nAllocSize = nSize;
				}
				size_t nAllcoSize = sizeof(size_t);
				nAllcoSize += size;
				if(nAllcoSize < ALLOC_MIN_SIZE) {
					nAllcoSize = ALLOC_MIN_SIZE;
				}
				void* pNew = malloc(nAllcoSize);
				if(NULL == pNew) {
					assert(false);
					return NULL;
				}
				++m_nCurSize;
				++m_nOffset;
				m_arrPtrs[m_nOffset] = pNew;
				(*(size_t*)pNew) = m_nOffset;
			} else {
				void* pCur = m_arrPtrs[m_nOffset + 1];	

				size_t nAllcoSize = sizeof(size_t);
				nAllcoSize += size;
				if(nAllcoSize < ALLOC_MIN_SIZE) {
					nAllcoSize = ALLOC_MIN_SIZE;
				}
				void* pNew = realloc(pCur, nAllcoSize);
				if(NULL == pNew) {
					assert(false);
					return NULL;
				} else if(pCur != pNew) {
					m_arrPtrs[m_nOffset + 1] = pNew;
					pCur = pNew;
					++m_nOffset;
				} else {
					++m_nOffset;
					// 因为这边分配最快，所以也检测一下是否有可释放的内存。
					CheckAndFree();
				}
					
				(*(size_t*)pCur) = m_nOffset;
			}

			return ((char*)m_arrPtrs[m_nOffset] + sizeof(size_t));
		}

		void ReleaseBlock(const void* pBlock) {

			if(NULL == pBlock) {
				assert(false);
				return;
			}
			thd::CScopedLock lock(m_mutex);
			if((size_t)-1 == m_nOffset) {
				assert(false);
				return;
			}

			void* pCur = ((char*)pBlock - sizeof(size_t));
			size_t nIndex = (*(size_t*)pCur);
			if(nIndex > m_nOffset) {
				assert(false);
				return;
			}
			if(m_arrPtrs[nIndex] != pCur) {
				// 说明不是这边分配出去的指针，所以不做回收。
				assert(false);
				return;
			}

			if(nIndex == m_nOffset) {
				(*(uint64_t*)pCur) = GetSysTickCount();
				--m_nOffset;
				//判断最后一个是否超时
				CheckAndFree();
				return;
			}

			pCur = m_arrPtrs[nIndex];
			void* pTail = m_arrPtrs[m_nOffset];	
			m_arrPtrs[m_nOffset] = pCur;
			m_arrPtrs[nIndex] = pTail;
			(*(size_t*)pTail) = nIndex;
			(*(uint64_t*)pCur) = GetSysTickCount();
			--m_nOffset;

			//判断最后一个是否超时
			CheckAndFree();
		}

	private:
		inline void CheckAndFree() {
			if(0 == m_nCurSize) {
				return;
			}
			size_t nTailIndex = m_nCurSize - 1;
			if(nTailIndex > m_nOffset || -1 == m_nOffset) {
				void* pFreeEnd = m_arrPtrs[nTailIndex];
				uint64_t oldTime = (*(uint64_t*)pFreeEnd);
				int64_t difTime = GetSysTickCount() - oldTime;
				if(difTime > RECOVER_TIMEOUT_SECOND * 1000) {
					m_arrPtrs[--m_nCurSize] = NULL;
					free(pFreeEnd);
					pFreeEnd = NULL;
				}
			}
		}

	private:
		// used object set
		void** m_arrPtrs;
		// size
		size_t m_nAllocSize;
		// use size
		size_t m_nCurSize;
		// allocation offset
		size_t m_nOffset;
		// allocation mutex
		thd::CSpinLock m_mutex;
	};

}

#endif /* SIMPLEALLOCATOR_H_ */
