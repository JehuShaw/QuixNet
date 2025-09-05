/*
 * File:   EnvLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef ENVLOCK_H
#define ENVLOCK_H

#include "IEnvLock.h"

#ifndef IGNORE_THREAD_SAFE_CHECK
#include "XqxTable1StoreS.h"
#endif

namespace thd {

/** An abstract base class for synchronization primitives.
 */
template<class WrapedType, class LockType, class UseType = WrapedType>
class CEnvLock : public IEnvLock, private IEnvLockData<UseType>
{
public:
	CEnvLock() {
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(int32_t) < sizeof(long));
#endif
	}

	CEnvLock(const CEnvLock& orig)
		: m_data(orig.m_data) {
	}

	~CEnvLock() {
		if(m_lock.Using()) {
			throw env_lock_used_error("The instance is being used !");
		}
	}

	CEnvLock& operator = (const CEnvLock& right) {
		m_data = right.m_data;
		return *this;
	}

private:
	/** Lock operation. */
	virtual void Lock() throw() {
		m_lock.Lock();
	}

	virtual bool TimedLock(uint32_t msec) throw() {
		return m_lock.TimedLock(msec);
	}

	/** Unlock operation. */
	virtual void Unlock() throw() {
		m_lock.Unlock();
	}

	virtual bool Using() throw() {
		return m_lock.Using();
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
	LockType m_lock;
#ifndef IGNORE_THREAD_SAFE_CHECK
	util::CXqxTable1StoreS<bool> m_envFlags;
#endif
};

}; // namespace thd

#endif // ENVLOCK_H

/* end of header file */
