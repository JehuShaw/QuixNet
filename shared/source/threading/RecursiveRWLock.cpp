#include "RecursiveRWLock.h"
#include "ThreadIndexManager.h"

namespace thd {

/*
 */

CRecursiveRWLock::CRecursiveRWLock() throw()
	: m_readersReading(0)
	, m_writerWriting(0)
	, m_threadId(0)
	, m_recursionCount(0)
{
}

/*
 */

CRecursiveRWLock::~CRecursiveRWLock() throw()
{
	m_mutex.Lock();
	m_writerWriting = 0;
	m_readersReading = 0;
	m_lockFree.NotifyAll();
	m_recursionCount = 0;
	m_threadId = 0;
	m_mutex.Unlock();
}

/*
 */

void CRecursiveRWLock::LockWrite() throw()
{
	m_mutex.Lock();

	const uint32_t threadId = GetSysCurrentThreadId();
	if(m_writerWriting > 0) {
		if(threadId == m_threadId) {
			++m_recursionCount;
			m_mutex.Unlock();
			return;
		}
	}
	
	while(m_writerWriting || m_readersReading) {
		m_lockFree.Wait(m_mutex);
	}

	m_writerWriting++;

	m_threadId = threadId;
	m_recursionCount = 1;

	m_mutex.Unlock();
}

/*
 */

bool CRecursiveRWLock::TimedLockWrite(uint32_t msec) throw()
{
	if(!m_mutex.TimedLock(msec)) {
		return false;
	}

	const uint32_t threadId = GetSysCurrentThreadId();
	if(m_writerWriting > 0) {
		if(threadId == m_threadId) {
			++m_recursionCount;
			m_mutex.Unlock();
			return true;
		}
	}
	
	uint64_t expires = GetSysTickCount() + msec;

	while(m_writerWriting || m_readersReading) {
		int64_t left = 0;
		uint64_t now = GetSysTickCount();
		if(now < expires) {
			left = (expires - now);
		}

		if(!m_lockFree.Wait(m_mutex, (uint32_t)left)) {
			m_mutex.Unlock();
			return false;
		}
	}

	++m_writerWriting;

	m_threadId = threadId;
	m_recursionCount = 1;

	m_mutex.Unlock();

	return true;
}

/*
 */

void CRecursiveRWLock::LockRead() throw()
{
	m_mutex.Lock();

	if(m_writerWriting > 0) {
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
			m_mutex.Unlock();
			return;
		}
	}

	if(IncThreadReader()) {
		m_mutex.Unlock();
		return;
	}
	
	while(m_writerWriting) {
		m_lockFree.Wait(m_mutex);
	}
	
	++m_readersReading;
	m_mutex.Unlock();
}

/*
 */

bool CRecursiveRWLock::TimedLockRead(uint32_t msec) throw()
{
	if(!m_mutex.TimedLock(msec)) {
		return false;
	}

	if(m_writerWriting > 0) {
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
			m_mutex.Unlock();
			return true;
		}
	}

	if(IncThreadReader()) {
		m_mutex.Unlock();
		return true;
	}

	uint64_t expires = GetSysTickCount() + msec;

	while(m_writerWriting) {
		int64_t left = 0;
		uint64_t now = GetSysTickCount();
		if(now < expires) {
			left = (expires - now);
		}

		if(!m_lockFree.Wait(m_mutex, (uint32_t)left)) {
			m_mutex.Unlock();
			return false;
		}
	}

	m_readersReading++;
	m_mutex.Unlock();

	return true;
}

/*
 */

void CRecursiveRWLock::Unlock() throw()
{
	m_mutex.Lock();
	if (m_writerWriting > 0) {
		// unlock write lock
		const uint32_t threadId = GetSysCurrentThreadId();
		if (threadId == m_threadId) {
			if (m_recursionCount > 1) {
				--m_recursionCount;
				m_mutex.Unlock();
				return;
			}
			else {
				m_recursionCount = 0;
				m_threadId = 0;
			}
		}

		m_writerWriting = 0;
		m_lockFree.NotifyAll();
	} else {

		if (DecThreadReader()) {
			m_mutex.Unlock();
			return;
		}

		// unlock read lock
		if (m_readersReading > 0)
		{
			if (--m_readersReading == 0) {
				m_lockFree.Notify();
			}
		}
	}
	m_mutex.Unlock();
}

