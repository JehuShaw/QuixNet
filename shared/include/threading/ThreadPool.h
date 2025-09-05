/*
 * File:   ThreadPool.h
 * Author: Jehu Shaw
 *
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "ThreadController.h"

namespace thd {


struct SHARED_DLL_DECL Thread
{	
	ThreadController thdController;
	virtual ~Thread() {}
private:
	friend class CThreadPool;
	ThreadBase* exeTarget;
	uint64_t idleTime;
	int32_t index;
	volatile char thdStatus;
	volatile char initFlag;
};


#define DEFAULT_THREAD_RESERVE_SIZE 10
#define THREAD_MAX_SIZE 1008
// microsecond
#define THREAD_FREE_TIMEOUT 180000

#define THREAD_STATUS_ACTIVE 0
#define THREAD_STATUS_FREE 1
#define THREAD_STATUS_TIMEOUT 2

template<typename DataType, int NodeKeepTimeMS, int NodeKeepSize>
class CCircleQueue;

class CSpinLock;

class SHARED_DLL_DECL CThreadPool
{
private:
	int32_t m_reserveSize;
	volatile int32_t m_thdsToExit;
	volatile int32_t m_thdsEaten;
	volatile bool m_bShutdown;

	// use size
	int32_t m_nCurSize;
	// allocation offset
	int32_t m_nOffset;
	// m_arrThreads lock
	CSpinLock* m_pArrLock;
	// Thread list
	Thread* m_arrThreads[THREAD_MAX_SIZE];


	CCircleQueue<ThreadBase*, 128, 300000>* m_pTbQueue;

public:

	CThreadPool();

	~CThreadPool();

	// call at startup
	void Startup(int32_t nCount = DEFAULT_THREAD_RESERVE_SIZE);

	// shutdown all threads
	void Shutdown();

	// grabs/spawns a thread, and tells it to execute a task.
	void ExecuteTask(ThreadBase * exeTarget);

	// prints some neat debug stats
	void ShowStats();

	// kills x idle threads
	void KillIdleThreads(int32_t count);

	// gets active thread count
	int32_t GetActiveThreadCount();

	// gets free thread count
	int32_t GetIdleThreadCount();

	// gets current thread count
	int32_t GetThreadCount();

	// creates a thread, returns a handle of it.
	Thread* StartThreadEx(ThreadBase * exeTarget);

private:
	// return true - suspend ourselves, and wait for a future task.
	// return false - exit, we're shutting down or no longer needed.
	bool ThreadExit(Thread* t);

	Thread* StartThread() throw();

	Thread* ActivateFromIdle() throw();

	bool InsertActiveThread(Thread* t);

	void GetActiveThreads(std::vector< Thread* >& vecThreads);

	void EraseThread(Thread* t);

	void EraseIdleThread(Thread* t);


#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ ) || defined( _WIN64 )
	static unsigned long WINAPI thread_proc(void* param);
#else
	static void * thread_proc(void * param);
#endif
};

extern SHARED_DLL_DECL CThreadPool ThreadPool;

}

#endif // THREADPOOL_H
