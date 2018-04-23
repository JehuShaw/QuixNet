/*
 * File:   WrapRWLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef __WRAPRWLOCK_H_
#define __WRAPRWLOCK_H_

#include "IWrapRWLock.h"

namespace thd {

/** An abstract base class for synchronization primitives.
 */
template<class LockType, class WrapedType, class UseType = WrapedType>
class CWrapRWLock : public IWrapRWLock, private IWrapData<UseType>
{
public:
	CWrapRWLock() {
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(int32_t) < sizeof(long));
#endif
	}

	~CWrapRWLock() {
		if(m_rwLock.Using()) {
			throw wrap_used_error("The instance is being used !");
		}
	}

private:
	/** Acquire a write lock, blocking until the unlockWrite is acquired. */
	virtual void LockWrite() throw() {
		m_rwLock.LockWrite();
	}

	virtual bool TimedLockWrite(uint32_t msec) throw() {
		return m_rwLock.TimedLockWrite(msec);
	}

	/** Release the write lock. */
	virtual void UnlockWrite() throw() {
		m_rwLock.UnlockWrite();
	}

	/** Acquire a read lock, blocking until the unlockRead is acquired. */
	virtual void LockRead() throw() {
		m_rwLock.LockRead();
	}

	virtual bool TimedLockRead(uint32_t msec) throw() {
		return m_rwLock.TimedLockRead(msec);
	}

	/** Release the read lock. */
	virtual void UnlockRead() throw() {
		m_rwLock.UnlockRead();
	}

	/** Upgrade the read lock. */
	virtual void UpgradeLockRead() throw() {
		m_rwLock.UpgradeLockRead();
	}

	virtual bool TimedUpgradeLockRead(uint32_t msec) throw() {
		return m_rwLock.TimedUpgradeLockRead(msec);
	}

	/** Degrade the write lock. */
	virtual bool DegradeLockWrite() throw() {
		return m_rwLock.DegradeLockWrite();
	}

	virtual bool Using() throw() {
		return m_rwLock.Using();
	}

private:
	virtual UseType* GetDataPtr() {
		return &m_data;
	}

private:
	WrapedType m_data;
	LockType m_rwLock;
};

}; // namespace thd

#endif // __WRAPRWLOCK_H_

/* end of header file */
