/*
 * File:   FastMutex.h
 * Author: Jehu Shaw
 *
 */

#ifndef FAST_MUTEX_H
#define FAST_MUTEX_H

#include "Common.h"
#include "ILock.h"
#include "AtomicLock.h"
#include "SysCurrentThreadId.h"

namespace thd {

class CFastMutex : public ILock
{
public:
	CFastMutex()
		: m_lock(0)
		, m_recursionCount(0)
	{
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(uint32_t) < sizeof(unsigned long));
#endif
	}

	~CFastMutex() {}

	void Lock() throw()
	{
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == (uint32_t)m_lock) {
			++m_recursionCount;
			return;
		}

		for(int i = 0; atomic_cmpxchg(&m_lock, 0, threadId); ++i) {
			cpu_relax(i);
		}

		m_recursionCount = 1;
	}

	bool TryLock() throw()
	{
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == (uint32_t)m_lock) {
			return true;
		}

		if(atomic_cmpxchg(&m_lock, 0, threadId) == 0) {
			m_recursionCount = 1;
			return true;
		}

		return false;
	}

	void Unlock() throw()
	{
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == (uint32_t)m_lock) {
			if(m_recursionCount > 1) {
				--m_recursionCount;
				return;
			} else {
				m_recursionCount = 0;
			}
		}
		atomic_xchg(&m_lock, 0);
	}

	bool Using() throw()
	{
		return 0 != m_lock;
	}

private:
	CFastMutex(const CFastMutex& orig) {}
	CFastMutex& operator=(const CFastMutex& orig) { return *this; }

private:
	volatile uint32_t m_lock;
	volatile int32_t m_recursionCount;
};

}

#endif /* FAST_MUTEX_H */

