/*
 * File:   ScopedPointer.h
 * Author: Jehu Shaw
 *
 * Created on 2013_10_2, 10:32
 */

#ifndef SCOPED_POINTER_H
#define	SCOPED_POINTER_H

#include "Common.h"
#include <stdio.h>
#include <stdexcept>
#include "AutoPointer.h"

namespace util {

class invalid_ptr_scoped : public invalid_pointer {
public:
	explicit invalid_ptr_scoped(const std::string& s):invalid_pointer(s) {

	}

	virtual ~invalid_ptr_scoped() throw() { }
};

template<typename T>
class invalid_ptr_scoped_ : public invalid_ptr_scoped {
public:
	explicit invalid_ptr_scoped_(const std::string& s):invalid_ptr_scoped(s) {

	}

	virtual ~invalid_ptr_scoped_() throw() { }
};

template<typename T>
class CScopedPointer {
public:
    //default constructor:unbound handle
    CScopedPointer():p(NULL),counter(NULL) {
	}
    //copy control members to manage the use count and pointers
	CScopedPointer(const CScopedPointer& orig):p(NULL),counter(NULL) {

		if (this == &orig) {
			return;
		}
		thd::CScopedReadLock rLockOrig(orig.rwticket);
		if (NULL != orig.counter) {
			p = orig.p;
			counter = orig.counter;
			atomic_inc(&orig.counter->weak);
		}
	}

	template<class T2>
	CScopedPointer(const CScopedPointer<T2>& orig):p(NULL),counter(NULL) {

		if ((intptr_t)this == (intptr_t)&orig) {
			return;
		}
		thd::CScopedReadLock rLockOrig(orig.rwticket);
		if (IsBase(orig.p)) {
			if(NULL != orig.counter){
				p = dynamic_cast<T*>(orig.p);
				counter = orig.counter;
				atomic_inc(&orig.counter->weak);
			}
		} else if (IsChild<T2>(p)) {
			T* pChild = dynamic_cast<T*>(orig.p);
			if (NULL == pChild) {
				return;
			}
			if (NULL != orig.counter) {
				p = pChild;
				counter = orig.counter;
				atomic_inc(&orig.counter->weak);
			}
		}
	}

	CScopedPointer(const CAutoPointer<T>& orig):p(NULL),counter(NULL) {

		thd::CScopedReadLock rLockOrig(orig.rwticket);
		if (NULL != orig.counter) {
			p = orig.p;
			counter = orig.counter;
			atomic_inc(&orig.counter->weak);
		}
	}

	template<class T2>
	CScopedPointer(const CAutoPointer<T2>& orig):p(NULL),counter(NULL) {

		thd::CScopedReadLock rLockOrig(orig.rwticket);
		if (IsBase(orig.p)) {
			if (NULL != orig.counter) {
				p = dynamic_cast<T*>(orig.p);
				counter = orig.counter;
				atomic_inc(&orig.counter->weak);
			}
		} else if (IsChild<T2>(p)) {
			T* pChild = dynamic_cast<T*>(orig.p);
			if (NULL == pChild) {
				return;
			}
			if (NULL != orig.counter) {
				p = pChild;
				counter = orig.counter;
				atomic_inc(&orig.counter->weak);
			}
		}
	}

    //dispose
    ~CScopedPointer(){
		thd::CScopedWriteLock wLockThis(rwticket);
		decr_use();
	}

	CScopedPointer& operator= (const CAutoPointer<T>& orig){

		thd::CScopedWriteLock wLockThis(rwticket);
		thd::CScopedReadLock rLockOrig(orig.rwticket);
		if (NULL != orig.counter) {
			atomic_inc(&orig.counter->weak);
		}
		decr_use();
		p = orig.p;
		counter = orig.counter;

		return *this;
	}

	template<class T2>
	CScopedPointer& operator= (const CAutoPointer<T2>& orig) {

		thd::CScopedWriteLock wLockThis(rwticket);
		thd::CScopedReadLock rLockOrig(orig.rwticket);
		if (IsBase(orig.p)) {
			if (NULL != orig.counter) {
				atomic_inc(&orig.counter->weak);
			}
			decr_use();
			p = dynamic_cast<T*>(orig.p);
			counter = orig.counter;

		} else if(IsChild<T2>(p)) {
			T* pChild = dynamic_cast<T*>(orig.p);
			if (NULL == pChild) {
				return *this;
			}
			if (NULL != orig.counter) {
				atomic_inc(&orig.counter->weak);
			}
			decr_use();
			p = pChild;
			counter = orig.counter;

		}

		return *this;
	}

    bool operator==(const CScopedPointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
        return p == right.p;
    }
    bool operator!=(const CScopedPointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
        return p != right.p;
    }
    bool operator> (const CScopedPointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
        return p > right.p;
    }
    bool operator< (const CScopedPointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
        return p < right.p;
    }

	bool operator==(const CAutoPointer<T>& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		return p == right.p;
	}
	bool operator!=(const CAutoPointer<T>& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		return p != right.p;
	}
	bool operator> (const CAutoPointer<T>& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		return p > right.p;
	}
	bool operator< (const CAutoPointer<T>& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		return p < right.p;
	}


    bool operator==(T* const right) const {
		thd::CScopedReadLock rLockThis(rwticket);
        return p == right;
    }
    bool operator!=(T* const right) const {
		thd::CScopedReadLock rLockThis(rwticket);
        return p != right;
    }
    bool operator > (T* const right) const {
		thd::CScopedReadLock rLockThis(rwticket);
        return p > right;
    }
    bool operator < (T* const right) const {
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
		if(NULL != counter && counter->use > 0){
			return p;
		}
		throw invalid_ptr_scoped_<T>("unbound Parameter");
	}

    const T* operator->() const {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL != counter && counter->use > 0){
			return p;
		}
		throw invalid_ptr_scoped_<T>("unbound Parameter");
    }

	T& operator*() {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL != counter && counter->use > 0){
			return *p;
		}
		throw invalid_ptr_scoped_<T>("unbound Parameter");
	}

    const T& operator*() const {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL != counter && counter->use > 0){
			return *p;
		}
        throw invalid_ptr_scoped_<T>("unbound Parameter");
    }

	inline bool IsInvalid() const {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL == counter) {
			return true;
		}
		if(counter->use < 1) {
			return true;
		}
		return false;
	}

private:
    static bool IsBase(T* p) {
        return true;
    }
    static bool IsBase(...) {
        return false;
    }
    template<class T2>
    static bool IsChild(T2* p) {
        return true;
    }
    template<class T2>
    static bool IsChild(...) {
        return false;
    }

private:
	template<class T2> friend class CScopedPointer;
    T* p;
    PtrCounter* counter;
	thd::CSpinRWLock rwticket;

    void decr_use(){
        if(NULL != counter){
            if(atomic_dec(&counter->weak) == 0){

				if(counter->use < 1 
                    && !counter->deleting) 
                {
					delete counter;
					counter = NULL;
				}
            }
        }
    }

private:
	CScopedPointer& operator= (const CScopedPointer& orig) { return *this; }

};

}

#endif	/* SCOPED_POINTER_H */