void CRecursiveRWLock::UpgradeLockRead() throw()
{
	m_mutex.Lock();

	const uint32_t threadId = GetSysCurrentThreadId();
	if(m_writerWriting > 0) {
		if(threadId == m_threadId) {
			m_mutex.Unlock();
			return;
		}
	}

	if(ClearThreadReader()) {
		// unlock read lock
		if(m_readersReading > 0)
		{
			--m_readersReading;
		}
	}

	while(m_writerWriting) {
		m_lockFree.Wait(m_mutex);
	}

	m_writerWriting = 1;

	// wait all read unlock
	while(m_readersReading) {
		m_lockFree.Wait(m_mutex);
	}

	m_threadId = threadId;
	m_recursionCount = 1;

	m_mutex.Unlock();
}

bool CRecursiveRWLock::TimedUpgradeLockRead(uint32_t msec) throw()
{
	if(!m_mutex.TimedLock(msec)) {
		return false;
	}

	const uint32_t threadId = GetSysCurrentThreadId();
	if(m_writerWriting > 0) {
		if(threadId == m_threadId) {
			m_mutex.Unlock();
			return true;
		}
	}

	if(ClearThreadReader()) {
		// unlock read lock
		if(m_readersReading > 0)
		{
			--m_readersReading;
		}
	}

	uint64_t expires = GetSysTickCount() + msec;

	while(m_writerWriting) {
		int64_t left = 0;
		uint64_t now = GetSysTickCount();
		if(now < expires) {
			left = (expires - now);
		}

		if(!m_lockFree.Wait(m_mutex, (uint32_t)left)) {
			m_mutex.Unlock();
			return false;
		}
	}

	m_writerWriting = 1;
	// wait all read unlock
	while(m_readersReading) {
		int64_t left = 0;
		uint64_t now = GetSysTickCount();
		if(now < expires) {
			left = (expires - now);
		}

		if(!m_lockFree.Wait(m_mutex, (uint32_t)left)) {
			m_mutex.Unlock();
			return false;
		}
	}

	m_mutex.Unlock();
	return true;
}

bool CRecursiveRWLock::DegradeLockWrite() throw()
{
	m_mutex.Lock();
	// unlock write lock
	if(m_writerWriting > 0)
	{
		assert(m_readersReading == 0);
		if(!IncThreadReader()) {
			++m_readersReading;
		}
		m_writerWriting = 0;
		m_lockFree.NotifyAll();

		m_mutex.Unlock();
		return true;
	}
	m_mutex.Unlock();
	return false;
}

bool CRecursiveRWLock::IncThreadReader()
{
	bool bSkipLock = false;
	uint64_t thdIndex = CThreadIndexManager::Pointer()->Get();
	if(m_thdContext.Has(thdIndex)) {
		int32_t readerCount = m_thdContext.Find(thdIndex) + 1;
		m_thdContext.Change(thdIndex, readerCount);
		if(readerCount > 1) {
			bSkipLock = true;
		}
	} else {
		m_thdContext.Add(thdIndex, 1);
	}
	return bSkipLock;
}

bool CRecursiveRWLock::DecThreadReader()
{
	bool bSkipUnlock = false;
	uint64_t thdIndex = CThreadIndexManager::Pointer()->Get();
	if(m_thdContext.Has(thdIndex)) {
		int32_t readerCount = m_thdContext.Find(thdIndex);
		if(readerCount > 0) {
			--readerCount;
			m_thdContext.Change(thdIndex, readerCount);
			if(readerCount > 0) {
				bSkipUnlock = true;
			}
		}
	}
	return bSkipUnlock;
}

bool CRecursiveRWLock::ClearThreadReader()
{
	bool bClear = false;
	uint64_t thdIndex = CThreadIndexManager::Pointer()->Get();
	if(m_thdContext.Has(thdIndex)) {
		int32_t readerCount = m_thdContext.Find(thdIndex);
		if(readerCount > 0) {
			m_thdContext.Change(thdIndex, 0);
			bClear = true;
		}
	}
	return bClear;
}

bool CRecursiveRWLock::HasThreadReader()
{
	bool hasLock = false;
	uint64_t thdIndex = CThreadIndexManager::Pointer()->Get();
	if(m_thdContext.Has(thdIndex)) {
		int32_t readerCount = m_thdContext.Find(thdIndex);
		if(readerCount > 0) {
			hasLock = true;
		}
	}
	return hasLock;
}

bool CRecursiveRWLock::Using() throw()
{
	bool bUsing = false;
	m_mutex.Lock();
	bUsing = (0 != m_writerWriting || 0 != m_readersReading);
	m_mutex.Unlock();
	return bUsing;
}

} // namespace thd

/* end of source file */
