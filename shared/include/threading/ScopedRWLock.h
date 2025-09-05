/*
 * File:   ScopedRWLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef SCOPEDRWLOCK_H
#define SCOPEDRWLOCK_H

#include "IRWLock.h"

namespace thd {

class CScopedWriteLock;
// scoped read lock help
class CScopedReadLock {

friend class CScopedWriteLock;
public:
	explicit CScopedReadLock(IRWLock& guard) throw() : m_rLock(&guard) {
		m_rLock->LockRead();
	}
	explicit CScopedReadLock(const IRWLock& guard) throw() : m_rLock(const_cast<IRWLock*>(&guard)) {
		m_rLock->LockRead();
	}
	SHARED_DLL_DECL explicit CScopedReadLock(CScopedWriteLock& scopedWrite);

	~CScopedReadLock() throw() {
		if (m_rLock == NULL) {
			return;
		}
		m_rLock->Unlock();
	}

	inline void Lock(IRWLock& guard)
	{
		if (m_rLock == &guard) {
			return;
		}
		if (NULL != m_rLock) {
			m_rLock->Unlock();
		}
		m_rLock = &guard;
		m_rLock->LockRead();
	}

	void Unlock() throw() {
		if (m_rLock == NULL) {
			return;
		}
		m_rLock->Unlock();
		m_rLock = NULL;
	}

	/** Degrade the write lock. */
	SHARED_DLL_DECL bool Degrade(CScopedWriteLock& scopedWrite);

private:
	IRWLock* m_rLock;

	CScopedReadLock(const CScopedReadLock& orig) {}
	CScopedReadLock& operator=(const CScopedReadLock &orig) { return *this; }
};

// scoped write lock help
class CScopedWriteLock {

friend class CScopedReadLock;
public:
	explicit CScopedWriteLock(IRWLock& guard) throw() : m_wLock(&guard) {
		m_wLock->LockWrite();
	}
	explicit CScopedWriteLock(const IRWLock& guard) throw() : m_wLock(const_cast<IRWLock*>(&guard)) {
		m_wLock->LockWrite();
	}

	SHARED_DLL_DECL explicit CScopedWriteLock(CScopedReadLock& scopedRead);

	~CScopedWriteLock() throw() {
		if (m_wLock == NULL) {
			return;
		}
		m_wLock->Unlock();
	}

	inline void Lock(IRWLock& guard)
	{
		if (m_wLock == &guard) {
			return;
		}
		if (NULL != m_wLock) {
			m_wLock->Unlock();
		}
		m_wLock = &guard;
		m_wLock->LockWrite();
	}

	void Unlock() throw() {
		if (m_wLock == NULL) {
			return;
		}
		m_wLock->Unlock();
		m_wLock = NULL;
	}

	/** Upgrade the read lock. */
	SHARED_DLL_DECL void Upgrade(CScopedReadLock& scopedRead);

private:
	IRWLock* m_wLock;

	CScopedWriteLock(const CScopedWriteLock& orig) {}
	CScopedWriteLock& operator=(const CScopedWriteLock &orig) { return *this; }
};

}; // namespace thd


#endif // SCOPEDRWLOCK_H

/* end of header file */
