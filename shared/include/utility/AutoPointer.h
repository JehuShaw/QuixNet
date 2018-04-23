/*
 * File:   AutoPointer.h
 * Author: Jehu Shaw
 *
 * Created on 2013_10_2, 10:32
 */

#ifndef AUTOPOINTER_H_
#define	AUTOPOINTER_H_

#include <stdio.h>
#include <stdexcept>
#include "AtomicLock.h"
#include "SpinRWLock.h"
#include "PoolBase.h"
#include "ScopedRWLock.h"

namespace util {

typedef struct PtrCounter : public PoolBase<PtrCounter> {

#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ )
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

		if(this == &orig) {
			return;
		}
		thd::CScopedReadLock scopeReadLock(orig.rwticket);
        if(NULL == orig.counter){
			return;
		}

		this->p = orig.p;
		this->counter = orig.counter;
		atomic_inc(&orig.counter->use);
    }
    //attaches a handle to a copy of the T object
    CAutoPointer(const T& orig):p(NULL), counter(NULL) {

		this->p = new T(orig);
		this->counter = new PtrCounter;
		atomic_xchg(&this->counter->use, 1);
		atomic_xchg(&this->counter->weak, 0);
		atomic_xchg8(&this->counter->deletable, true);
        atomic_xchg8(&this->counter->deleting, false);

    }
    //If pass new operator pointer, you must set "bDel" true.
   explicit CAutoPointer(const T* orig, bool bDel = true):p(NULL), counter(NULL) {

        if(NULL != orig){
            this->p = const_cast<T*>(orig);
            this->counter = new PtrCounter;
			atomic_xchg(&this->counter->use, 1);
			atomic_xchg(&this->counter->weak, 0);
			atomic_xchg8(&this->counter->deletable, bDel);
            atomic_xchg8(&this->counter->deleting, false);
        }
    }

    template<class T2>
    CAutoPointer(const CAutoPointer<T2>& orig):p(NULL), counter(NULL) {
		if((intptr_t)this == (intptr_t)&orig) {
			return;
		}
		thd::CScopedReadLock scopeReadLock(orig.rwticket);
		if(NULL == orig.counter) {
			return;
		}

        if(IsBase(orig.p)) {

			this->p = dynamic_cast<T*>(orig.p);
			this->counter = orig.counter;
			atomic_inc(&orig.counter->use);
        } else if(IsChild<T2>(this->p)) {

            T* pChild = dynamic_cast<T*>(orig.p);
            if(NULL == pChild) {
                return;
            }
			this->p = pChild;
			this->counter = orig.counter;
			atomic_inc(&orig.counter->use);
        }
    }

    //dispose
    ~CAutoPointer(){
		thd::CScopedWriteLock scopeWriteLock(this->rwticket);
		decr_use();
	}

    CAutoPointer& operator= (const CAutoPointer& orig){

		thd::CScopedWriteLock scopeWriteLock(rwticket);
		if(this == &orig) {
			return *this;
		}
		thd::CScopedReadLock scopeReadLock(orig.rwticket);
		if(NULL != orig.counter){
			atomic_inc(&orig.counter->use);
		}
		decr_use();
		this->p = orig.p;
		this->counter = orig.counter;

        return *this;
    }

    template<class T2>
    CAutoPointer& operator= (const CAutoPointer<T2>& orig) {

		thd::CScopedWriteLock scopeWriteLock(rwticket);
		if((intptr_t)this == (intptr_t)&orig) {
			return *this;
		}
		thd::CScopedReadLock scopeReadLock(orig.rwticket);
		if(IsBase(orig.p)) {
			if(NULL != orig.counter) {
				atomic_inc(&orig.counter->use);
			}
			decr_use();
			this->p = dynamic_cast<T*>(orig.p);
			this->counter = orig.counter;

		} else if(IsChild<T2>(this->p)) {
			T* pChild = dynamic_cast<T*>(orig.p);
			if(NULL == pChild) {
				return *this;
			}
			if(NULL != orig.counter) {
				atomic_inc(&orig.counter->use);
			}
			decr_use();
			this->p = pChild;
			this->counter = orig.counter;
		}

        return *this;
    }

