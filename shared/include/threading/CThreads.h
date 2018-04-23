/*
 * File:   CThreads.h
 * Author: Jehu Shaw
 *
 */

// Class CThread - Base class for all threads in the
// server, and allows for easy management .

#ifndef _CTHREADS_H
#define _CTHREADS_H

#include "ThreadBase.h"
#include "AtomicLock.h"

namespace thd {

enum CThreadState
{
	THREADSTATE_TERMINATE,
	THREADSTATE_FREE,
	THREADSTATE_BUSY,
	THREADSTATE_AWAITING,
};

class SHARED_DLL_DECL CThread : public ThreadBase
{
public:
	CThread() {
		SetThreadState(THREADSTATE_AWAITING);
	}

	~CThread() {
	}

	INLINE void SetThreadState(CThreadState threadState) {
		atomic_xchg8(&m_threadState, (char)threadState);
	}

	INLINE CThreadState GetThreadState() const {
		return (CThreadState)m_threadState;
	}

	virtual bool Run() {	
		// to do
		return false;
	}

	virtual bool OnRun() {
		SetThreadState(THREADSTATE_BUSY);
		bool bRet = Run();
		SetThreadState(THREADSTATE_FREE);
		return bRet;
	}

	virtual void OnShutdown() {
		SetThreadState(THREADSTATE_TERMINATE);
	}

	void WaitForDone() {
		for(int i = 0; (char)THREADSTATE_BUSY == m_threadState; ++i) {
			cpu_relax(i);
		}
	}

protected:

	CThread& operator=( CThread &other ) {
		atomic_xchg8(&m_threadState, other.m_threadState);
		return *this;
	}

	volatile char m_threadState;
};

}

#endif /* _CTHREADS_H */
