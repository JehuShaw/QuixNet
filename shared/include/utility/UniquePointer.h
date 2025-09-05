/*
 * File:   UniquePointer.h
 * Author: Jehu Shaw
 *
 * Created on 2013_10_2, 10:32
 */

#ifndef UNIQUE_POINTER_H
#define	UNIQUE_POINTER_H

#include <stdint.h>
#include <stdio.h>
#include <stdexcept>
#include "AtomicLock.h"
#include "SpinRWLock.h"
#include "PoolBase.h"
#include "ScopedRWLock.h"

namespace util {

class invalid_ptr_unique : public std::runtime_error {
public:
	explicit invalid_ptr_unique(const std::string& s):std::runtime_error(s) {
	}

	virtual ~invalid_ptr_unique() throw() { }
};

template<typename T>
class invalid_ptr_unique_ : public invalid_ptr_unique {
public:
	explicit invalid_ptr_unique_(const std::string& s)
		: invalid_ptr_unique(s) {

	}

	virtual ~invalid_ptr_unique_() throw() {}
};

template<typename T>
class CUniquePointer {
public:
    //default constructor:unbound handle
    explicit CUniquePointer(T *ptr = NULL) : p(ptr) {
	}
    //copy control members to manage the use count and pointers
	CUniquePointer(CUniquePointer& orig) : p(NULL) {
		if(this == &orig) {
			return;
		}
		thd::CScopedWriteLock wLockOrig(orig.rwticket);
		p = orig.Release();
	}

	// T2 sub class or base class
	template<class T2>
	CUniquePointer(const CUniquePointer<T2>& orig) : p(NULL) {
		if((intptr_t)this == (intptr_t)&orig) {
			return;
		}
		thd::CScopedWriteLock wLockOrig(orig.rwticket);
		p = dynamic_cast<T*>(const_cast<CUniquePointer<T2>&>(orig).Release());
	}

    //dispose
    ~CUniquePointer(){
		thd::CScopedWriteLock wLockThis(rwticket);
		delete p;
	}

	CUniquePointer& operator= (CUniquePointer& right) {
		if(this == &right) {
			return *this;
		}
		thd::CScopedWriteLock wLockThis(rwticket);
		thd::CScopedWriteLock wLockRight(right.rwticket);
		Reset(right.Release());
		return *this; 
	}

	// T2 sub class or base class
	template<class T2>
	CUniquePointer& operator= (CUniquePointer<T2>& right) {
		if((intptr_t)this == (intptr_t)&right) {
			return *this;
		}
		thd::CScopedWriteLock wLockThis(rwticket);
		thd::CScopedWriteLock wLockRight(right.rwticket);
		Reset(dynamic_cast<T*>(right.Release()));
		return *this;
	}


    bool operator==(const CUniquePointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
        return p == right.p;
    }
    bool operator!=(const CUniquePointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
        return p != right.p;
    }
    bool operator> (const CUniquePointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
        return p > right.p;
    }
    bool operator< (const CUniquePointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
        return p < right.p;
    }

	// T2 sub class or base class
	template<class T2>
	bool operator==(const CUniquePointer<T2>& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		return p == (T2*)right.p;
	}
	template<class T2>
	bool operator!=(const CUniquePointer<T2>& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		return p != (T2*)right.p;
	}
	template<class T2>
	bool operator> (const CUniquePointer<T2>& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		return p > (T2*)right.p;
	}
	template<class T2>
	bool operator< (const CUniquePointer<T2>& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		return p < (T2*)right.p;
	}


    bool operator==(const T* const right) const {
		thd::CScopedReadLock rLockThis(rwticket);
        return p == right;
    }
    bool operator!=(const T* const right) const {
		thd::CScopedReadLock rLockThis(rwticket);
        return p != right;
    }
    bool operator > (const T* const right) const {
		thd::CScopedReadLock rLockThis(rwticket);
        return p > right;
    }
    bool operator < (const T* const right) const {
		thd::CScopedReadLock rLockThis(rwticket);
        return p < right;
    }

    bool operator==(intptr_t right) const {
		thd::CScopedReadLock rLockThis(rwticket);
        return (intptr_t)p == right;
    }
    bool operator!=(intptr_t right) const {
		thd::CScopedReadLock rLockThis(rwticket);
        return (intptr_t)p != right;
    }
    bool operator > (intptr_t right) const {
		thd::CScopedReadLock rLockThis(rwticket);
        return (intptr_t)p > right;
    }
    bool operator < (intptr_t right) const {
		thd::CScopedReadLock rLockThis(rwticket);
        return (intptr_t)p < right;
    }

	T* operator->() {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL != p){
			return p;
		}
		throw invalid_ptr_unique_<T>("unbound Parameter");
	}

    const T* operator->() const {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL != p){
			return p;
		}
		throw invalid_ptr_unique_<T>("unbound Parameter");
    }

	T& operator*() {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL != p){
			return *p;
		}
		throw invalid_ptr_unique_<T>("unbound Parameter");
	}

    const T& operator*() const {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL != p){
			return *p;
		}
        throw invalid_ptr_unique_<T>("unbound Parameter");
    }

	inline bool IsInvalid() const {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL == p) {
			return true;
		}
		return false;
	}

	inline T *Get() const throw()
	{
		return p;
	}

	inline T *Release() throw() 
	{
		T *temp = p;
		p = NULL;
		return temp;
	}

	inline void Reset(T *ptr = NULL)
	{
		if (ptr != p) {
			delete p;
		}
		p = ptr;
	}

private:
	template<class T2> friend class CUniquePointer;
    T* p;
	thd::CSpinRWLock rwticket;
};

}

#endif	/* UNIQUE_POINTER_H */

