/*
 * File:   WeakPointer.h
 * Author: Jehu Shaw
 *
 * Created on 2013_10_2, 10:32
 */

#ifndef WEAKPOINTER_H_
#define	WEAKPOINTER_H_

#include "Common.h"
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

		if(this == &orig) {
			return;
		}
		thd::CScopedReadLock scopeReadLock(orig.rwticket);
        if(NULL != orig.counter){
            this->p = orig.p;
            this->counter = orig.counter;
			atomic_inc(&orig.counter->weak);
        }
    }

    template<class T2>
    CWeakPointer(const CWeakPointer<T2>& orig):p(NULL),counter(NULL) {

		if((intptr_t)this == (intptr_t)&orig) {
			return;
		}
		thd::CScopedReadLock scopeReadLock(orig.rwticket);
        if(IsBase(orig.p)) {
            if(NULL != orig.counter){
                this->p = dynamic_cast<T*>(orig.p);
                this->counter = orig.counter;
				atomic_inc(&orig.counter->weak);
            }
        } else if(IsChild<T2>(this->p)) {
            T* pChild = dynamic_cast<T*>(orig.p);
            if(NULL == pChild) {
                return;
            }
            if(NULL != orig.counter){
                this->p = pChild;
                this->counter = orig.counter;
				atomic_inc(&orig.counter->weak);
            }
        }
    }

	CWeakPointer(const CAutoPointer<T>& orig):p(NULL),counter(NULL) {

		thd::CScopedReadLock scopeReadLock(orig.rwticket);
		if(NULL != orig.counter){
			this->p = orig.p;
			this->counter = orig.counter;
			atomic_inc(&orig.counter->weak);
		}
	}

	template<class T2>
	CWeakPointer(const CAutoPointer<T2>& orig):p(NULL),counter(NULL) {

		thd::CScopedReadLock scopeReadLock(orig.rwticket);
		if(IsBase(orig.p)) {
			if(NULL != orig.counter){
				this->p = dynamic_cast<T*>(orig.p);
				this->counter = orig.counter;
				atomic_inc(&orig.counter->weak);
			}
		} else if(IsChild<T2>(this->p)) {
			T* pChild = dynamic_cast<T*>(orig.p);
			if(NULL == pChild) {
				return;
			}
			if(NULL != orig.counter){
				this->p = pChild;
				this->counter = orig.counter;
				atomic_inc(&orig.counter->weak);
			}
		}
	}

    //dispose
    ~CWeakPointer(){
		thd::CScopedWriteLock scopedWriteLock(rwticket);
		decr_use();
	}

    CWeakPointer& operator= (const CWeakPointer& orig){

		thd::CScopedWriteLock scopedWriteLock(rwticket);
		if(this == &orig) {
			return *this;
		}
		thd::CScopedReadLock scopeReadLock(orig.rwticket);
		if(NULL != orig.counter){
			atomic_inc(&orig.counter->weak);
		}
		decr_use();
		this->p = orig.p;
		this->counter = orig.counter;

        return *this;
    }

    template<class T2>
    CWeakPointer& operator= (const CWeakPointer<T2>& orig) {

		thd::CScopedWriteLock scopedWriteLock(rwticket);
		if((intptr_t)this == (intptr_t)&orig) {
			return *this;
		}
		thd::CScopedReadLock scopeReadLock(orig.rwticket);
        if(IsBase(orig.p)) {
			if(NULL != orig.counter){
				atomic_inc(&orig.counter->weak);
			}
			decr_use();
			this->p = dynamic_cast<T*>(orig.p);
			this->counter = orig.counter;

        } else if(IsChild<T2>(this->p)) {
            T* pChild = dynamic_cast<T*>(orig.p);
            if(NULL == pChild) {
                return *this;
            }
			if(NULL != orig.counter){
				atomic_inc(&orig.counter->weak);
			}
			decr_use();
			this->p = pChild;
			this->counter = orig.counter;

        }

        return *this;
    }


	CWeakPointer& operator= (const CAutoPointer<T>& orig){

		thd::CScopedWriteLock scopedWriteLock(rwticket);
		thd::CScopedReadLock scopeReadLock(orig.rwticket);
		if(NULL != orig.counter){
			atomic_inc(&orig.counter->weak);
		}
		decr_use();
		this->p = orig.p;
		this->counter = orig.counter;

		return *this;
	}

	template<class T2>
	CWeakPointer& operator= (const CAutoPointer<T2>& orig) {

		thd::CScopedWriteLock scopedWriteLock(rwticket);
		thd::CScopedReadLock scopeReadLock(orig.rwticket);
		if(IsBase(orig.p)) {
			if(NULL != orig.counter){
				atomic_inc(&orig.counter->weak);
			}
			decr_use();
			this->p = dynamic_cast<T*>(orig.p);
			this->counter = orig.counter;

		} else if(IsChild<T2>(this->p)) {
			T* pChild = dynamic_cast<T*>(orig.p);
			if(NULL == pChild) {
				return *this;
			}
			if(NULL != orig.counter){
				atomic_inc(&orig.counter->weak);
			}
			decr_use();
			this->p = pChild;
			this->counter = orig.counter;

		}

		return *this;
	}

    bool operator==(const CWeakPointer& right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
        return this->p == right.p;
    }
    bool operator!=(const CWeakPointer& right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
        return this->p != right.p;
    }
    bool operator> (const CWeakPointer& right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
        return this->p > right.p;
    }
    bool operator< (const CWeakPointer& right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
        return this->p < right.p;
    }

	bool operator==(const CAutoPointer<T>& right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
		return this->p == right.p;
	}
	bool operator!=(const CAutoPointer<T>& right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
		return this->p != right.p;
	}
	bool operator> (const CAutoPointer<T>& right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
		return this->p > right.p;
	}
	bool operator< (const CAutoPointer<T>& right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
		return this->p < right.p;
	}


    bool operator==(T* const right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
        return this->p == right;
    }
    bool operator!=(T* const right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
        return this->p != right;
    }
    bool operator > (T* const right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
        return this->p > right;
    }
    bool operator < (T* const right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
        return this->p < right;
    }

    bool operator==(intptr_t right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
        return (intptr_t)this->p == right;
    }
    bool operator!=(intptr_t right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
        return (intptr_t)this->p != right;
    }
    bool operator > (intptr_t right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
        return (intptr_t)this->p > right;
    }
    bool operator < (intptr_t right ) const{
		thd::CScopedReadLock scopedReadLock(rwticket);
        return (intptr_t)this->p < right;
    }

	T* operator->() {
		thd::CScopedReadLock scopedReadLock(rwticket);
		if(NULL != this->counter && this->counter->use > 0){
			return this->p;
		}
		throw invalid_ptr_weak_<T>("unbound Parameter");
	}

    const T* operator->() const{
		thd::CScopedReadLock scopedReadLock(rwticket);
		if(NULL != this->counter && this->counter->use > 0){
			return this->p;
		}
		throw invalid_ptr_weak_<T>("unbound Parameter");
    }

	T& operator*() {
		thd::CScopedReadLock scopedReadLock(rwticket);
		if(NULL != this->counter && this->counter->use > 0){
			return *this->p;
		}
		throw invalid_ptr_weak_<T>("unbound Parameter");
	}

    const T& operator*() const{
		thd::CScopedReadLock scopedReadLock(rwticket);
		if(NULL != this->counter && this->counter->use > 0){
			return *this->p;
		}
        throw invalid_ptr_weak_<T>("unbound Parameter");
    }

	inline bool IsInvalid() const{
		thd::CScopedReadLock scopedReadLock(rwticket);
		if(NULL == this->counter) {
			return true;
		}
		if(this->counter->use < 1) {
			return true;
		}
		return false;
	}

	CAutoPointer<T> GetStrong() {
		thd::CScopedReadLock scopedReadLock(rwticket);
		CAutoPointer<T> autoPointer;
		if(NULL != this->counter
			&& this->counter->use > 0) {
			autoPointer.counter = this->counter;
			autoPointer.p = this->p;

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
        if(NULL != this->counter){
            if(atomic_dec(&this->counter->weak) == 0){

				if(this->counter->use < 1 
                    && !this->counter->deleting) 
                {
					delete this->counter;
					this->counter = NULL;
				}
            }
        }
    }

};

}

#endif	/* WEAKPOINTER_H_ */

