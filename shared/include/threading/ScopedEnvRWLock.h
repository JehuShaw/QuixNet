/*
 * File:   ScopedEnvRWLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef SCOPEDENVRWLOCK_H
#define SCOPEDENVRWLOCK_H

#include "EnvRWLock.h"
#include "ScopedPointer.h"

#ifndef IGNORE_THREAD_SAFE_CHECK
#include "SysCurrentThreadId.h"
#include "ThreadIndexManager.h"
#include "XqxTable1StoreS.h"
#endif

namespace thd {

// scoped read write lock help
template<class UseType>
class CScopedEnvRWLock
{
public:
	CScopedEnvRWLock(IEnvLockBase& lock)
		: m_rwLock(dynamic_cast<IEnvRWLock&>(lock))
		, m_data(dynamic_cast<IEnvLockData<UseType>&>(lock).GetDataPtr(), false)
		, m_state(ENV_RWLOCK_UNINIT)
		, m_msec(0)
#ifndef IGNORE_THREAD_SAFE_CHECK
		, m_envFlags(dynamic_cast<IEnvLockData<UseType>&>(lock).GetEnvInfo())
#endif
	{
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(uint32_t) < sizeof(unsigned long));
#endif
#ifndef IGNORE_THREAD_SAFE_CHECK
		const uint32_t threadId = GetSysCurrentThreadId();
		atomic_xchg(&m_threadId, threadId);
#endif
	}

	CScopedEnvRWLock(IEnvLockBase& lock, uint32_t msec)
		: m_rwLock(dynamic_cast<IEnvRWLock&>(lock))
		, m_data(dynamic_cast<IEnvLockData<UseType>&>(lock).GetDataPtr(), false)
		, m_state(ENV_RWLOCK_UNINIT_TIMEOUT)
		, m_msec(msec)
#ifndef IGNORE_THREAD_SAFE_CHECK
		, m_envFlags(dynamic_cast<IEnvLockData<UseType>&>(lock).GetEnvInfo())
#endif
	{
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(uint32_t) < sizeof(unsigned long));
#endif
#ifndef IGNORE_THREAD_SAFE_CHECK
		const uint32_t threadId = GetSysCurrentThreadId();
		atomic_xchg(&m_threadId, threadId);
#endif
	}

	~CScopedEnvRWLock() {
		m_data.SetRawPointer(NULL, false);
		m_rwLock.Unlock();
	}

	util::CScopedPointer<UseType> operator->() {
#ifndef IGNORE_THREAD_SAFE_CHECK
		assert(CheckEnvFlag());

		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
#endif
			if(ENV_RWLOCK_UNINIT == m_state) {
				m_rwLock.LockWrite();
				m_state = ENV_RWLOCK_WRITE;
			} else if(ENV_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(m_rwLock.TimedLockWrite(m_msec)) {
					m_state = ENV_RWLOCK_WRITE;
				} else {
					throw env_lock_timeout_error("LockWrite timeout!");
				}
			}
			if(ENV_RWLOCK_WRITE == m_state) {
				return m_data;
			} else if(ENV_RWLOCK_READ == m_state) {
				throw env_lock_readonly_error("Read only ! "
					"Use the method \"operator->() const\" please.");
			} else {
				assert(false);
				return util::CScopedPointer<UseType>();
			}
#ifndef IGNORE_THREAD_SAFE_CHECK
		} else {
			assert(false);
			return util::CScopedPointer<UseType>();
		}
#endif
	}

	util::CScopedPointer<const UseType> operator->() const {
#ifndef IGNORE_THREAD_SAFE_CHECK
		assert(CheckEnvFlag());

		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
#endif
			CScopedEnvRWLock& thisRef = const_cast<
				CScopedEnvRWLock&>(*this);
			if(ENV_RWLOCK_UNINIT == m_state) {
				thisRef.m_rwLock.LockRead();
				thisRef.m_state = ENV_RWLOCK_READ;
			} else if(ENV_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(thisRef.m_rwLock.TimedLockRead(m_msec)) {
					thisRef.m_state = ENV_RWLOCK_READ;
				} else {
					throw env_lock_timeout_error("LockRead timeout!");
				}
			}

			if(ENV_RWLOCK_READ == m_state
				|| ENV_RWLOCK_WRITE == m_state)
			{
				return m_data;
			} else {
				assert(false);
				return util::CScopedPointer<const UseType>();
			}
#ifndef IGNORE_THREAD_SAFE_CHECK
		} else {
			assert(false);
			return util::CScopedPointer<const UseType>();
		}
#endif
	}

	util::CScopedPointer<UseType> GetData() {
#ifndef IGNORE_THREAD_SAFE_CHECK
		assert(CheckEnvFlag());

		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
#endif
			if(ENV_RWLOCK_UNINIT == m_state) {
				m_rwLock.LockWrite();
				m_state = ENV_RWLOCK_WRITE;
			} else if(ENV_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(m_rwLock.TimedLockWrite(m_msec)) {
					m_state = ENV_RWLOCK_WRITE;
				} else {
					throw env_lock_timeout_error("LockWrite timeout!");
				}
			}
			if(ENV_RWLOCK_WRITE == m_state) {
				return m_data;
			} else if(ENV_RWLOCK_READ == m_state) {
				throw env_lock_readonly_error("Read only ! "
					"Use the method \"GetData() const\" please.");
			} else {
				assert(false);
				return util::CScopedPointer<UseType>();
			}
#ifndef IGNORE_THREAD_SAFE_CHECK
		} else {
			assert(false);
			return util::CScopedPointer<UseType>();
		}
#endif
	}

	util::CScopedPointer<const UseType> GetData() const {
#ifndef IGNORE_THREAD_SAFE_CHECK
		assert(CheckEnvFlag());

		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
#endif
			CScopedEnvRWLock& thisRef = const_cast<
				CScopedEnvRWLock&>(*this);
			if(ENV_RWLOCK_UNINIT == m_state) {
				thisRef.m_rwLock.LockRead();
				thisRef.m_state = ENV_RWLOCK_READ;
			} else if(ENV_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(thisRef.m_rwLock.TimedLockRead(m_msec)) {
					thisRef.m_state = ENV_RWLOCK_READ;
				} else {
					throw env_lock_timeout_error("LockRead timeout!");
				}
			}

			if(ENV_RWLOCK_READ == m_state
				|| ENV_RWLOCK_WRITE == m_state)
			{
				return m_data;
			} else {
				assert(false);
				return util::CScopedPointer<const UseType>();
			}
#ifndef IGNORE_THREAD_SAFE_CHECK
		} else {
			assert(false);
			return util::CScopedPointer<const UseType>();
		}
#endif
	}

	const CScopedEnvRWLock& DegradeLockWrite() {
#ifndef IGNORE_THREAD_SAFE_CHECK
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
#endif
			if(ENV_RWLOCK_UNINIT == m_state) {
				m_rwLock.LockRead();
				m_state = ENV_RWLOCK_READ;
			} else if(ENV_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(m_rwLock.TimedLockRead(m_msec)) {
					m_state = ENV_RWLOCK_READ;
				} else {
					throw env_lock_timeout_error("LockRead timeout!");
				}
			} else {
				assert(ENV_RWLOCK_READ != m_state);
				if(m_rwLock.DegradeLockWrite()) {
					m_state = ENV_RWLOCK_READ;
					// Let the previous pointer fail.
					const UseType& data = *m_data;
					m_data.SetRawPointer(&data, false);
				} else {
					assert(false);
				}
			}
#ifndef IGNORE_THREAD_SAFE_CHECK
		} else {
			assert(false);
		}
#endif
		return *this;
	}

	CScopedEnvRWLock& UpgradeLockRead() const {
		CScopedEnvRWLock& thisRef = const_cast<
			CScopedEnvRWLock&>(*this);
#ifndef IGNORE_THREAD_SAFE_CHECK
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
#endif
			if(ENV_RWLOCK_UNINIT == m_state) {
				thisRef.m_rwLock.LockWrite();
				thisRef.m_state = ENV_RWLOCK_WRITE;
			} else if(ENV_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(thisRef.m_rwLock.TimedLockWrite(m_msec)) {
					thisRef.m_state = ENV_RWLOCK_WRITE;
				} else {
					throw env_lock_timeout_error("LockWrite timeout!");
				}
			} else {
				assert(ENV_RWLOCK_WRITE != m_state);
				thisRef.m_rwLock.UpgradeLockRead();
				thisRef.m_state = ENV_RWLOCK_WRITE;
			}
#ifndef IGNORE_THREAD_SAFE_CHECK
		} else {
			assert(false);
		}
#endif
		return thisRef;
	}

	CScopedEnvRWLock& TimedUpgradeLockRead(uint32_t msec) const {
		CScopedEnvRWLock& thisRef = const_cast<
			CScopedEnvRWLock&>(*this);
#ifndef IGNORE_THREAD_SAFE_CHECK
		const uint32_t threadId = GetSysCurrentThreadId();
		if(threadId == m_threadId) {
#endif
			if(ENV_RWLOCK_UNINIT == m_state) {
				thisRef.m_rwLock.LockWrite();
				thisRef.m_state = ENV_RWLOCK_WRITE;
			} else if(ENV_RWLOCK_UNINIT_TIMEOUT == m_state) {
				if(thisRef.m_rwLock.TimedLockWrite(msec)) {
					thisRef.m_state = ENV_RWLOCK_WRITE;
				} else {
					throw env_lock_timeout_error("LockWrite timeout!");
				}
			} else {
				assert(ENV_RWLOCK_WRITE != m_state);
				if(thisRef.m_rwLock.TimedUpgradeLockRead(msec)) {
					thisRef.m_state = ENV_RWLOCK_WRITE;
				} else {
					throw env_lock_timeout_error("UpgradeLockRead timeout!");
				}
			}
#ifndef IGNORE_THREAD_SAFE_CHECK
		} else {
			assert(false);
		}
#endif
		return thisRef;
	}

	bool operator==(const CScopedEnvRWLock<UseType>& right) const {
		return m_data == right.m_data;
	}

	bool operator!=(const CScopedEnvRWLock<UseType>& right) const {
		return m_data != right.m_data;
	}

	bool operator>(const CScopedEnvRWLock<UseType>& right) const {
		return m_data > right.m_data;
	}

	bool operator<(const CScopedEnvRWLock<UseType>& right) const {
		return m_data < right.m_data;
	}
	protected:
		void SetEnvFlag() {
#ifndef IGNORE_THREAD_SAFE_CHECK
			uint64_t thdIndex = CThreadIndexManager::Pointer()->Get();
			if(!m_envFlags.Has(thdIndex)) { 
				m_envFlags.Add(thdIndex, true);
			}
#endif
		}

		void RemoveEnvFlag() {
#ifndef IGNORE_THREAD_SAFE_CHECK
			uint64_t thdIndex = CThreadIndexManager::Pointer()->Get();
			m_envFlags.Remove(thdIndex);
#endif
		}

#ifndef IGNORE_THREAD_SAFE_CHECK
	private:
		bool CheckEnvFlag() const {
			uint64_t thdIndex = CThreadIndexManager::Pointer()->Get();
			if(m_envFlags.Has(thdIndex)) { 
				return m_envFlags.Find(thdIndex);
			}
			return false;
		}
#endif

private:
	enum eEnvRWLockState {
		ENV_RWLOCK_UNINIT,
		ENV_RWLOCK_UNINIT_TIMEOUT,
		ENV_RWLOCK_READ,
		ENV_RWLOCK_WRITE,
	};
private:
	IEnvRWLock& m_rwLock;
	util::CAutoPointer<UseType> m_data;
#ifndef IGNORE_THREAD_SAFE_CHECK
	volatile uint32_t m_threadId;
#endif
	enum eEnvRWLockState m_state;
	uint32_t m_msec;
#ifndef IGNORE_THREAD_SAFE_CHECK
	util::CXqxTable1StoreS<bool>& m_envFlags;
#endif

	CScopedEnvRWLock(const CScopedEnvRWLock& orig) {}
	CScopedEnvRWLock& operator=(const CScopedEnvRWLock &orig) { return *this; }
};

}; // namespace thd


#endif // SCOPEDENVRWLOCK_H

/* end of header file */
