/*
 * File:   ScopedEnvLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef SCOPEDENVLOCK_H
#define SCOPEDENVLOCK_H

#include "EnvLock.h"
#include "ScopedPointer.h"

#ifndef IGNORE_THREAD_SAFE_CHECK
#include "SysCurrentThreadId.h"
#include "ThreadIndexManager.h"
#include "XqxTable1StoreS.h"
#endif

namespace thd {

	template<class UseType>
	class CScopedEnvLock
	{
	public:
		/** Construct a new CScopedock for the given Lock. The Lock is locked
		* immediately.
		*/
		CScopedEnvLock(IEnvLockBase& lock)
			: m_lock(dynamic_cast<IEnvLock&>(lock))
			, m_data(dynamic_cast<IEnvLockData<UseType>&>(lock).GetDataPtr(), false)
			, m_state(ENV_LOCK_UNINIT)
			, m_msec(0)
#ifndef IGNORE_THREAD_SAFE_CHECK
			, m_envFlags(dynamic_cast<IEnvLockData<UseType>&>(lock).GetEnvInfo())
#endif
		{
#ifndef IGNORE_THREAD_SAFE_CHECK
			const uint32_t threadId = GetSysCurrentThreadId();
			atomic_xchg(&m_threadId, threadId);
#endif
		}

		CScopedEnvLock(IEnvLockBase& lock, uint32_t msec)
			: m_lock(dynamic_cast<IEnvLock&>(lock))
			, m_data(dynamic_cast<IEnvLockData<UseType>&>(lock).GetDataPtr(), false)
			, m_state(ENV_LOCK_UNINIT_TIMEOUT)
			, m_msec(msec)
#ifndef IGNORE_THREAD_SAFE_CHECK
			, m_envFlags(dynamic_cast<IEnvLockData<UseType>&>(lock).GetEnvInfo())
#endif
		{
#ifndef IGNORE_THREAD_SAFE_CHECK
			const uint32_t threadId = GetSysCurrentThreadId();
			atomic_xchg(&m_threadId, threadId);
#endif
		}

		/** Destructor. Unlocks the Lock. */
		~CScopedEnvLock()
		{
			m_data.SetRawPointer(NULL, false);
			if(ENV_LOCK_LOCKED == m_state) {
				m_lock.Unlock();
			}
		}

		util::CScopedPointer<UseType> operator->()
		{
#ifndef IGNORE_THREAD_SAFE_CHECK
			assert(CheckEnvFlag());

			const uint32_t threadId = GetSysCurrentThreadId();
			if(threadId == m_threadId) {
#endif
				if(ENV_LOCK_UNINIT == m_state) {
					m_lock.Lock();
					m_state = ENV_LOCK_LOCKED;
				} else if(ENV_LOCK_UNINIT_TIMEOUT == m_state) {
					if(m_lock.TimedLock(m_msec)) {
						m_state = ENV_LOCK_LOCKED;
					} else {
						throw env_lock_timeout_error("Lock timeout!");
					}
				}
				return m_data;
#ifndef IGNORE_THREAD_SAFE_CHECK
			} else {
				assert(false);
				return util::CScopedPointer<UseType>();
			}
#endif
		}

		util::CScopedPointer<const UseType> operator->() const
		{
#ifndef IGNORE_THREAD_SAFE_CHECK
			assert(CheckEnvFlag());

			const uint32_t threadId = GetSysCurrentThreadId();
			if(threadId == m_threadId) {
#endif
				CScopedEnvLock& thisRef = const_cast<
					CScopedEnvLock&>(*this);
				if(ENV_LOCK_UNINIT == m_state) {
					thisRef.m_lock.Lock();
					thisRef.m_state = ENV_LOCK_LOCKED;
				} else if(ENV_LOCK_UNINIT_TIMEOUT == m_state) {
					if(thisRef.m_lock.TimedLock(m_msec)) {
						thisRef.m_state = ENV_LOCK_LOCKED;
					} else {
						throw env_lock_timeout_error("Lock timeout!");
					}
				}
				return m_data;
#ifndef IGNORE_THREAD_SAFE_CHECK
			} else {
				assert(false);
				return util::CScopedPointer<const UseType>();
			}
#endif
		}

		util::CScopedPointer<UseType> GetData()
		{
#ifndef IGNORE_THREAD_SAFE_CHECK
			assert(CheckEnvFlag());

			const uint32_t threadId = GetSysCurrentThreadId();
			if(threadId == m_threadId) {
#endif
				if(ENV_LOCK_UNINIT == m_state) {
					m_lock.Lock();
					m_state = ENV_LOCK_LOCKED;
				} else if(ENV_LOCK_UNINIT_TIMEOUT == m_state) {
					if(m_lock.TimedLock(m_msec)) {
						m_state = ENV_LOCK_LOCKED;
					} else {
						throw env_lock_timeout_error("Lock timeout!");
					}
				}
				return m_data;
#ifndef IGNORE_THREAD_SAFE_CHECK
			} else {
				assert(false);
				return util::CScopedPointer<UseType>();
			}
#endif
		}

		util::CScopedPointer<const UseType> GetData() const
		{
#ifndef IGNORE_THREAD_SAFE_CHECK
			assert(CheckEnvFlag());

			const uint32_t threadId = GetSysCurrentThreadId();
			if(threadId == m_threadId) {
#endif
				CScopedEnvLock& thisRef = const_cast<
					CScopedEnvLock&>(*this);
				if(ENV_LOCK_UNINIT == m_state) {
					thisRef.m_lock.Lock();
					thisRef.m_state = ENV_LOCK_LOCKED;
				} else if(ENV_LOCK_UNINIT_TIMEOUT == m_state) {
					if(thisRef.m_lock.TimedLock(m_msec)) {
						thisRef.m_state = ENV_LOCK_LOCKED;
					} else {
						throw env_lock_timeout_error("Lock timeout!");
					}
				}
				return m_data;
#ifndef IGNORE_THREAD_SAFE_CHECK
			} else {
				assert(false);
				return util::CScopedPointer<const UseType>();
			}
#endif
		}

		bool operator==(const CScopedEnvLock<UseType>& right) const {
			return m_data == right.m_data;
		}

		bool operator!=(const CScopedEnvLock<UseType>& right) const {
			return m_data != right.m_data;
		}

		bool operator>(const CScopedEnvLock<UseType>& right) const {
			return m_data > right.m_data;
		}

		bool operator<(const CScopedEnvLock<UseType>& right) const {
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
		enum eEnvLockState {
			ENV_LOCK_UNINIT,
			ENV_LOCK_UNINIT_TIMEOUT,
			ENV_LOCK_LOCKED,
		};
	private:
		IEnvLock& m_lock;
		util::CAutoPointer<UseType> m_data;
#ifndef IGNORE_THREAD_SAFE_CHECK
		volatile uint32_t m_threadId;
#endif
		enum eEnvLockState m_state;
		uint32_t m_msec;
#ifndef IGNORE_THREAD_SAFE_CHECK
		util::CXqxTable1StoreS<bool>& m_envFlags;
#endif
		CScopedEnvLock(const CScopedEnvLock& orig){}
		CScopedEnvLock& operator=(const CScopedEnvLock &orig) { return *this; }
	};

}; // namespace thd


#endif // SCOPEDENVLOCK_H

/* end of header file */
