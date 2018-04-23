/*
 * File:   PoolBase.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_23, 10:32
 */

#ifndef POOLBASE_H_
#define POOLBASE_H_

#include "Common.h"
#include <vector>
#include "XqxAllocator.h"

namespace util 
{
#ifdef DISABLE_POOL_BASE
	template <
		typename T
	>	
	class PoolBase{};

#else

	template <
		typename T,
		class Alloc = CXqxAllocator
	>
	class PoolBase
	{
		typedef Alloc MyAlloc;
	public:
		virtual ~PoolBase() {
		}

		void* operator new(size_t s) {
			return myAlloc.AllocateBlock(s);
		}

		void operator delete(void* p) {
			myAlloc.ReleaseBlock(p);
		}

		void* operator new[](size_t s) {
			return myAlloc.AllocateBlock(s);
		}

		void operator delete[](void* p) {
			myAlloc.ReleaseBlock(p);
		}

	private:
		static MyAlloc myAlloc;
	};

	template<typename T, class Alloc>
	typename PoolBase<T, Alloc>::MyAlloc PoolBase<T, Alloc>::myAlloc;


#endif // DISABLE_POOL_BASE

}

#endif /* POOLBASE_H_ */