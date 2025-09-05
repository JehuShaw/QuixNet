#include "ScopedRWLock.h"

namespace thd {

CScopedReadLock::CScopedReadLock(CScopedWriteLock& scopedWrite) : m_rLock(scopedWrite.m_wLock)
{
	if (NULL == m_rLock) {
		throw std::runtime_error("Must lock write !");
	}
	if (!m_rLock->DegradeLockWrite()) {
		throw std::runtime_error("Degrade lock fail !");
	}
	scopedWrite.m_wLock = NULL;
}

bool CScopedReadLock::Degrade(CScopedWriteLock& scopedWrite)
{
	if (NULL == scopedWrite.m_wLock) {
		return false;
	}

	if (!scopedWrite.m_wLock->DegradeLockWrite()) {
		return false;
	}

	if (NULL != m_rLock && m_rLock != scopedWrite.m_wLock) {
		m_rLock->Unlock();
	}
	m_rLock = scopedWrite.m_wLock;
	scopedWrite.m_wLock = NULL;
	return true;
}
//////////////////////////////////////////////////////////////////////////
CScopedWriteLock::CScopedWriteLock(CScopedReadLock& scopedRead) : m_wLock(scopedRead.m_rLock)
{
	if (NULL == m_wLock) {
		throw std::runtime_error("Must lock read !");
	}
	m_wLock->UpgradeLockRead();
	scopedRead.m_rLock = NULL;
}

void CScopedWriteLock::Upgrade(CScopedReadLock& scopedRead)
{
	if (NULL == scopedRead.m_rLock) {
		return;
	}
	if (NULL != m_wLock && m_wLock != scopedRead.m_rLock) {
		m_wLock->Unlock();
	}
	m_wLock = scopedRead.m_rLock;
	scopedRead.m_rLock = NULL;
	m_wLock->UpgradeLockRead();
}

}; // namespace thd

