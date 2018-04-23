/*
 * File:   ScopedWrapRWLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef __SCOPEDWRAPRWLOCK_H_
#define __SCOPEDWRAPRWLOCK_H_

#include "WrapRWLock.h"
#include "ScopedPointer.h"
#include "SysCurrentThreadId.h"

namespace thd {

// scoped read write lock help
template<class UseType>
class CScopedWrapRWLock
{
public:
	CScopedWrapRWLock(IWrapBase& lock)
		: m_rwLock(dynamic_cast<IWrapRWLock&>(lock))
		, m_data(dynamic_cast<IWrapData<UseType>&>(lock).GetDataPtr(), false)
		, m_state(WRAP_RWLOCK_UNINIT)
		, m_msec(0)
	{
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(uint32_t) < sizeof(unsigned long));
#endif
		const uint32_t threadId = GetSysCurrentThreadId();
		atomic_xchg(&m_threadId, threadId);
	}

	CScopedWrapRWLock(IWrapBase& lock, uint32_t msec)
		: m_rwLock(dynamic_cast<IWrapRWLock&>(lock))
		, m_data(dynamic_cast<IWrapData<UseType>&>(lock).GetDataPtr(), false)
		, m_state(WRAP_RWLOCK_UNINIT_TIMEOUT)
		, m_msec(msec)
	{
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(uint32_t) < sizeof(unsigned long));
#endif
		const uint32_t threadId = GetSysCurrentThreadId();
		atomic_xchg(&m_threadId, threadId);
	}

	~CScopedWrapRWLock() {
		m_data.SetRawPointer(NULL, false);
		if(WRAP_RWLOCK_READ == m_state) {
			m_rwLock.UnlockRead();
		} else if(WRAP_RWLOCK_WRITE == m_state) {
			m_rwLock.UnlockWrite();
		}
	}

	util::CScopedPointer<UseType> operator->() {
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
			if(WRAP_RWLOCK_UNINIT == m_state) {
				m_rwLock.LockWrite();
				m_state = WRAP_RWLOCK_WRITE;
			} else if(WRAP_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(m_rwLock.TimedLockWrite(m_msec)) {
					m_state = WRAP_RWLOCK_WRITE;
				} else {
					throw wrap_timeout_error("LockWrite timeout!");
				}
			}
			if(WRAP_RWLOCK_WRITE == m_state) {
				return m_data;
			} else if(WRAP_RWLOCK_READ == m_state) {
				throw wrap_readonly_error("Read only ! "
					"Use the method \"operator->() const\" please.");
			} else {
				assert(false);
				return util::CScopedPointer<UseType>();
			}
		} else {
			assert(false);
			return util::CScopedPointer<UseType>();
		}
	}

	util::CScopedPointer<const UseType> operator->() const {
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
			CScopedWrapRWLock* pThis = const_cast<
				CScopedWrapRWLock*>(this);
			if(WRAP_RWLOCK_UNINIT == m_state) {
				pThis->m_rwLock.LockRead();
				pThis->m_state = WRAP_RWLOCK_READ;
			} else if(WRAP_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(pThis->m_rwLock.TimedLockRead(m_msec)) {
					pThis->m_state = WRAP_RWLOCK_READ;
				} else {
					throw wrap_timeout_error("LockRead timeout!");
				}
			}

			if(WRAP_RWLOCK_READ == m_state
				|| WRAP_RWLOCK_WRITE == m_state)
			{
				return m_data;
			} else {
				assert(false);
				return util::CScopedPointer<const UseType>();
			}
		} else {
			assert(false);
			return util::CScopedPointer<const UseType>();
		}
	}

	util::CScopedPointer<UseType> GetData() {
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
			if(WRAP_RWLOCK_UNINIT == m_state) {
				m_rwLock.LockWrite();
				m_state = WRAP_RWLOCK_WRITE;
			} else if(WRAP_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(m_rwLock.TimedLockWrite(m_msec)) {
					m_state = WRAP_RWLOCK_WRITE;
				} else {
					throw wrap_timeout_error("LockWrite timeout!");
				}
			}
			if(WRAP_RWLOCK_WRITE == m_state) {
				return m_data;
			} else if(WRAP_RWLOCK_READ == m_state) {
				throw wrap_readonly_error("Read only ! "
					"Use the method \"GetData() const\" please.");
			} else {
				assert(false);
				return util::CScopedPointer<UseType>();
			}
		} else {
			assert(false);
			return util::CScopedPointer<UseType>();
		}
	}

	util::CScopedPointer<const UseType> GetData() const {
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
			CScopedWrapRWLock* pThis = const_cast<
				CScopedWrapRWLock*>(this);
			if(WRAP_RWLOCK_UNINIT == m_state) {
				pThis->m_rwLock.LockRead();
				pThis->m_state = WRAP_RWLOCK_READ;
			} else if(WRAP_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(pThis->m_rwLock.TimedLockRead(m_msec)) {
					pThis->m_state = WRAP_RWLOCK_READ;
				} else {
					throw wrap_timeout_error("LockRead timeout!");
				}
			}

			if(WRAP_RWLOCK_READ == m_state
				|| WRAP_RWLOCK_WRITE == m_state)
			{
				return m_data;
			} else {
				assert(false);
				return util::CScopedPointer<const UseType>();
			}
		} else {
			assert(false);
			return util::CScopedPointer<const UseType>();
		}
	}

	const CScopedWrapRWLock& DegradeLockWrite() {
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
			if(WRAP_RWLOCK_UNINIT == m_state) {
				m_rwLock.LockRead();
				m_state = WRAP_RWLOCK_READ;
			} else if(WRAP_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(m_rwLock.TimedLockRead(m_msec)) {
					m_state = WRAP_RWLOCK_READ;
				} else {
					throw wrap_timeout_error("LockRead timeout!");
				}
			} else {
				assert(WRAP_RWLOCK_READ != m_state);
				if(m_rwLock.DegradeLockWrite()) {
					m_state = WRAP_RWLOCK_READ;
					// Let the previous pointer fail.
					const UseType& data = *m_data;
					m_data.SetRawPointer(&data, false);
				} else {
					assert(false);
				}
			}
		} else {
			assert(false);
		}
		return *this;
	}

	CScopedWrapRWLock& UpgradeLockRead() const {
		CScopedWrapRWLock* pThis = const_cast<
			CScopedWrapRWLock*>(this);
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
			if(WRAP_RWLOCK_UNINIT == m_state) {
				pThis->m_rwLock.LockWrite();
				pThis->m_state = WRAP_RWLOCK_WRITE;
			} else if(WRAP_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(pThis->m_rwLock.TimedLockWrite(m_msec)) {
					pThis->m_state = WRAP_RWLOCK_WRITE;
				} else {
					throw wrap_timeout_error("LockWrite timeout!");
				}
			} else {
				assert(WRAP_RWLOCK_WRITE != m_state);
				pThis->m_rwLock.UpgradeLockRead();
				pThis->m_state = WRAP_RWLOCK_WRITE;
			}
		} else {
			assert(false);
		}
		return *pThis;
	}

	CScopedWrapRWLock& TimedUpgradeLockRead(uint32_t msec) const {
		CScopedWrapRWLock* pThis = const_cast<
			CScopedWrapRWLock*>(this);
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
			if(WRAP_RWLOCK_UNINIT == m_state) {
				pThis->m_rwLock.LockWrite();
				pThis->m_state = WRAP_RWLOCK_WRITE;
			} else if(WRAP_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(pThis->m_rwLock.TimedLockWrite(msec)) {
					pThis->m_state = WRAP_RWLOCK_WRITE;
				} else {
					throw wrap_timeout_error("LockWrite timeout!");
				}
			} else {
				assert(WRAP_RWLOCK_WRITE != m_state);
				if(pThis->m_rwLock.TimedUpgradeLockRead(msec)) {
					pThis->m_state = WRAP_RWLOCK_WRITE;
				} else {
					throw wrap_timeout_error("UpgradeLockRead timeout!");
				}
			}
		} else {
			assert(false);
		}
		return *pThis;
	}

private:
	enum eWrapRWLockState {
		WRAP_RWLOCK_UNINIT,
		WRAP_RWLOCK_UNINIT_TIMEOUT,
		WRAP_RWLOCK_READ,
		WRAP_RWLOCK_WRITE,
	};
private:
	IWrapRWLock& m_rwLock;
	util::CAutoPointer<UseType> m_data;
	volatile uint32_t m_threadId;
	enum eWrapRWLockState m_state;
	uint32_t m_msec;

	CScopedWrapRWLock(const CScopedWrapRWLock& orig) {}
	CScopedWrapRWLock& operator=(const CScopedWrapRWLock &orig) { return *this; }
};

}; // namespace thd


#endif // __SCOPEDWRAPRWLOCK_H_

/* end of header file */
