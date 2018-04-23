/*
 * File:   ScopedPointer.h
 * Author: Jehu Shaw
 *
 * Created on 2013_10_2, 10:32
 */

#ifndef SCOPEDPOINTER_H_
#define	SCOPEDPOINTER_H_

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

		if(this == &orig) {
			return;
		}
		thd::CScopedReadLock rLock(orig.rwticket);
		if(NULL != orig.counter){
			this->p = orig.p;
			this->counter = orig.counter;
			atomic_inc(&orig.counter->weak);
		}
	}

	template<class T2>
	CScopedPointer(const CScopedPointer<T2>& orig):p(NULL),counter(NULL) {

		if((intptr_t)this == (intptr_t)&orig) {
			return;
		}
		thd::CScopedReadLock rLock(orig.rwticket);
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

	CScopedPointer(const CAutoPointer<T>& orig):p(NULL),counter(NULL) {

		thd::CScopedReadLock rLock(orig.rwticket);
		if(NULL != orig.counter){
			this->p = orig.p;
			this->counter = orig.counter;
			atomic_inc(&orig.counter->weak);
		}
	}

	template<class T2>
	CScopedPointer(const CAutoPointer<T2>& orig):p(NULL),counter(NULL) {

		thd::CScopedReadLock rLock(orig.rwticket);
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
    ~CScopedPointer(){
		thd::CScopedWriteLock wLock(rwticket);
		decr_use();
	}

	CScopedPointer& operator= (const CAutoPointer<T>& orig){

		thd::CScopedWriteLock wLock(rwticket);
		thd::CScopedReadLock rLock(orig.rwticket);
		if(NULL != orig.counter){
			atomic_inc(&orig.counter->weak);
		}
		decr_use();
		this->p = orig.p;
		this->counter = orig.counter;

		return *this;
	}

	template<class T2>
	CScopedPointer& operator= (const CAutoPointer<T2>& orig) {

		thd::CScopedWriteLock wLock(rwticket);
		thd::CScopedReadLock rLock(orig.rwticket);
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

    bool operator==(const CScopedPointer& right ) const{
		thd::CScopedReadLock rLock(rwticket);
        return this->p == right.p;
    }
    bool operator!=(const CScopedPointer& right ) const{
		thd::CScopedReadLock rLock(rwticket);
        return this->p != right.p;
    }
    bool operator> (const CScopedPointer& right ) const{
		thd::CScopedReadLock rLock(rwticket);
        return this->p > right.p;
    }
    bool operator< (const CScopedPointer& right ) const{
		thd::CScopedReadLock rLock(rwticket);
        return this->p < right.p;
    }

	bool operator==(const CAutoPointer<T>& right ) const{
		thd::CScopedReadLock rLock(rwticket);
		return this->p == right.p;
	}
	bool operator!=(const CAutoPointer<T>& right ) const{
		thd::CScopedReadLock rLock(rwticket);
		return this->p != right.p;
	}
	bool operator> (const CAutoPointer<T>& right ) const{
		thd::CScopedReadLock rLock(rwticket);
		return this->p > right.p;
	}
	bool operator< (const CAutoPointer<T>& right ) const{
		thd::CScopedReadLock rLock(rwticket);
		return this->p < right.p;
	}


    bool operator==(T* const right ) const{
		thd::CScopedReadLock rLock(rwticket);
        return this->p == right;
    }
    bool operator!=(T* const right ) const{
		thd::CScopedReadLock rLock(rwticket);
        return this->p != right;
    }
    bool operator > (T* const right ) const{
		thd::CScopedReadLock rLock(rwticket);
        return this->p > right;
    }
    bool operator < (T* const right ) const{
		thd::CScopedReadLock rLock(rwticket);
        return this->p < right;
    }

    bool operator==(intptr_t right ) const{
		thd::CScopedReadLock rLock(rwticket);
        return (intptr_t)this->p == right;
    }
    bool operator!=(intptr_t right ) const{
		thd::CScopedReadLock rLock(rwticket);
        return (intptr_t)this->p != right;
    }
    bool operator > (intptr_t right ) const{
		thd::CScopedReadLock rLock(rwticket);
        return (intptr_t)this->p > right;
    }
    bool operator < (intptr_t right ) const{
		thd::CScopedReadLock rLock(rwticket);
        return (intptr_t)this->p < right;
    }

	T* operator->() {
		thd::CScopedReadLock rLock(rwticket);
		if(NULL != this->counter && this->counter->use > 0){
			return this->p;
		}
		throw invalid_ptr_scoped_<T>("unbound Parameter");
	}

    const T* operator->() const{
		thd::CScopedReadLock rLock(rwticket);
		if(NULL != this->counter && this->counter->use > 0){
			return this->p;
		}
		throw invalid_ptr_scoped_<T>("unbound Parameter");
    }

	T& operator*() {
		thd::CScopedReadLock rLock(rwticket);
		if(NULL != this->counter && this->counter->use > 0){
			return *this->p;
		}
		throw invalid_ptr_scoped_<T>("unbound Parameter");
	}

    const T& operator*() const{
		thd::CScopedReadLock rLock(rwticket);
		if(NULL != this->counter && this->counter->use > 0){
			return *this->p;
		}
        throw invalid_ptr_scoped_<T>("unbound Parameter");
    }

	inline bool IsInvalid() const{
		thd::CScopedReadLock rLock(rwticket);
		if(NULL == this->counter) {
			return true;
		}
		if(this->counter->use < 1) {
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

private:
	CScopedPointer& operator= (const CScopedPointer& orig) { return *this; }

};

}

#endif	/* SCOPEDPOINTER_H_ */

