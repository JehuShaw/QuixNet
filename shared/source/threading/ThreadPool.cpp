#include "ThreadPool.h"
#include "Log.h"
#include "SpinLock.h"
#include "ScopedLock.h"
#include "CircleQueue.h"
#include "SysTickCount.h"
#include "ThreadIndexManager.h"
#include "PoolBase.h"

using namespace util;

#ifdef WIN32
#include <process.h>
#endif

#define MAX_IDLE_TIME 0xFFFFFFFFFFFFFFFF

namespace thd {

	inline static int32_t AtomicDecUntilZero(volatile int32_t* pValue) {
		int32_t Old;
		int32_t New;
		do {
			Old = *pValue;
			if (Old > 0) {
				New = Old - 1;
			} else {
				return Old;
			}
		} while (atomic_cmpxchg(pValue, Old, New) != Old);
		return Old;
	}

SHARED_DLL_DECL CThreadPool ThreadPool;

CThreadPool::CThreadPool()
	: m_reserveSize(0)
	, m_thdsToExit(0)
	, m_thdsEaten(0)
	, m_bShutdown(false)
	, m_nCurSize(0)
    , m_nOffset(-1)
{
	m_pArrLock = new CSpinLock;
//	m_arrThreads = new Thread *[THREAD_MAX_SIZE];
	
	m_pTbQueue = new CCircleQueue<ThreadBase*, 128>;
}

	CThreadPool::~CThreadPool()
	{
		delete m_pTbQueue;
		m_pTbQueue = NULL;

	//	delete[] m_arrThreads;
		delete m_pArrLock;
	}

	bool CThreadPool::ThreadExit(Thread* t)
	{
		// Is there a thread that needs to be deleted?
		if (AtomicDecUntilZero(&m_thdsToExit) > 0
			|| THREAD_STATUS_TIMEOUT == t->thdStatus) {
			// Remove the STL of the current thread
			CThreadIndexManager::Pointer()->Remove();
			// remove self.
			atomic_inc(&m_thdsEaten);
			EraseThread(t);
#ifdef OPEN_THREAD_POOL_DEBUG
			if (THREAD_STATUS_TIMEOUT == t->thdStatus) {
				OutputDebug("Thread %u timeout erase. thdStatus = %d", t->thdController.GetId(), t->thdStatus);
			} else {
				OutputDebug("Thread %u thdsToExit erase. thdStatus = %d", t->thdController.GetId(), t->thdStatus);
			}
#endif
			delete t;
			return true;
		}

		// enter the "suspended" pool
		atomic_inc(&m_thdsEaten);
		atomic_xchg8(&t->thdStatus, THREAD_STATUS_FREE);
		do {
			CScopedLock lock(*m_pArrLock);
			uint64_t curTime = GetSysTickCount();
			// Move to idle area
			if (t->index > -1 && t->index <= m_nOffset) {
				if (t->index != m_nOffset) {
					m_arrThreads[t->index] = m_arrThreads[m_nOffset];
					m_arrThreads[m_nOffset]->index = t->index;
					t->index = m_nOffset;
					m_arrThreads[m_nOffset] = t;
				}
				t->idleTime = curTime;
				--m_nOffset;
			} else {
				OutputError("CThreadPool::ThreadExit  No found !  index = %d m_nOffset = %d m_nCurSize = %d ", t->index, m_nOffset, m_nCurSize);
			}
			// Check timeout
			int32_t lastIndex = m_nCurSize - 1;
			if (m_nCurSize > m_reserveSize && m_nOffset < lastIndex) {
				Thread* lastThd = m_arrThreads[lastIndex];
				if (curTime - lastThd->idleTime > THREAD_FREE_TIMEOUT) {
					atomic_xchg8(&lastThd->thdStatus, THREAD_STATUS_TIMEOUT);
					if (!lastThd->thdController.Resume()) {
					    atomic_xchg8(&lastThd->thdStatus, THREAD_STATUS_FREE);
					}
				}
			}
		} while (false);

#ifdef OPEN_THREAD_POOL_DEBUG
		OutputDebug("Thread %u entered the free pool. thdStatus = %d", t->thdController.GetId(), t->thdStatus);
#endif
		return false;
	}

