/*
 * File:   Singleton.h
 * Author: Jehu Shaw
 *
 */

#ifndef _SERVER_SINGLETON_H
#define _SERVER_SINGLETON_H

#include <assert.h>
#include "SpinLock.h"
#include "ScopedLock.h"
#include "AutoPointer.h"

namespace util {

template <class T>
class Singleton
{
public:
	typedef CAutoPointer<T> PTR_T;

	static PTR_T Pointer();

	static void Release();
protected:

	Singleton(void){ if(s_instance != NULL) { assert(false); int* p = NULL; *p = 0;} }

	~Singleton(void){}

	Singleton(const Singleton&){}

	Singleton & operator= (const Singleton &){ return *this; }

	static CAutoPointer<T> s_instance;
	static thd::CSpinLock s_lock; 
};

template <class T>
CAutoPointer<T> Singleton<T>::s_instance;

template <class T>
thd::CSpinLock Singleton<T>::s_lock; 

template <class T>
typename Singleton<T>::PTR_T Singleton<T>::Pointer()
{
	if(s_instance == NULL) {
		thd::CScopedLock scopedLock(s_lock);
		if(s_instance == NULL) {
			s_instance.SetRawPointer(new T);
		}
	}
	return s_instance;
}

template <class T>
void Singleton<T>::Release()
{
	thd::CScopedLock scopedLock(s_lock);
	if(s_instance != NULL) {
		s_instance.SetRawPointer(NULL);
	}
}

}

#endif
