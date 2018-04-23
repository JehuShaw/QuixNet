/*
 * File:   WrapLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef __WRAPLOCK_H_
#define __WRAPLOCK_H_

#include "IWrapLock.h"

namespace thd {

/** An abstract base class for synchronization primitives.
 */
template<class LockType, class WrapedType, class UseType = WrapedType>
class CWrapLock : public IWrapLock, private IWrapData<UseType>
{
public:
	CWrapLock() {
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(int32_t) < sizeof(long));
#endif
	}

	~CWrapLock() {
		if(m_lock.Using()) {
			throw wrap_used_error("The instance is being used !");
		}
	}

private:
	/** %Lock operation. */
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

private:
	WrapedType m_data;
	LockType m_lock;
};

}; // namespace thd

#endif // __WRAPLOCK_H_

/* end of header file */
