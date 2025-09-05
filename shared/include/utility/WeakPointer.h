/*
 * File:   WeakPointer.h
 * Author: Jehu Shaw
 *
 * Created on 2013_10_2, 10:32
 */

#ifndef WEAKPOINTER_H
#define	WEAKPOINTER_H

#include <stdint.h>
#include <stdio.h>
#include <stdexcept>
#include "AutoPointer.h"

namespace util {

class invalid_ptr_weak : public invalid_pointer {
public:
	explicit invalid_ptr_weak(const std::string& s):invalid_pointer(s) {

	}

	virtual ~invalid_ptr_weak() throw() { }
};

template<typename T>
class invalid_ptr_weak_ : public invalid_ptr_weak {
public:
	explicit invalid_ptr_weak_(const std::string& s):invalid_ptr_weak(s) {

	}

	virtual ~invalid_ptr_weak_() throw() { }
};

template<typename T>
class CWeakPointer {
public:
    //default constructor:unbound handle
    CWeakPointer():p(NULL),counter(NULL) {
	}
    //copy control members to manage the use count and pointers
    CWeakPointer(const CWeakPointer& orig):p(NULL),counter(NULL) {

		if (this == &orig) {
			return;
		}
		thd::CScopedReadLock rLockOrig(orig.rwticket);
        if (NULL != orig.counter){
            p = orig.p;
            counter = orig.counter;
			atomic_inc(&orig.counter->weak);
        }
    }

    template<class T2>
    CWeakPointer(const CWeakPointer<T2>& orig):p(NULL),counter(NULL) {

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

	CWeakPointer(const CAutoPointer<T>& orig):p(NULL),counter(NULL) {

		thd::CScopedReadLock rLockOrig(orig.rwticket);
		if (NULL != orig.counter) {
			p = orig.p;
			counter = orig.counter;
			atomic_inc(&orig.counter->weak);
		}
	}

	template<class T2>
	CWeakPointer(const CAutoPointer<T2>& orig):p(NULL),counter(NULL) {

		thd::CScopedReadLock rLockOrig(orig.rwticket);
		if(IsBase(orig.p)) {
			if(NULL != orig.counter) {
				p = dynamic_cast<T*>(orig.p);
				counter = orig.counter;
				atomic_inc(&orig.counter->weak);
			}
		} else if(IsChild<T2>(p)) {
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
    ~CWeakPointer() {
		thd::CScopedWriteLock wLockThis(rwticket);
		decr_use();
	}

    CWeakPointer& operator= (const CWeakPointer& right) {
		if (this == &right) {
			return *this;
		}
		thd::CScopedWriteLock wLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		if (NULL != right.counter) {
			atomic_inc(&right.counter->weak);
		}
		decr_use();
		p = right.p;
		counter = right.counter;

        return *this;
    }

    template<class T2>
    CWeakPointer& operator= (const CWeakPointer<T2>& right) {
		if ((intptr_t)this == (intptr_t)&right) {
			return *this;
		}
		thd::CScopedWriteLock wLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
        if (IsBase(right.p)) {
			if (NULL != right.counter) {
				atomic_inc(&right.counter->weak);
			}
			decr_use();
			p = dynamic_cast<T*>(right.p);
			counter = right.counter;

        } else if(IsChild<T2>(p)) {
            T* pChild = dynamic_cast<T*>(right.p);
            if (NULL == pChild) {
                return *this;
            }
			if (NULL != right.counter) {
				atomic_inc(&right.counter->weak);
			}
			decr_use();
			p = pChild;
			counter = right.counter;
        }

        return *this;
    }


	CWeakPointer& operator= (const CAutoPointer<T>& right) {

		thd::CScopedWriteLock wLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		if (NULL != right.counter) {
			atomic_inc(&right.counter->weak);
		}
		decr_use();
		p = right.p;
		counter = right.counter;

		return *this;
	}

	template<class T2>
	CWeakPointer& operator= (const CAutoPointer<T2>& right) {

		thd::CScopedWriteLock wLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		if (IsBase(right.p)) {
			if (NULL != right.counter) {
				atomic_inc(&right.counter->weak);
			}
			decr_use();
			p = dynamic_cast<T*>(right.p);
			counter = right.counter;

		} else if(IsChild<T2>(p)) {
			T* pChild = dynamic_cast<T*>(right.p);
			if (NULL == pChild) {
				return *this;
			}
			if (NULL != right.counter) {
				atomic_inc(&right.counter->weak);
			}
			decr_use();
			p = pChild;
			counter = right.counter;
		}

		return *this;
	}

    bool operator==(const CWeakPointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
        return p == right.p;
    }
    bool operator!=(const CWeakPointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
        return p != right.p;
    }
    bool operator> (const CWeakPointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
        return p > right.p;
    }
    bool operator< (const CWeakPointer& right) const {
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
		if(NULL != counter && counter->use > 0){
			return p;
		}
		throw invalid_ptr_weak_<T>("unbound Parameter");
	}

    const T* operator->() const {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL != counter && counter->use > 0){
			return p;
		}
		throw invalid_ptr_weak_<T>("unbound Parameter");
    }

	T& operator*() {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL != counter && counter->use > 0){
			return *p;
		}
		throw invalid_ptr_weak_<T>("unbound Parameter");
	}

    const T& operator*() const {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL != counter && counter->use > 0){
			return *p;
		}
        throw invalid_ptr_weak_<T>("unbound Parameter");
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

	CAutoPointer<T> GetStrong() {
		thd::CScopedReadLock rLockThis(rwticket);
		CAutoPointer<T> autoPointer;
		if(NULL != counter
			&& counter->use > 0) {
			autoPointer.counter = counter;
			autoPointer.p = p;

			atomic_inc(&autoPointer.counter->use);
		}
		return autoPointer;
	}

private:
    static bool IsBase(const T* p) {
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
	static bool IsChild(const T2* p) {
		return true;
	}
    template<class T2>
    static bool IsChild(...) {
        return false;
    }

private:
	template<class T2> friend class CWeakPointer;
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

};

}

#endif	/* WEAKPOINTER_H */

