#include "ReadWriteLock.h"

namespace thd {

/*
 */

CReadWriteLock::CReadWriteLock() throw()
	: m_readersReading(0)
	, m_writerWriting(0)
{
}

/*
 */

CReadWriteLock::~CReadWriteLock() throw()
{
}

/*
 */

void CReadWriteLock::LockWrite() throw()
{
	m_mutex.Lock();
	
	while(m_writerWriting || m_readersReading) {
		m_lockFree.Wait(m_mutex);
	}

	m_writerWriting++;
	m_mutex.Unlock();
}

/*
 */

bool CReadWriteLock::TimedLockWrite(uint32_t msec) throw()
{
	if(!m_mutex.TimedLock(msec)) {
		return false;
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
	m_mutex.Unlock();

	return true;
}

/*
 */

void CReadWriteLock::LockRead() throw()
{
	m_mutex.Lock();
	
	while(m_writerWriting) {
		m_lockFree.Wait(m_mutex);
	}
	
	++m_readersReading;
	m_mutex.Unlock();
}

/*
 */

bool CReadWriteLock::TimedLockRead(uint32_t msec) throw()
{
	if(!m_mutex.TimedLock(msec)) {
		return false;
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

void CReadWriteLock::Unlock() throw()
{
	m_mutex.Lock();
	if (m_writerWriting > 0) {
		// unlock write lock
		m_writerWriting = 0;
		m_lockFree.NotifyAll();
	} else if(m_readersReading > 0) {
		// unlock read lock
		if(--m_readersReading == 0) {
			m_lockFree.Notify();
		}
	}
	m_mutex.Unlock();
}

void CReadWriteLock::UpgradeLockRead() throw()
{
	m_mutex.Lock();

	// unlock read lock
	if(m_readersReading > 0)
	{
		--m_readersReading;
	}

	while(m_writerWriting) {
		m_lockFree.Wait(m_mutex);
	}

	m_writerWriting = 1;

	// wait all read unlock
	while(m_readersReading) {
		m_lockFree.Wait(m_mutex);
	}

	m_mutex.Unlock();
}

bool CReadWriteLock::TimedUpgradeLockRead(uint32_t msec) throw()
{
	m_mutex.Lock();

	// unlock read lock
	if(m_readersReading > 0)
	{
		--m_readersReading;
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

bool CReadWriteLock::DegradeLockWrite() throw()
{
	m_mutex.Lock();
	// unlock write lock
	if(m_writerWriting > 0)
	{
		++m_readersReading;
		m_writerWriting = 0;
		m_lockFree.NotifyAll();

		m_mutex.Unlock();
		return true;
	}
	m_mutex.Unlock();
	return false;
}

bool CReadWriteLock::Using() throw()
{
	bool bUsing = false;
	m_mutex.Lock();
	bUsing = (0 != m_writerWriting || 0 != m_readersReading);
	m_mutex.Unlock();
	return bUsing;
}

} // namespace thd

/* end of source file */
