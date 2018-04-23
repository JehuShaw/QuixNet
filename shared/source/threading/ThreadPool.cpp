#include "ThreadPool.h"
#include "Log.h"
#include "CircleQueue.h"
#include "SysTickCount.h"

using namespace util;

#ifdef WIN32
#include <process.h>
#endif

namespace thd {

inline static int32_t AtomicDecUntilZero(volatile int32_t* pValue) {
	int32_t Old;
	int32_t New;
	do {
		Old = *pValue;
		if(Old > 0) {
			New = Old - 1;
		} else {
			return Old;
		}
	} while (atomic_cmpxchg(pValue, New, Old) != Old);
	return Old;
}

union TicketLockTest
{
	uint32_t u;
	struct
	{
		uint16_t ticket;
		uint16_t users;
	} s;
};

uint64_t Thread::lastAddTime = 0;

SHARED_DLL_DECL CThreadPool ThreadPool;

CThreadPool::CThreadPool()
	: m_reserveSize(0)
	, m_thdsToExit(0)
	, m_thdsEaten(0)
	, m_bShutdown(false)
{
	m_pActiveLock = new CSpinRWLock;
	m_pFreeLock = new CSpinRWLock;
	m_ptbQueue = new CCircleQueue<ThreadBase *>;
}

CThreadPool::~CThreadPool()
{
	delete m_ptbQueue;
	m_ptbQueue = NULL;
	delete m_pFreeLock;
	m_pFreeLock = NULL;
	delete m_pActiveLock;
	m_pActiveLock = NULL;
}

bool CThreadPool::ThreadExit(Thread * t)
{
	// do we have to kill off some threads?
	if(AtomicDecUntilZero(&m_thdsToExit) > 0) {
		// kill us.
		if(THREAD_STATUS_FREE == t->thdStatus) {
			CScopedWriteLock wrLock(*m_pFreeLock);
			m_freeThreads.erase(t);
		} else if(THREAD_STATUS_ACTIVE == t->thdStatus) {
			EraseActiveThread(t);
		}
		delete t;
		return false;
	}

	// timeout auto remove
	if(THREAD_STATUS_TIMEOUT == t->thdStatus) {
		delete t;
		return false;
	}

	// enter the "suspended" pool
	atomic_inc(&m_thdsEaten);

	bool bExist = false;
	if(true) {
		CScopedWriteLock wrLock(*m_pFreeLock);

		// we're definitely no longer active
		EraseActiveThread(t);
		uint64_t curTime = GetSysTickCount();
		t->startTime = curTime;
		Thread::lastAddTime = curTime;
		atomic_xchg8(&t->thdStatus, THREAD_STATUS_FREE);
		std::pair<ThreadFreeSet::iterator, bool> pairIB(m_freeThreads.insert(t));
		if(!pairIB.second) {
			bExist = true;
		}
		uint32_t curThreadCount = m_freeThreads.size() + GetActiveThreadCount();
		if(curThreadCount > m_reserveSize) {
			ThreadFreeSet::const_iterator lastIt(m_freeThreads.end());
			--lastIt;
			Thread* lastThd = *lastIt;
			if(t != lastThd && (curTime - lastThd->startTime) > THREAD_FREE_TIMEOUT) {
				m_freeThreads.erase(lastIt);
				atomic_xchg8(&lastThd->thdStatus, THREAD_STATUS_TIMEOUT);
				if(!lastThd->thdController.Resume()) {
					atomic_xchg8(&lastThd->thdStatus, THREAD_STATUS_FREE);
					m_freeThreads.insert(m_freeThreads.end(), lastThd);
				}
			}
		}
	}

	if(bExist) {
		OutputError("Thread %u duplicated", t->thdController.GetId());
	}
#ifdef OPEN_THREAD_POOL_DEBUG
	OutputDebug("Thread %u entered the free pool.", t->thdController.GetId());
#endif
	return true;
}

Thread * CThreadPool::ActivateFromFree() throw()
{
	CScopedWriteLock wtLock(*m_pFreeLock);

	if(m_freeThreads.empty()) {
		return NULL;
	}
	Thread * t = NULL;
	ThreadFreeSet::iterator itF(m_freeThreads.begin());
	while(m_freeThreads.end() != itF) {

		t = *itF;
		if(NULL == t) {
			m_freeThreads.erase(itF++);
			continue;
		}
		// execute the task on this thread.
		t->exeTarget = NULL;
		// resume the thread, and it should start working.
		if(t->thdController.Resume()) {
			atomic_dec(&m_thdsEaten);

			m_freeThreads.erase(itF++);
			InsertActiveThread(t);

			return t;
		}
		++itF;
	}
	return NULL;
}

void CThreadPool::ExecuteTask(ThreadBase * exeTarget)
{
	if(m_bShutdown) {
		return;
	}

	if(NULL != exeTarget) {
		thd::ThreadBase** pPTB = m_ptbQueue->WriteLock();
		*pPTB = exeTarget;
		m_ptbQueue->WriteUnlock();
	}

    Thread * t = ActivateFromFree();
	// grab one from the pool, if we have any.
	if(NULL == t)
	{
		uint32_t threadCount = GetActiveThreadCount() + GetFreeThreadCount();
		if(threadCount >= THREAD_MAX_SIZE) {
			return;
		}
		// creating a new thread means it heads straight to its task.
		// no need to resume it :)
		t = StartThread();
	}
	else
	{
#ifdef OPEN_THREAD_POOL_DEBUG
		OutputDebug("Thread %u left the thread pool.", t->thdController.GetId());
#endif
	}

    if(NULL != t) {
#ifdef OPEN_THREAD_POOL_DEBUG
	// add the thread to the active set
#ifdef WIN32
	OutputDebug("Thread %u is now executing task at 0x%p.", t->thdController.GetId(), exeTarget);
#else
	OutputDebug("Thread %u is now executing task at %p.", t->thdController.GetId(), exeTarget);
#endif
#endif
    }

}

void CThreadPool::Startup(int tCount/* = THREAD_RESERVE*/)
{
	int32_t def = tCount - m_reserveSize;
	m_reserveSize = tCount;

	memory_barrier();

	for(int i= 0; i < def; ++i) {
		StartThread();
	}
#ifdef OPEN_THREAD_POOL_DEBUG
	OutputDebug("Startup, launched %u threads.", tCount);
#endif
}

void CThreadPool::ShowStats()
{
	uint32_t activeCount = GetActiveThreadCount();
	uint32_t freeCount = GetFreeThreadCount();
	uint32_t requestCount = m_ptbQueue->Size() + activeCount;
	uint32_t existCount = activeCount + freeCount;
	float rate = 1.0f;
	if(existCount > 0) {
		rate = (float)requestCount * 100.0f / (float)existCount;
	}
	PrintDebug("============ ThreadPool Status =============");
	PrintDebug("Active Threads: %u", activeCount);
	PrintDebug("Suspended Threads: %u", freeCount);
	PrintDebug("Requested-To-Existent Ratio: %.3f%% (%u/%u)", rate, requestCount, existCount);
	PrintDebug("Eaten Count: %d (negative is bad!)", m_thdsEaten);
	PrintDebug("============================================");
}

void CThreadPool::KillFreeThreads(const uint32_t count)
{
#ifdef OPEN_THREAD_POOL_DEBUG
	OutputDebug("Killing %u excess threads.", count);
#endif

	CScopedReadLock rdLock(*m_pFreeLock);
	Thread * t = NULL;
	ThreadFreeSet::const_iterator itr(m_freeThreads.begin());
	ThreadFreeSet::const_iterator end(m_freeThreads.end());
	for(uint32_t i = 0; i < count && end != itr; ++i, ++itr)
	{
		t = *itr;
		//t->exeTarget = NULL;
		atomic_inc(&m_thdsToExit);
		t->thdController.Resume();
	}
}

void CThreadPool::Shutdown()
{
	atomic_xchg8(&m_bShutdown, true);

	// exit all
#ifdef OPEN_THREAD_POOL_DEBUG
	OutputDebug("Shutting down %u threads.", GetActiveThreadCount() + GetFreeThreadCount());
#endif
	KillFreeThreads(GetFreeThreadCount());
	atomic_xadd(&m_thdsToExit, GetActiveThreadCount());

	if(true) {
		CScopedReadLock rdLock(*m_pActiveLock);
		ThreadActiveSet::iterator itr(m_activeThreads.begin());
		for(; itr != m_activeThreads.end(); ++itr)
		{
			Thread *t = *itr;
			if(t->exeTarget) {
				t->exeTarget->OnShutdown();
			} else {
				t->thdController.Resume();
			}
		}
	}// scoped

	for(int i = 0;; ++i) {

		bool IsActiveNoEmpty = !IsActiveThreadEmpty();
		bool IsFreeNoEmpty = !IsFreeThreadEmpty();
		if(IsActiveNoEmpty || IsFreeNoEmpty)
		{
			if(IsFreeNoEmpty)
			{
				/*if we are here then a thread in the free pool checked if it was being shut down just before CThreadPool::Shutdown() was called,
				but called Suspend() just after KillFreeThreads(). All we need to do is to resume it.*/
				CScopedReadLock rdLock(*m_pFreeLock);
				Thread * t = NULL;
				atomic_cmpxchg(&m_thdsToExit, (int32_t)m_freeThreads.size(), 0);
				ThreadFreeSet::const_iterator itr(m_freeThreads.begin());
				for(; itr != m_freeThreads.end(); ++itr)
				{
					t = *itr;
					t->thdController.Resume();
				}
			}
#ifdef OPEN_THREAD_POOL_DEBUG
			OutputDebug("%u active and %u free threads remaining...",
                GetActiveThreadCount(), GetFreeThreadCount());
#endif
            Sleep(1);
			continue;
		}
		break;
	}
}

// gets active thread count
uint32_t CThreadPool::GetActiveThreadCount() {
	CScopedReadLock rdLock(*m_pActiveLock);
	return (uint32_t)m_activeThreads.size();
}

// gets free thread count
uint32_t CThreadPool::GetFreeThreadCount() {
	CScopedReadLock rdLock(*m_pFreeLock);
	return (uint32_t)m_freeThreads.size();
}

void CThreadPool::InsertActiveThread(Thread* t)
{
	CScopedWriteLock wtLock(*m_pActiveLock);
	atomic_xchg8(&t->thdStatus, THREAD_STATUS_ACTIVE);
	m_activeThreads.insert(t);
}

void CThreadPool::EraseActiveThread(Thread* t) {
	CScopedWriteLock wtLock(*m_pActiveLock);
	m_activeThreads.erase(t);
}

Thread * CThreadPool::StartThreadEx(ThreadBase* exeTarget) {

	if(NULL != exeTarget) {
		ThreadBase** pPTB = m_ptbQueue->WriteLock();
		*pPTB = exeTarget;
		m_ptbQueue->WriteUnlock();
	}

	return StartThread();
}

bool CThreadPool::IsActiveThreadEmpty() {
	CScopedReadLock rdLock(*m_pActiveLock);
	return m_activeThreads.empty();
}

bool CThreadPool::IsFreeThreadEmpty() {
	CScopedReadLock rdLock(*m_pFreeLock);
	return m_freeThreads.empty();
}

/* this is the only platform-specific code. neat, huh! */
#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ )

unsigned long WINAPI CThreadPool::thread_proc(void* param)
{
	Thread * t = (Thread*)param;
	// wait for init
	for(int i = 0; TRUE == t->lockFlag; ++i) {
		cpu_relax(i);
	}

	atomic_dec(&thd::ThreadPool.m_thdsEaten);
	thd::ThreadPool.InsertActiveThread(t);

	ThreadBase ** pPTB = NULL;
    uint32_t tid = t->thdController.GetId();
    bool ht = (t->exeTarget != NULL);

#ifdef OPEN_THREAD_POOL_DEBUG
	OutputDebug("Thread %u started.", t->thdController.GetId());
#endif
	for(;;)
	{
		pPTB = ThreadPool.m_ptbQueue->ReadLock();
		if(NULL != pPTB) {
			t->exeTarget = *pPTB;
			ThreadPool.m_ptbQueue->ReadUnlock();
		}

		if(t->exeTarget != NULL)
		{
			if(t->exeTarget->OnRun()) {
				delete t->exeTarget;
			}
			t->exeTarget = NULL;

		}

		if(ThreadPool.m_ptbQueue->Size() > 0) {
			continue;
		}

		if(!thd::ThreadPool.ThreadExit(t))
		{
#ifdef OPEN_THREAD_POOL_DEBUG
			OutputDebug("Thread %u exiting.", tid);
#endif
			break;
		}
		else
		{
#ifdef OPEN_THREAD_POOL_DEBUG
			if(ht) {
				OutputDebug("Thread %u waiting for a new task.", tid);
			}
#endif
			// enter "suspended" state. when we return, the threadpool will either tell us to fuk off, or to execute a new task.
			t->thdController.Suspend();
			// after resuming, this is where we will end up. start the loop again, check for tasks, then go back to the threadpool.
		}
	}

	// at this point the t pointer has already been freed, so we can just cleanly exit.
	ExitThread(0);
}

Thread * CThreadPool::StartThread() throw()
{
	if(m_bShutdown) {
		return NULL;
	}

	Thread * t = new Thread;
	t->thdStatus = THREAD_STATUS_ACTIVE;
	t->exeTarget = NULL;
	atomic_xchg8(&t->lockFlag, TRUE);

	HANDLE hThread = CreateThread(NULL, 0,
		&thread_proc, (LPVOID)t, 0, NULL);

	if(NULL != hThread) {
		t->thdController.Setup(hThread);
		atomic_xchg8(&t->lockFlag, FALSE);
		return t;
	} else {
		delete t;
		return NULL;
	}
}

#else

void * CThreadPool::thread_proc(void * param)
{
	Thread * t = (Thread*)param;
	// wait for init
	for(int i = 0; TRUE == t->lockFlag; ++i) {
		cpu_relax(i);
	}

	atomic_dec(&thd::ThreadPool.m_thdsEaten);
	thd::ThreadPool.InsertActiveThread(t);

	ThreadBase ** pPTB = NULL;
	uint32_t tid = t->thdController.GetId();

#ifdef OPEN_THREAD_POOL_DEBUG
	    Log.Debug("ThreadPool", "Thread %u started.", tid);
#endif

	for(;;)
	{
		pPTB = ThreadPool.m_ptbQueue->ReadLock();
		if(NULL != pPTB) {
			t->exeTarget = *pPTB;
			ThreadPool.m_ptbQueue->ReadUnlock();
		}

		if(t->exeTarget != NULL)
		{
			if(t->exeTarget->OnRun()) {
				delete t->exeTarget;
			}
			t->exeTarget = NULL;
		}

		if(ThreadPool.m_ptbQueue->Size() > 0) {
			continue;
		}

		if(!ThreadPool.ThreadExit(t)) {
			break;
		} else {
			// enter "suspended" state. when we return, the threadpool will either tell us to fuk off, or to execute a new task.
			t->thdController.Suspend();
			// after resuming, this is where we will end up. start the loop again, check for tasks, then go back to the threadpool.
		}
	}
	pthread_exit(0);
}

Thread * CThreadPool::StartThread() throw()
{
	if(m_bShutdown) {
		return NULL;
	}

	Thread * t = new Thread;
	t->thdStatus = THREAD_STATUS_ACTIVE;
	t->exeTarget = NULL;
	atomic_xchg8(&t->lockFlag, TRUE);

	pthread_t target;
    if(pthread_create(&target, NULL, &thread_proc, (void*)t) == 0) {
		t->thdController.Setup(target);
		atomic_xchg8(&t->lockFlag, FALSE);
		pthread_detach(target);
		return t;
	} else {
		delete t;
		return NULL;
	}
}

#endif

} // end namespace thd