    bool operator==(const CAutoPointer& right ) const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		return this->p == right.p;
    }
    bool operator!=(const CAutoPointer& right ) const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		return this->p != right.p;
    }
    bool operator> (const CAutoPointer& right ) const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		return this->p > right.p;
    }
    bool operator< (const CAutoPointer& right ) const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		return this->p < right.p;
    }

	//If pass new operator pointer, you must set "bDel" true.
    void SetRawPointer(const T* const orig, bool bDel = true){

		thd::CScopedWriteLock scopeWriteLock(rwticket);
		decr_use();
		if(NULL == orig) {
			this->p = NULL;
			this->counter = NULL;
			return;
		}
        if(NULL == this->counter){
			this->p = const_cast<T*>(orig);
			this->counter = new PtrCounter;
			this->counter->use = 1;
			atomic_xchg(&this->counter->use, 1);
			atomic_xchg(&this->counter->weak, 0);
			atomic_xchg8(&this->counter->deletable, bDel);
            atomic_xchg8(&this->counter->deleting, false);
        }else{
			if(orig == this->p) {
				atomic_inc(&this->counter->use);
				atomic_xchg8(&this->counter->deletable, bDel);
			}else {
				// reallocation ,don't delete before pointer.
				this->p = const_cast<T*>(orig);
				this->counter = new PtrCounter;
				atomic_xchg(&this->counter->use, 1);
				atomic_xchg(&this->counter->weak, 0);
				atomic_xchg8(&this->counter->deletable, bDel);
                atomic_xchg8(&this->counter->deleting, false);
			}
        }
        return;
    }

    bool operator==(T* const right) const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		return this->p == right;
    }

    bool operator!=(T* const right) const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		return this->p != right;
    }

    bool operator > (T* const right) const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		return this->p > right;
    }

    bool operator < (T* const right) const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		return this->p < right;
    }

    bool operator==(intptr_t right) const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		return (intptr_t)this->p == right;
    }

    bool operator!=(intptr_t right) const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		return (intptr_t)this->p != right;
    }

    bool operator > (intptr_t right) const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		return (intptr_t)this->p > right;
    }

    bool operator < (intptr_t right) const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		return (intptr_t)this->p < right;
    }

	T* operator->() {
		thd::CScopedReadLock scopeReadLock(rwticket);
		if(NULL != this->p){return this->p;}
		else{ throw invalid_ptr_auto_<T>("unbound Parameter");}
	}

    const T* operator->() const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		if(NULL != this->p){return this->p;}
		else{ throw invalid_ptr_auto_<T>("unbound Parameter");}
    }

	T& operator*() {
		thd::CScopedReadLock scopeReadLock(rwticket);
		if(NULL != this->p){return *this->p;}
		else{ throw invalid_ptr_auto_<T>("unbound Parameter");}
	}

    const T& operator*() const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		if(NULL != this->p){return *this->p;}
		else{ throw invalid_ptr_auto_<T>("unbound Parameter");}
    }

	bool IsInvalid() const{
		thd::CScopedReadLock scopeReadLock(rwticket);
		if(NULL == this->p) {
			return true;
		}
		return false;
	}

	long PeekUse() const {
		thd::CScopedReadLock scopeReadLock(rwticket);
		if(NULL == this->counter) {
			return 0;
		}
		return (long)this->counter->use;
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
        if(NULL != this->counter) {
            if(atomic_dec(&this->counter->use) == 0) {
                if(NULL != this->p) {
					if(this->counter->deletable) {
						atomic_xchg8(&this->counter->deleting, true);
						delete this->p;
						atomic_xchg8(&this->counter->deleting, false);
					}
                    this->p = NULL;
                }

				if(this->counter->weak < 1) {
					delete this->counter;
					this->counter = NULL;
				}
            }
        }
    }

};

}

#endif	/* AUTOPOINTER_H_ */

