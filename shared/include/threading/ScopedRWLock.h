/*
 * File:   ScopedRWLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef __SCOPEDRWLOCK_H_
#define __SCOPEDRWLOCK_H_

#include "IRWLock.h"

namespace thd {

// scoped read lock help
class CScopedReadLock {

public:
	explicit CScopedReadLock(IRWLock& guard) throw() : m_rLock(guard) {
		m_rLock.LockRead();
	}
	explicit CScopedReadLock(const IRWLock& guard) throw() : m_rLock(const_cast<IRWLock&>(guard)) {
		m_rLock.LockRead();
	}
	~CScopedReadLock() {
		m_rLock.UnlockRead();
	}

private:
	IRWLock& m_rLock;

	CScopedReadLock(const CScopedReadLock& orig);
	CScopedReadLock& operator=(const CScopedReadLock &orig);
};

// scoped write lock help
class CScopedWriteLock {

public:
	explicit CScopedWriteLock(IRWLock& guard) throw() : m_wLock(guard) {
		m_wLock.LockWrite();
	}
	explicit CScopedWriteLock(const IRWLock& guard) throw() : m_wLock(const_cast<IRWLock&>(guard)) {
		m_wLock.LockWrite();
	}
	~CScopedWriteLock() {
		m_wLock.UnlockWrite();
	}

private:
	IRWLock& m_wLock;

	CScopedWriteLock(const CScopedWriteLock& orig);
	CScopedWriteLock& operator=(const CScopedWriteLock &orig);
};

}; // namespace thd


#endif // __SCOPEDRWLOCK_H_

/* end of header file */
