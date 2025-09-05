/*
 * File:   EnvRWLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef ENVRWLOCK_H
#define ENVRWLOCK_H

#include "IEnvRWLock.h"

#ifndef IGNORE_THREAD_SAFE_CHECK
#include "XqxTable1StoreS.h"
#endif

namespace thd {

/** An abstract base class for synchronization primitives.
 */
template<class WrapedType, class LockType, class UseType = WrapedType>
class CEnvRWLock : public IEnvRWLock, private IEnvLockData<UseType>
{
public:
	CEnvRWLock() {
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(int32_t) < sizeof(long));
#endif
	}

	CEnvRWLock(const CEnvRWLock& orig)
		: m_data(orig.m_data) {
	}

	~CEnvRWLock() {
		// "Is the instance is being used ?"
		assert(!m_rwLock.Using());
	}

	CEnvRWLock& operator = (const CEnvRWLock& right) {
		m_data = right.m_data;
		return *this;
	}

private:
	/** Acquire a write lock, blocking until the unlockWrite is acquired. */
	virtual void LockWrite() throw() {
		m_rwLock.LockWrite();
	}

	virtual bool TimedLockWrite(uint32_t msec) throw() {
		return m_rwLock.TimedLockWrite(msec);
	}

	/** Acquire a read lock, blocking until the unlockRead is acquired. */
	virtual void LockRead() throw() {
		m_rwLock.LockRead();
	}

	virtual bool TimedLockRead(uint32_t msec) throw() {
		return m_rwLock.TimedLockRead(msec);
	}

	/** Release read or write lock. */
	virtual void Unlock() throw() {
		m_rwLock.Unlock();
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

#ifndef IGNORE_THREAD_SAFE_CHECK
	virtual util::CXqxTable1StoreS<bool>& GetEnvInfo() {
		return m_envFlags;
	}
#endif

private:
	WrapedType m_data;
	LockType m_rwLock;
#ifndef IGNORE_THREAD_SAFE_CHECK
	util::CXqxTable1StoreS<bool> m_envFlags;
#endif
};

}; // namespace thd

#endif // ENVRWLOCK_H

/* end of header file */