	Thread* CThreadPool::ActivateFromIdle() throw()
	{
		CScopedLock lock(*m_pArrLock);
		for (int32_t i = m_nCurSize - 1; i > m_nOffset; --i) {
			Thread* t = m_arrThreads[i];
			if (THREAD_STATUS_FREE != t->thdStatus) {
				continue;
			}
			// resume the thread, and it should start working.
			if (t->thdController.Resume()) {
				return t;
			}
		}
		// empty array
		return NULL;
	}

	void CThreadPool::ExecuteTask(ThreadBase * exeTarget)
	{
		if (m_bShutdown) {
			return;
		}

		if (NULL != exeTarget) {
			ThreadBase** ppThdBase = m_pTbQueue->WriteLock();
			*ppThdBase = exeTarget;
			m_pTbQueue->WriteUnlock();
		}

		Thread* t = ActivateFromIdle();
		// grab one from the pool, if we have any.
		if (NULL == t)
		{
			if (GetThreadCount() >= THREAD_MAX_SIZE) {
				return;
			}
			// creating a new thread means it heads straight to its task.
			// no need to resume it :)
			t = StartThread();
		}
#ifdef OPEN_THREAD_POOL_DEBUG
		else {
			OutputDebug("Thread %u left the thread pool.", t->thdController.GetId());
		}

		if (NULL != t) {
			// add the thread to the active set
#ifdef WIN32
			OutputDebug("Thread %u is now executing task at 0x%p.", t->thdController.GetId(), exeTarget);
#else
			OutputDebug("Thread %u is now executing task at %p.", t->thdController.GetId(), exeTarget);
#endif
		}
#endif

	}

	void CThreadPool::Startup(int32_t nCount/* = THREAD_RESERVE*/)
	{
		if (nCount >= THREAD_MAX_SIZE) {
			OutputError("CThreadPool::Startup  No found !  nCount = %d THREAD_MAX_SIZE = %d ", nCount, THREAD_MAX_SIZE);
			return;
		}

		int32_t def = nCount - m_reserveSize;
		m_reserveSize = nCount;

		memory_barrier();

		for (int i = 0; i < def; ++i) {
			StartThread();
		}
#ifdef OPEN_THREAD_POOL_DEBUG
		OutputDebug("Startup, launched %u threads.", nCount);
#endif
	}

	void CThreadPool::ShowStats()
	{
		uint32_t activeCount = GetActiveThreadCount();
		uint32_t freeCount = GetIdleThreadCount();
		uint32_t requestCount = m_pTbQueue->Size() + activeCount;
		uint32_t existCount = activeCount + freeCount;
		float rate = 1.0f;
		if (existCount > 0) {
			rate = (float)requestCount * 100.0f / (float)existCount;
		}
		PrintDebug("============ ThreadPool Status =============");
		PrintDebug("Active Threads: %u", activeCount);
		PrintDebug("Suspended Threads: %u", freeCount);
		PrintDebug("Requested-To-Existent Ratio: %.3f%% (%u/%u)", rate, requestCount, existCount);
		PrintDebug("Eaten Count: %d (negative is bad!)", m_thdsEaten);
		PrintDebug("============================================");
	}

	void CThreadPool::KillIdleThreads(int32_t count)
	{
#ifdef OPEN_THREAD_POOL_DEBUG
		OutputDebug("Killing %u excess threads.", count);
#endif

		CScopedLock lock(*m_pArrLock);
		for (int32_t i = m_nOffset + 1,j = 0; j < count && i < m_nCurSize; ++i) {
			Thread* t = m_arrThreads[i];
			atomic_inc(&m_thdsToExit);
			// resume the thread, and it should start working.
			if (t->thdController.Resume()) {
				++j;
				continue;
			}
		}
	}

