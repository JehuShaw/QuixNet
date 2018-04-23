/*
 * File:   ThreadPool.h
 * Author: Jehu Shaw
 *
 */

#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include "Common.h"

namespace thd {

#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ )

class SHARED_DLL_DECL ThreadController
{
	HANDLE m_hThread;
    HANDLE m_hSema;
public:
	ThreadController() : m_hThread(NULL), m_hSema(NULL) {}

	void Setup(HANDLE hThread)
	{
		m_hThread = hThread;
        m_hSema = CreateSemaphore(NULL, 0, 2147483647, NULL);
	}

	void Suspend()
	{
		// We can't be suspended by someone else. That is a big-no-no and will lead to crashes.
		ASSERT(GetCurrentThreadId() == GetThreadId(m_hThread));

        WaitForSingleObject(m_hSema, INFINITE);
	}

	bool Resume()
	{
		// This SHOULD be called by someone else.
		ASSERT(GetCurrentThreadId() != GetThreadId(m_hThread));

		return ReleaseSemaphore(m_hSema, 1, NULL) != FALSE;
	}

	void Join()
	{
		WaitForSingleObject(m_hThread, INFINITE);
	}

	uint32_t GetId() 
	{ 
		return GetThreadId(m_hThread); 
	}
};

#else
#ifndef HAVE_DARWIN
#include <semaphore.h>

class ThreadController
{
	sem_t m_hSema;
	pthread_t m_hThread;
public:

	ThreadController() 
	{
#if PLATFORM != PLATFORM_APPLE
		ASSERT(sizeof(m_hThread) <= 32);
#endif
	}

	~ThreadController()
	{
		sem_destroy(&m_hSema);
	}

	void Setup(pthread_t h)
	{
		m_hThread = h;
		sem_init(&m_hSema, PTHREAD_PROCESS_PRIVATE, 0);
	}

	void Suspend()
	{
		ASSERT(pthread_equal(pthread_self(), m_hThread));
		sem_wait(&m_hSema);
	}

	bool Resume()
	{
		ASSERT(!pthread_equal(pthread_self(), m_hThread));
		return sem_post(&m_hSema) == 0;
	}

	void Join()
	{
		// waits until the thread finishes then returns
		pthread_join(m_hThread, NULL);
	}

	uint32_t GetId() 
	{ 
#if PLATFORM == PLATFORM_APPLE
		return (uint32_t)pthread_mach_thread_np(m_hThread);
#else
		return (uint32_t)m_hThread;
#endif
	}
};

#else

class ThreadController
{
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	pthread_t m_hThread;
public:
	void Setup(pthread_t h)
	{
		m_hThread = h;
		pthread_mutex_init(&mutex,NULL);
		pthread_cond_init(&cond,NULL);
	}
	~ThreadController()
	{
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&cond);
	}
	void Suspend()
	{
		pthread_cond_wait(&cond, &mutex);
	}
	bool Resume()
	{
		return pthread_cond_signal(&cond) == 0;
	}
	void Join()
	{
		pthread_join(m_hThread,NULL);
	}
	INLINE uint32_t GetId() { 
#if PLATFORM == PLATFORM_APPLE
		return (uint32_t)pthread_mach_thread_np(m_hThread);
#else
		return (uint32_t)m_hThread;
#endif
	}
};

#endif

#endif

struct SHARED_DLL_DECL Thread
{	
	ThreadController thdController;

private:
	friend class CThreadPool;
	friend struct ThreadTimeCompare;
	ThreadBase * exeTarget;
	static uint64_t lastAddTime;
	uint64_t startTime;
	volatile char thdStatus;
	volatile char lockFlag;
};

struct ThreadTimeCompare {
	bool operator()(const Thread* pThread1
		, const Thread* pThread2)const
	{
		if(pThread1 == pThread2) {
			return false;
		}
		uint64_t lastAddTime = Thread::lastAddTime;
		int64_t ll = lastAddTime - pThread1->startTime;
		int64_t rl = lastAddTime - pThread2->startTime;
		if(ll < rl) {
			return true;
		} else if(ll == rl) {
			if(pThread1 < pThread2) {
				return true;
			}
		}
		return false;
	}
};

typedef std::set<Thread*> ThreadActiveSet;
typedef std::set<Thread*, struct ThreadTimeCompare> ThreadFreeSet;
#define DEFAULT_THREAD_RESERVE_SIZE 10
#define THREAD_MAX_SIZE 1008
// microsecond
#define THREAD_FREE_TIMEOUT 300000

#define THREAD_STATUS_ACTIVE 0
#define THREAD_STATUS_FREE 1
#define THREAD_STATUS_TIMEOUT 2

template<class T>
class CCircleQueue;

class CSpinRWLock;

class SHARED_DLL_DECL CThreadPool
{
	uint32_t m_reserveSize;
	volatile uint32_t m_thdsRequested;
	volatile int32_t m_thdsToExit;
	volatile int32_t m_thdsEaten;
	volatile bool m_bShutdown;

	//CMutex m_mutex;
    ThreadActiveSet m_activeThreads;
	ThreadFreeSet m_freeThreads;
	CSpinRWLock* m_pActiveLock;
	CSpinRWLock* m_pFreeLock;

	CCircleQueue<ThreadBase*>* m_ptbQueue;

public:
	CThreadPool();

	~CThreadPool();

	// call at startup
	void Startup(int tCount = DEFAULT_THREAD_RESERVE_SIZE);

	// shutdown all threads
	void Shutdown();

	// grabs/spawns a thread, and tells it to execute a task.
	void ExecuteTask(ThreadBase * exeTarget);

	// prints some neat debug stats
	void ShowStats();

	// kills x free threads
	void KillFreeThreads(const uint32_t count);

	// gets active thread count
	uint32_t GetActiveThreadCount();

	// gets free thread count
	uint32_t GetFreeThreadCount();

	// creates a thread, returns a handle to it.
	Thread * StartThreadEx(ThreadBase * exeTarget);

private:
	// return true - suspend ourselves, and wait for a future task.
	// return false - exit, we're shutting down or no longer needed.
	bool ThreadExit(Thread * t);

	Thread * StartThread() throw();

	Thread * ActivateFromFree() throw();

	bool IsActiveThreadEmpty();

	bool IsFreeThreadEmpty();

	void InsertActiveThread(Thread* t);

	void EraseActiveThread(Thread* t);

#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ )
	static unsigned long WINAPI thread_proc(void* param);
#else
	static void * thread_proc(void * param);
#endif
};

extern SHARED_DLL_DECL CThreadPool ThreadPool;

}

#endif
