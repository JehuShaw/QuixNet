#include "CondVar.h"
#include "Mutex.h"


namespace thd {

/*
 */

CCondVar::CCondVar() throw()
#if COMPILER == COMPILER_MICROSOFT
	: m_waitersCount(0)
    , m_broadcast(0)
#endif
{
#if COMPILER == COMPILER_MICROSOFT
	m_sem = ::CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
	m_waitersDone = ::CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	::pthread_cond_init(&m_cond, NULL);
#endif
}

/*
 */

CCondVar::~CCondVar() throw()
{
#if COMPILER == COMPILER_MICROSOFT
	::CloseHandle(m_waitersDone);
	::CloseHandle(m_sem);
#else
	while(::pthread_cond_destroy(&m_cond) == EBUSY) {
		::pthread_cond_broadcast(&m_cond);
		::sched_yield();
	}
#endif
}

/*
 */

bool CCondVar::Wait(CMutex& mutex, uint32_t msec) throw()
{
#if COMPILER == COMPILER_MICROSOFT

	// avoid race conditions
	m_waitersLock.Lock();
	++m_waitersCount;
	m_waitersLock.Unlock();

	// atomically release the mutex and wait on the semaphore
	// until notify() or notifyAll() is called by another thread

	mutex.Unlock();
	DWORD r = ::WaitForSingleObjectEx(m_sem, msec, FALSE);

	// require lock to avoid race conditions
	m_waitersLock.Lock();
	--m_waitersCount; // we're no longer waiting

	// if the wait timed out, just leave
	if(r == WAIT_TIMEOUT) {
		if(m_waitersCount == 0) {
			::SetEvent(m_waitersDone);
		}
		
		m_waitersLock.Unlock();
		mutex.Lock();
		return false;
	}

	// if we were last, then let all other threads proceed
	if(m_broadcast && (m_waitersCount == 0))
	{
		// atomically signal _waitersDone event and wait until we can acquire
		// the mutex; this is done to ensure fairness
		::SetEvent(m_waitersDone);
	}
	m_waitersLock.Unlock();
	mutex.Lock();
	return true;
#else
	struct timespec tspec;
	clock_gettime(CLOCK_REALTIME , &tspec);

	tspec.tv_sec += (msec / 1000);
	tspec.tv_nsec += ((msec % 1000) * 1000000);

	// assume errno == ETIMEDOUT on failure
	return (::pthread_cond_timedwait(&m_cond, &mutex.m_mutex, &tspec) == 0);
#endif
}

bool CCondVar::Wait(CMutex& mutex) throw()
{
#if COMPILER == COMPILER_MICROSOFT

	// avoid race conditions
	m_waitersLock.Lock();
	++m_waitersCount;
	m_waitersLock.Unlock();

	// atomically release the mutex and wait on the semaphore
	// until notify() or notifyAll() is called by another thread

	mutex.Unlock();
	DWORD r = ::WaitForSingleObjectEx(m_sem, INFINITE, FALSE);

	// require lock to avoid race conditions
	m_waitersLock.Lock();
	--m_waitersCount; // we're no longer waiting

	// if we were last, then let all other threads proceed
	if(m_broadcast && (m_waitersCount == 0))
	{
		// atomically signal _waitersDone event and wait until we can acquire
		// the mutex; this is done to ensure fairness
		::SetEvent(m_waitersDone);
	}
	m_waitersLock.Unlock();
	mutex.Lock();
	return true;
#else
	::pthread_cond_wait(&m_cond, &mutex.m_mutex);
	return true;
#endif
}

/*
 */

void CCondVar::Notify() throw()
{
#if COMPILER == COMPILER_MICROSOFT
	m_waitersLock.Lock();
	bool haveWaiters = (m_waitersCount > 0);
	m_waitersLock.Unlock();

	// if there aren't any waiters, this is a no-op
	if(haveWaiters) {
		::ReleaseSemaphore(m_sem, 1, 0);
	}
#else
	::pthread_cond_signal(&m_cond);
#endif
}

/*
 */

void CCondVar::NotifyAll() throw()
{
#if COMPILER == COMPILER_MICROSOFT

	// this is needed to ensure that _waitersCount and _broadcast are
	// consistent relative to each other
	
	m_waitersLock.Lock();
	bool haveWaiters = false;

	if(m_waitersCount > 0) {
		m_broadcast = true;
		haveWaiters = true;
	}

	if(haveWaiters)
	{
		// wake up all the waiters atomically

		::ReleaseSemaphore(m_sem, m_waitersCount, NULL);
		m_waitersLock.Unlock();

		// wait for all awakened threads to acquire the counting semaphore
		DWORD dwRet = 0; 
		for(;;)
		{
			if((dwRet = ::WaitForSingleObjectEx(m_waitersDone,
				INFINITE, TRUE)) != WAIT_IO_COMPLETION)
			{
				break;
			}
		}

		// this assignment is OK, even without _waitersLock held,
		// because no other waiter threads can wake up to access it
		m_broadcast = false;
	}
	else
	{
		m_waitersLock.Unlock();
	}
#else
	::pthread_cond_broadcast(&m_cond);
#endif
}


}; // namespace thd

/* end of source file */
