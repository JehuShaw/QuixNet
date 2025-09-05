/*
 * File:   AutoPointer.h
 * Author: Jehu Shaw
 *
 * Created on 2013_10_2, 10:32
 */

#ifndef AUTO_POINTER_H
#define	AUTO_POINTER_H

#include <stdint.h>
#include <stdio.h>
#include <stdexcept>
#include "AtomicLock.h"
#include "SpinRWLock.h"
#include "PoolBase.h"
#include "ScopedRWLock.h"

namespace util {

typedef struct PtrCounter : public PoolBase<PtrCounter> {

#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ ) || defined( _WIN64 )
	volatile long use;
	volatile long weak;
#else
	volatile int use;
	volatile int weak;
#endif
	volatile bool deletable;
    volatile bool deleting;
} PtrCounter;

class invalid_pointer : public std::runtime_error {
public:
	explicit invalid_pointer(const std::string& s):std::runtime_error(s) {

	}

	virtual ~invalid_pointer() throw() { }
};

class invalid_ptr_auto : public invalid_pointer {
public:
	explicit invalid_ptr_auto(const std::string& s):invalid_pointer(s) {

	}

	virtual ~invalid_ptr_auto() throw() { }
};

template<typename T>
class invalid_ptr_auto_ : public invalid_ptr_auto {
public:
	explicit invalid_ptr_auto_(const std::string& s):invalid_ptr_auto(s) {

	}

	virtual ~invalid_ptr_auto_() throw() { }
};

template<typename T>
class CAutoPointer {
public:
    //default constructor:unbound handle
    CAutoPointer():p(NULL), counter(NULL) {
	}
    //copy control members to manage the use count and pointers
    CAutoPointer(const CAutoPointer& orig):p(NULL), counter(NULL) {

		if (this == &orig) {
			return;
		}
		thd::CScopedReadLock rLockOrig(orig.rwticket);
        if (NULL == orig.counter) {
			return;
		}

		p = orig.p;
		counter = orig.counter;
		atomic_inc(&orig.counter->use);
    }
    //attaches a handle to a copy of the T object
    CAutoPointer(const T& orig):p(NULL), counter(NULL) {

		p = new T(orig);
		counter = new PtrCounter;
		atomic_xchg(&counter->use, 1);
		atomic_xchg(&counter->weak, 0);
		atomic_xchg8(&counter->deletable, true);
        atomic_xchg8(&counter->deleting, false);

    }
    //If pass new operator pointer, you must set "bDel" true.
   explicit CAutoPointer(const T* orig, bool bDel = true):p(NULL), counter(NULL) {

        if (NULL != orig) {
            p = const_cast<T*>(orig);
            counter = new PtrCounter;
			atomic_xchg(&counter->use, 1);
			atomic_xchg(&counter->weak, 0);
			atomic_xchg8(&counter->deletable, bDel);
            atomic_xchg8(&counter->deleting, false);
        }
    }

    template<class T2>
    CAutoPointer(const CAutoPointer<T2>& orig):p(NULL), counter(NULL) {
		if ((intptr_t)this == (intptr_t)&orig) {
			return;
		}
		thd::CScopedReadLock rLockOrig(orig.rwticket);
		if (NULL == orig.counter) {
			return;
		}

        if (IsBase(orig.p)) {

			p = dynamic_cast<T*>(orig.p);
			counter = orig.counter;
			atomic_inc(&orig.counter->use);
        } else if(IsChild<T2>(p)) {

            T* pChild = dynamic_cast<T*>(orig.p);
            if(NULL == pChild) {
                return;
            }
			p = pChild;
			counter = orig.counter;
			atomic_inc(&orig.counter->use);
        }
    }

    //dispose
    ~CAutoPointer(){
		thd::CScopedWriteLock wLockThis(rwticket);
		decr_use();
	}

