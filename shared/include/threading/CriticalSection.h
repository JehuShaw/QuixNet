/*
 * File:   CriticalSection.h
 * Author: Jehu Shaw
 *
 */

#ifndef __CRITICALSECTION_H_
#define __CRITICALSECTION_H_

#include "Common.h"
#include "ILock.h"
#include "AtomicLock.h"
#include "SysTickCount.h"

namespace thd {

class SHARED_DLL_DECL CCriticalSection : public ILock
{
public:

	/** Initializes a mutex class, with InitializeCriticalSection / pthread_mutex_init
	 */
	CCriticalSection()
	{
#if COMPILER == COMPILER_MICROSOFT
		InitializeCriticalSection(&m_cs);
#else
		pthread_mutexattr_t attr;
		::pthread_mutexattr_init(&attr);

#if PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_BSD
		::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#else
		::pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#endif
		::pthread_mutex_init(&m_mutex, &attr);
#endif
	}

	/** Deletes the associated critical section / mutex
	 */
	~CCriticalSection()
	{
#if COMPILER == COMPILER_MICROSOFT
		DeleteCriticalSection(&m_cs);
#else
		::pthread_mutex_destroy(&m_mutex);
#endif
	}

	/** Lock this mutex. If it cannot be acquired immediately, it will block.
	 */
	void Lock() throw()
	{
#if COMPILER == COMPILER_MICROSOFT
		EnterCriticalSection(&m_cs);
#else
		pthread_mutex_lock(&m_mutex);
#endif
	}

	bool TimedLock(uint32_t msec) throw()
	{
#if COMPILER == COMPILER_MICROSOFT
		uint64_t startTime = GetSysTickCount();
		for(int i = 0; TryEnterCriticalSection(&m_cs) != TRUE; ++i) {
			if((int64_t)(GetSysTickCount() -
				startTime) >= (int64_t)msec)
			{
				return false;
			}
			cpu_relax(i);
		}
		return true;
#else
#ifndef NO_PTHREAD_MUTEX_TIMEDLOCK
		struct timespec tspec;
		clock_gettime(CLOCK_REALTIME , &tspec);

		tspec.tv_sec += (msec / 1000);
		tspec.tv_nsec += ((msec % 1000) * 1000000);

		return(::pthread_mutex_timedlock(&m_mutex, &tspec) == 0);
#else
		uint64_t startTime = GetSysTickCount();
		for(int i = 0; pthread_mutex_trylock(&m_mutex) != 0; ++i) {
			if((int64_t)(GetSysTickCount() -
				startTime) >= (int64_t)msec)
			{
				return false;
			}
			cpu_relax(i);
		}
#endif
		return true;
#endif
	}

	/** Unlock this mutex. No error checking performed
	 */
	void Unlock() throw()
	{
#if COMPILER == COMPILER_MICROSOFT
		LeaveCriticalSection(&m_cs);
#else
		pthread_mutex_unlock(&m_mutex);
#endif
	}

	/** Attempts to acquire this mutex. If it cannot be acquired (held by another thread)
	 * it will return false.
	 * @return false if cannot be acquired, true if it was acquired.
	 */
	bool TryLock() throw()
	{
#if COMPILER == COMPILER_MICROSOFT
		return (TryEnterCriticalSection(&m_cs) == TRUE ? true : false);
#else
		return (pthread_mutex_trylock(&m_mutex) == 0);
#endif
	}

	bool Using() throw() {
		bool bUsing = !TryLock();
		if(!bUsing) {
			Unlock();
		}
		return bUsing;
	}

protected:
#if COMPILER == COMPILER_MICROSOFT
	/** Critical section used for system calls
	 */
	CRITICAL_SECTION m_cs;

#else
	/** pthread struct used in system calls
	 */
	pthread_mutex_t m_mutex;
#endif
};

}

#endif /*__CRITICALSECTION_H_*/