	void CThreadPool::Shutdown()
	{
		atomic_xchg8(&m_bShutdown, true);

		// exit all
#ifdef OPEN_THREAD_POOL_DEBUG
		OutputDebug("Shutting down %u threads.", GetActiveThreadCount() + GetIdleThreadCount());
#endif
		KillIdleThreads(GetIdleThreadCount());
		atomic_xadd(&m_thdsToExit, GetActiveThreadCount());

		std::vector< Thread* > vecThreads;
		GetActiveThreads(vecThreads);
		for (int i = 0; i < static_cast<int>(vecThreads.size()); ++i) {
			Thread* t = vecThreads[i];
			ThreadBase* exeTarget = t->exeTarget;
			if (NULL != exeTarget) {
				exeTarget->OnShutdown();
			}
		}

		do {
			if (GetThreadCount() <= 0)
			{
				break;
			}
			if (GetIdleThreadCount() > 0)
			{
				/*if we are here then a thread in the free pool checked if it was being shut down just before CThreadPool::Shutdown() was called,
				but called Suspend() just after KillIdleThreads(). All we need to do is to resume it.*/
				CScopedLock lock(*m_pArrLock);
				int32_t idleThreadCount = m_nCurSize - m_nOffset - 1;
				atomic_cmpxchg(&m_thdsToExit, 0, idleThreadCount);
				for (int32_t i = m_nOffset + 1; i < m_nCurSize; ++i) {
					Thread* t = m_arrThreads[i];
					t->thdController.Resume();
				}
			}
#ifdef OPEN_THREAD_POOL_DEBUG
			OutputDebug("%u active and %u free %d thdsEaten threads remaining...",
				GetActiveThreadCount(), GetFreeThreadCount(), (int)m_thdsEaten);
			do {
				CScopedLock lock(*m_pArrLock);
				for (int32_t i = 0; i <= m_nOffset; ++i) {
					Thread* t = m_arrThreads[i];
					OutputDebug("Thread %u bad active thread. thdStatus = %d ", t->thdController.GetId(), t->thdStatus);
				}
			} while (false);// scoped
#endif
			Sleep(6);
		} while (true);
	}

// gets active thread count
int32_t CThreadPool::GetActiveThreadCount() {
	CScopedLock lock(*m_pArrLock);
	return m_nOffset + 1;
}

// gets free thread count
int32_t CThreadPool::GetIdleThreadCount() {
	CScopedLock lock(*m_pArrLock);
	return m_nCurSize - m_nOffset - 1;
}

int32_t CThreadPool::GetThreadCount() {
	CScopedLock lock(*m_pArrLock);
	return m_nCurSize;
}

bool CThreadPool::InsertActiveThread(Thread* t) {
	CScopedLock lock(*m_pArrLock);
	if (m_nCurSize >= THREAD_MAX_SIZE) {
		return false;
	}
	if (m_nOffset + 1 < m_nCurSize) {
		m_arrThreads[m_nCurSize] = m_arrThreads[m_nOffset + 1];
		m_arrThreads[m_nCurSize]->index = m_nCurSize;
		if (m_nCurSize > 0) {
			m_arrThreads[m_nCurSize]->idleTime = m_arrThreads[m_nCurSize - 1]->idleTime;
		}
	}
	t->index = m_nOffset + 1;
	if (t->thdStatus != THREAD_STATUS_TIMEOUT) {
		t->idleTime = MAX_IDLE_TIME;
	}
	m_arrThreads[t->index] = t;
	++m_nOffset;
	++m_nCurSize;
	return true;
}

void CThreadPool::GetActiveThreads(std::vector< Thread* >& vecThreads) {
	CScopedLock lock(*m_pArrLock);
	for (int32_t i = 0; i <= m_nOffset; ++i) {
		vecThreads.push_back(m_arrThreads[i]);
	}
}

void CThreadPool::EraseThread(Thread* t) {
	CScopedLock lock(*m_pArrLock);
	if (t->index > -1 && t->index <= m_nOffset) {
		// active area
		if (t->index != m_nOffset) {
			m_arrThreads[t->index] = m_arrThreads[m_nOffset];
			m_arrThreads[m_nOffset]->index = t->index;
			t->index = m_nOffset;
			m_arrThreads[m_nOffset] = t;
		}
		--m_nOffset;
	} else if (t->index <= m_nOffset || t->index >= m_nCurSize) {
		OutputError("CThreadPool::EraseThread  No found !  index = %d m_nOffset = %d m_nCurSize = %d ", t->index, m_nOffset, m_nCurSize);
		return;
	}
	// idle area
	if (t->index != m_nCurSize - 1) {
		m_arrThreads[t->index] = m_arrThreads[m_nCurSize - 1];
		m_arrThreads[t->index]->index = t->index;
		t->index = m_nCurSize - 1;
	}
	m_arrThreads[m_nCurSize - 1] = NULL;
	--m_nCurSize;
}

void CThreadPool::EraseIdleThread(Thread* t) {
	CScopedLock lock(*m_pArrLock);
	if (t->index <= m_nOffset || t->index >= m_nCurSize) {
		OutputError("CThreadPool::EraseIdleThread  No found !  index = %d m_nOffset = %d m_nCurSize = %d ", t->index, m_nOffset, m_nCurSize);
		return;
	}
	if (t->index != m_nCurSize - 1) {
		m_arrThreads[t->index] = m_arrThreads[m_nCurSize - 1];
		m_arrThreads[t->index]->index = t->index;
		t->index = m_nCurSize - 1;
		m_arrThreads[t->index] = t;
	}
	m_arrThreads[m_nCurSize - 1] = NULL;
	--m_nCurSize;
}
	
Thread* CThreadPool::StartThreadEx(ThreadBase* exeTarget) {

	if (m_bShutdown) {
		return NULL;
	}

	if(NULL != exeTarget) {
		ThreadBase** ppThdBase = m_pTbQueue->WriteLock();
		*ppThdBase = exeTarget;
		m_pTbQueue->WriteUnlock();
	}

	if (GetThreadCount() >= THREAD_MAX_SIZE) {
		return ActivateFromIdle();
	}

	return StartThread();
}

/* this is the only platform-specific code. neat, huh! */
#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ ) || defined ( _WIN64 )

unsigned long WINAPI CThreadPool::thread_proc(void* param)
{
	Thread* t = (Thread*)param;
	atomic_xchg8(&t->thdStatus, THREAD_STATUS_ACTIVE);
	// wait for init
	for(int i = 0; FALSE == t->initFlag; ++i) {
		cpu_relax(i);
	}

	if (!thd::ThreadPool.InsertActiveThread(t)) {
		ExitThread(0);
		return 0;
	}
	atomic_dec(&thd::ThreadPool.m_thdsEaten);

	ThreadBase** ppThdBase = NULL;
    uint32_t tid = t->thdController.GetId();

#ifdef OPEN_THREAD_POOL_DEBUG
	OutputDebug("Thread %u started.", t->thdController.GetId());
#endif
	for(;;)
	{
		ppThdBase = thd::ThreadPool.m_pTbQueue->ReadLock();
		if (NULL != ppThdBase) {
			t->exeTarget = *ppThdBase;
			thd::ThreadPool.m_pTbQueue->ReadUnlock();
		}
		
		if (t->exeTarget != NULL) {
			if (t->exeTarget->OnRun()) {
				delete t->exeTarget;
			}
			t->exeTarget = NULL;
		}

		if (thd::ThreadPool.m_pTbQueue->Size() > 0) {
			continue;
		}

		if(thd::ThreadPool.ThreadExit(t))
		{
#ifdef OPEN_THREAD_POOL_DEBUG
			OutputDebug("Thread Active %u exiting.", tid);
#endif
			break;
		}
		else
		{
#ifdef OPEN_THREAD_POOL_DEBUG
			
			OutputDebug("Thread %u waiting for a new task.", tid);
			
#endif
			if (thd::ThreadPool.m_pTbQueue->Size() < 1) {
				// enter "suspended" state. when we return, the threadpool will either tell us to fuk off, or to execute a new task.
				t->thdController.Suspend();
			}
			// after resuming, this is where we will end up. start the loop again, check for tasks, then go back to the threadpool.

			atomic_xchg8(&t->thdStatus, THREAD_STATUS_ACTIVE);
			thd::ThreadPool.EraseIdleThread(t);
			if (thd::ThreadPool.m_bShutdown) {
#ifdef OPEN_THREAD_POOL_DEBUG
				OutputDebug("Thread Shutdown %u exiting.", tid);
#endif
				break;
			} else {
				if (!thd::ThreadPool.InsertActiveThread(t)) {
					break;
				}
				atomic_dec(&thd::ThreadPool.m_thdsEaten);
			}
		}
	}

	// at this point the t pointer has already been freed, so we can just cleanly exit.
	ExitThread(0);
	return 0;
}

Thread* CThreadPool::StartThread() throw()
{
	if (m_bShutdown) {
		return NULL;
	}

	Thread* t = new util::WrapPoolIs<Thread>;
	t->thdStatus = THREAD_STATUS_ACTIVE;
	t->exeTarget = NULL;
	t->idleTime = MAX_IDLE_TIME;
	t->index = -1;
	atomic_xchg8(&t->initFlag, FALSE);

	HANDLE hThread = CreateThread(NULL, 0,
		&thread_proc, (LPVOID)t, 0, NULL);

	if(NULL != hThread) {
		t->thdController.Setup(hThread);
		atomic_xchg8(&t->initFlag, TRUE);
		return t;
	} else {
		delete t;
		return NULL;
	}
}

#else

void * CThreadPool::thread_proc(void * param)
{
	Thread* t = (Thread*)param;
	atomic_xchg8(&t->thdStatus, THREAD_STATUS_ACTIVE);
	// wait for init
	for(int i = 0; FALSE == t->initFlag; ++i) {
		cpu_relax(i);
	}

	if (!thd::ThreadPool.InsertActiveThread(t)) {
		pthread_exit(0);
		return NULL;
	}
	atomic_dec(&thd::ThreadPool.m_thdsEaten);

	ThreadBase** ppThdBase = NULL;
	uint32_t tid = t->thdController.GetId();

#ifdef OPEN_THREAD_POOL_DEBUG
	OutputDebug("ThreadPool Thread %u started.", tid);
#endif

	for(;;)
	{
		ppThdBase = thd::ThreadPool.m_pTbQueue->ReadLock();
		if (NULL != ppThdBase) {
			t->exeTarget = *ppThdBase;
			thd::ThreadPool.m_pTbQueue->ReadUnlock();
		}
		
		if (t->exeTarget != NULL) {
			if (t->exeTarget->OnRun()) {
				delete t->exeTarget;
			}
			t->exeTarget = NULL;
		}

		if (thd::ThreadPool.m_pTbQueue->Size() > 0) {
			continue;
		}

		if(thd::ThreadPool.ThreadExit(t)) {
#ifdef OPEN_THREAD_POOL_DEBUG
			OutputDebug("Thread Active %u exiting.", tid);
#endif
			break;
		} else {
			if (thd::ThreadPool.m_pTbQueue->Size() < 1) {
				// enter "suspended" state. when we return, the threadpool will either tell us to fuk off, or to execute a new task.
				t->thdController.Suspend();
			}
			// after resuming, this is where we will end up. start the loop again, check for tasks, then go back to the threadpool.

			atomic_xchg8(&t->thdStatus, THREAD_STATUS_ACTIVE);
			thd::ThreadPool.EraseIdleThread(t);
			if (thd::ThreadPool.m_bShutdown) {
#ifdef OPEN_THREAD_POOL_DEBUG
				OutputDebug("Thread Shutdown %u exiting.", tid);
#endif
				break;
			} else {
				if (!thd::ThreadPool.InsertActiveThread(t)) {
					break;
				}
				atomic_dec(&thd::ThreadPool.m_thdsEaten);
			} 
		}
	}
	pthread_exit(0);
	return NULL;
}

Thread* CThreadPool::StartThread() throw()
{
	if(m_bShutdown) {
		return NULL;
	}

	Thread* t = new util::WrapPoolIs<Thread>;
	t->thdStatus = THREAD_STATUS_ACTIVE;
	t->exeTarget = NULL;
	t->idleTime = MAX_IDLE_TIME;
	t->index = -1;
	atomic_xchg8(&t->initFlag, FALSE);

	pthread_t target;
    if(pthread_create(&target, NULL, &thread_proc, (void*)t) == 0) {
		t->thdController.Setup(target);
		atomic_xchg8(&t->initFlag, TRUE);
		return t;
	} else {
		delete t;
		return NULL;
	}
}

#endif

} // end namespace thd
