/*
 * File:   Mutex.h
 * Author: Jehu Shaw
 *
 */

#ifndef __MUTEX_H_
#define __MUTEX_H_

#include "Common.h"
#include "ILock.h"

namespace thd {

class SHARED_DLL_DECL CMutex : public ILock
{
public:

	/** Initializes a mutex class, with Mutex / Event / pthread_mutex_init
	 */
	CMutex(bool recursive = false)
		: m_recursive(recursive)
	{
#if COMPILER == COMPILER_MICROSOFT
		if(m_recursive) {
			m_mutex = ::CreateMutex(NULL, FALSE, NULL);
		} else {
			m_mutex = ::CreateEvent(NULL, FALSE, TRUE, NULL);
		}
#else
		pthread_mutexattr_t attr;
		::pthread_mutexattr_init(&attr);

#if PLATFORM == PLATFORM_APPLE || PLATFORM == PLATFORM_BSD
		::pthread_mutexattr_settype(&attr, (m_recursive
			? PTHREAD_MUTEX_RECURSIVE
			: PTHREAD_MUTEX_DEFAULT));
#else
		::pthread_mutexattr_settype(&attr, (m_recursive
			? PTHREAD_MUTEX_RECURSIVE_NP
			: PTHREAD_MUTEX_TIMED_NP));
#endif
		::pthread_mutex_init(&m_mutex, &attr);
#endif
	}

	/** Deletes the associated critical section / mutex
	 */
	~CMutex()
	{
#if COMPILER == COMPILER_MICROSOFT
		if(m_mutex != NULL) {
			::CloseHandle(m_mutex);
		}
#else
		::pthread_mutex_destroy(&m_mutex);
#endif
	}

	/** Lock this mutex. If it cannot be acquired immediately, it will block.
	 */
	void Lock() throw()
	{
#if COMPILER == COMPILER_MICROSOFT
		for(;;)
		{
			if(::WaitForSingleObjectEx(m_mutex,
				INFINITE, TRUE) == WAIT_OBJECT_0)
			{
				break;
			}
		}
#else
		::pthread_mutex_lock(&m_mutex);
#endif
	}

	bool TimedLock(uint32_t msec) throw()
	{

#if COMPILER == COMPILER_MICROSOFT
		return(::WaitForSingleObjectEx(m_mutex,
			msec, TRUE) == WAIT_OBJECT_0);
#else

#ifndef NO_PTHREAD_MUTEX_TIMEDLOCK
		struct timespec tspec;
		clock_gettime(CLOCK_REALTIME , &tspec);

		tspec.tv_sec += (msec / 1000);
		tspec.tv_nsec += ((msec % 1000) * 1000000);

		return(::pthread_mutex_timedlock(&m_mutex, &tspec) == 0);
#else
		return(::pthread_mutex_trylock(&m_mutex) == 0);
#endif // NO_PTHREAD_MUTEX_TIMEDLOCK

#endif // COMPILER == COMPILER_MICROSOFT
	}

	/** Unlock this mutex. No error checking performed
	 */
	void Unlock() throw()
	{
#if COMPILER == COMPILER_MICROSOFT
		if(m_recursive) {
			::ReleaseMutex(m_mutex);
		} else {
			::SetEvent(m_mutex);
		}
#else
		::pthread_mutex_unlock(&m_mutex);
#endif
	}

	/** Attempts to acquire this mutex. If it cannot be acquired (held by another thread)
	 * it will return false.
	 * @return false if cannot be acquired, true if it was acquired.
	 */
	bool TryLock() throw()
	{
#if COMPILER == COMPILER_MICROSOFT
		return (::WaitForSingleObjectEx(m_mutex,
			0, TRUE) == WAIT_OBJECT_0);
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
	friend class CCondVar;
#if COMPILER == COMPILER_MICROSOFT
	/** Handle used for system calls
	 */
	HANDLE m_mutex;
#else
	/** pthread struct used in system calls
	 */
	pthread_mutex_t m_mutex;
#endif
	bool m_recursive;

private:
	CMutex(const CMutex& orig) {}
	CMutex& operator=(const CMutex& right) { return *this; }
};

}

#endif /*__MUTEX_H_ */

