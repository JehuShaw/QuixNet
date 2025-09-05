/*
 * File:   PoolBase.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_23, 10:32
 */

#ifndef POOLBASE_H
#define POOLBASE_H

#include <vector>
#include "XqxAllocator.h"

namespace util
{
#ifdef DISABLE_POOL_BASE
	template <typename T>
    class PoolBase {};

#else

	template <typename T, class AllocT = CXqxAllocator>
    class PoolBase {
	public:
		virtual ~PoolBase() {}

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
		static AllocT myAlloc;
	};

	template<typename T, class AllocT>
	AllocT PoolBase<T, AllocT>::myAlloc;


#endif // DISABLE_POOL_BASE

#ifdef SUPPORT_CPLUSPLUS11
	// use c++11
	template<typename T, class AllocT = CXqxAllocator>
	class WrapPoolIs : public T {
	public:
		WrapPoolIs() : T() {}
		template<typename ...A>
		WrapPoolIs(A ... args) : T(args ...) {}

		virtual ~WrapPoolIs() override {}

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
		static AllocT myAlloc;
	};

	template<typename T, class AllocT>
	AllocT WrapPoolIs<T, AllocT>::myAlloc;


	template<typename T, class AllocT = CXqxAllocator>
	class WrapPoolHas : public PoolBase< WrapPoolHas<T, AllocT>, AllocT > {
	public:
		WrapPoolHas() : m_data() {}
		
		template<typename ...A>
		WrapPoolHas(A ... args) : m_data(args ...) {}

		~WrapPoolHas() {}

		T *operator&() {
			return &m_data;
		}

		const T *operator&() const {
			return &m_data;
		}

		T &operator*() {
			return m_data;
		}

		const T &operator*() const {
			return m_data;
		}

		T *operator->() {
			return &m_data;
		}

		const T *operator->()const {
			return &m_data;
		}

	private:
		T m_data;
	};

#else
	// T destructor must be virtual 
	template<typename T, class AllocT = CXqxAllocator>
	class WrapPoolIs : public T {
	public:
		WrapPoolIs() : T() {}
		template<typename A1>
		WrapPoolIs(A1 arg1) : T(arg1) {}
		template<typename A1, typename A2>
		WrapPoolIs(A1 arg1, A2 arg2) : T(arg1, arg2) {}
		template<typename A1, typename A2, typename A3>
		WrapPoolIs(A1 arg1, A2 arg2, A3 arg3) : T(arg1, arg2, arg3) {}
		template<typename A1, typename A2, typename A3, typename A4>
		WrapPoolIs(A1 arg1, A2 arg2, A3 arg3, A4 arg4) : T(arg1, arg2, arg3, arg4) {}
		template<typename A1, typename A2, typename A3, typename A4, typename A5>
		WrapPoolIs(A1 arg1, A2 arg2, A3 arg3, A4 arg4, A5 arg5) : T(arg1, arg2, arg3, arg4, arg5) {}

		virtual ~WrapPoolIs() {}

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
		static AllocT myAlloc;
	};

	template<typename T, class AllocT>
	AllocT WrapPoolIs<T, AllocT>::myAlloc;

	template<typename T, class AllocT = CXqxAllocator>
	class WrapPoolHas : public PoolBase< WrapPoolHas<T, AllocT>, AllocT > {
	public:
		WrapPoolHas() : m_data() {}
		template<typename A1>
		WrapPoolHas(A1 arg1) : m_data(arg1) {}
		template<typename A1, typename A2>
		WrapPoolHas(A1 arg1, A2 arg2) : m_data(arg1, arg2) {}
		template<typename A1, typename A2, typename A3>
		WrapPoolHas(A1 arg1, A2 arg2, A3 arg3) : m_data(arg1, arg2, arg3) {}
		template<typename A1, typename A2, typename A3, typename A4>
		WrapPoolHas(A1 arg1, A2 arg2, A3 arg3, A4 arg4) : m_data(arg1, arg2, arg3, arg4) {}
		template<typename A1, typename A2, typename A3, typename A4, typename A5>
		WrapPoolHas(A1 arg1, A2 arg2, A3 arg3, A4 arg4, A5 arg5) : m_data(arg1, arg2, arg3, arg4, arg5) {}

		~WrapPoolHas() {}

		T *operator&() {
			return &m_data;
		}

		const T *operator&() const {
			return &m_data;
		}

		T &operator*() {
			return m_data;
		}

		const T &operator*() const {
			return m_data;
		}

		T *operator->() {
			return &m_data;
		}

		const T *operator->()const {
			return &m_data;
		}

	private:
		T m_data;
	};

#endif /* SUPPORT_CPLUSPLUS11 */

}

#endif /* POOLBASE_H */