    CAutoPointer& operator= (const CAutoPointer& orig){

		if (this == &orig) {
			return *this;
		}
		thd::CScopedWriteLock wLockThis(rwticket);
		thd::CScopedReadLock rLockOrig(orig.rwticket);
		if (NULL != orig.counter) {
			atomic_inc(&orig.counter->use);
		}
		decr_use();
		p = orig.p;
		counter = orig.counter;

        return *this;
    }

    template<class T2>
    CAutoPointer& operator= (const CAutoPointer<T2>& orig) {

		if ((intptr_t)this == (intptr_t)&orig) {
			return *this;
		}
		thd::CScopedWriteLock wLockThis(rwticket);
		thd::CScopedReadLock rLockOrig(orig.rwticket);
		if (IsBase(orig.p)) {
			if(NULL != orig.counter) {
				atomic_inc(&orig.counter->use);
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
				atomic_inc(&orig.counter->use);
			}
			decr_use();
			p = pChild;
			counter = orig.counter;
		}

        return *this;
    }

    bool operator==(const CAutoPointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		return p == right.p;
    }
    bool operator!=(const CAutoPointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		return p != right.p;
    }
    bool operator> (const CAutoPointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		return p > right.p;
    }
    bool operator< (const CAutoPointer& right) const {
		thd::CScopedReadLock rLockThis(rwticket);
		thd::CScopedReadLock rLockRight(right.rwticket);
		return p < right.p;
    }

	//If pass new operator pointer, you must set "bDel" true.
    void SetRawPointer(const T* const orig, bool bDel = true) {

		thd::CScopedWriteLock wLockThis(rwticket);
		decr_use();
		if(NULL == orig) {
			p = NULL;
			counter = NULL;
			return;
		}
        if (NULL == counter) {
			p = const_cast<T*>(orig);
			counter = new PtrCounter;
			counter->use = 1;
			atomic_xchg(&counter->use, 1);
			atomic_xchg(&counter->weak, 0);
			atomic_xchg8(&counter->deletable, bDel);
            atomic_xchg8(&counter->deleting, false);
        } else {
			if (orig == p) {
				atomic_inc(&counter->use);
				atomic_xchg8(&counter->deletable, bDel);
			} else {
				// reallocation ,don't delete before pointer.
				p = const_cast<T*>(orig);
				counter = new PtrCounter;
				atomic_xchg(&counter->use, 1);
				atomic_xchg(&counter->weak, 0);
				atomic_xchg8(&counter->deletable, bDel);
                atomic_xchg8(&counter->deleting, false);
			}
        }
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
		if(NULL != p){return p;}
		else{ throw invalid_ptr_auto_<T>("unbound Parameter");}
	}

    const T* operator->() const {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL != p){return p;}
		else{ throw invalid_ptr_auto_<T>("unbound Parameter");}
    }

	T& operator*() {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL != p){return *p;}
		else{ throw invalid_ptr_auto_<T>("unbound Parameter");}
	}

    const T& operator*() const {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL != p){return *p;}
		else{ throw invalid_ptr_auto_<T>("unbound Parameter");}
    }

	bool IsInvalid() const {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL == p) {
			return true;
		}
		return false;
	}

	long PeekUse() const {
		thd::CScopedReadLock rLockThis(rwticket);
		if(NULL == counter) {
			return 0;
		}
		return (long)counter->use;
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
	template<class FT> friend class CWeakPointer;
	template<class FT2> friend class CAutoPointer;
	template<class FT3> friend class CScopedPointer;
    T* p;
    PtrCounter* counter;
	thd::CSpinRWLock rwticket;

    void decr_use() throw() {
        if (NULL != counter) {
            if (atomic_dec(&counter->use) == 0) {
                if (NULL != p) {
					if (counter->deletable) {
						atomic_xchg8(&counter->deleting, true);
						delete p;
						atomic_xchg8(&counter->deleting, false);
					}
                    p = NULL;
                }

				if (counter->weak < 1) {
					delete counter;
					counter = NULL;
				}
            }
        }
    }

};

}

#endif	/* AUTO_POINTER_H */

