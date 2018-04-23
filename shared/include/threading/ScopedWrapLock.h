/*
 * File:   ScopedWrapLock.h
 * Author: Jehu Shaw
 *
 */

#ifndef __SCOPEDWRAPLOCK_H_
#define __SCOPEDWRAPLOCK_H_

#include "WrapLock.h"
#include "ScopedPointer.h"
#include "SysCurrentThreadId.h"

namespace thd {

	template<class UseType>
	class CScopedWrapLock
	{
	public:
		/** Construct a new CScopedock for the given Lock. The Lock is locked
		* immediately.
		*/
		CScopedWrapLock(IWrapBase& lock)
			: m_lock(dynamic_cast<IWrapLock&>(lock))
			, m_data(dynamic_cast<IWrapData<UseType>&>(lock).GetDataPtr(), false)
			, m_state(WRAP_LOCK_UNINIT)
			, m_msec(0)
		{
			const uint32_t threadId = GetSysCurrentThreadId();
			atomic_xchg(&m_threadId, threadId);
		}

		CScopedWrapLock(IWrapBase& lock, uint32_t msec)
			: m_lock(dynamic_cast<IWrapLock&>(lock))
			, m_data(dynamic_cast<IWrapData<UseType>&>(lock).GetDataPtr(), false)
			, m_state(WRAP_LOCK_UNINIT_TIMEOUT)
			, m_msec(msec)
		{
			const uint32_t threadId = GetSysCurrentThreadId();
			atomic_xchg(&m_threadId, threadId);
		}

		/** Destructor. Unlocks the Lock. */
		~CScopedWrapLock()
		{
			m_data.SetRawPointer(NULL, false);
			if(WRAP_LOCK_LOCKED == m_state) {
				m_lock.Unlock();
			}
		}

		util::CScopedPointer<UseType> operator->()
		{	
			const uint32_t threadId = GetSysCurrentThreadId();
			if(threadId == m_threadId) {
				if(WRAP_LOCK_UNINIT == m_state) {
					m_lock.Lock();
					m_state = WRAP_LOCK_LOCKED;
				} else if(WRAP_LOCK_UNINIT_TIMEOUT == m_state) {
					if(m_lock.TimedLock(m_msec)) {
						m_state = WRAP_LOCK_LOCKED;
					} else {
						throw wrap_timeout_error("Lock timeout!");
					}
				}
				return m_data;
			} else {
				assert(false);
				return util::CScopedPointer<UseType>();
			}
		}

		util::CScopedPointer<const UseType> operator->() const
		{
			const uint32_t threadId = GetSysCurrentThreadId();
			if(threadId == m_threadId) {
				CScopedWrapLock* pThis = const_cast<
					CScopedWrapLock*>(this);
				if(WRAP_LOCK_UNINIT == m_state) {
					pThis->m_lock.Lock();
					pThis->m_state = WRAP_LOCK_LOCKED;
				} else if(WRAP_LOCK_UNINIT_TIMEOUT == m_state) {
					if(pThis->m_lock.TimedLock(m_msec)) {
						pThis->m_state = WRAP_LOCK_LOCKED;
					} else {
						throw wrap_timeout_error("Lock timeout!");
					}
				}
				return m_data;
			} else {
				assert(false);
				return util::CScopedPointer<const UseType>();
			}
		}

		util::CScopedPointer<UseType> GetData()
		{
			const uint32_t threadId = GetSysCurrentThreadId();
			if(threadId == m_threadId) {
				if(WRAP_LOCK_UNINIT == m_state) {
					m_lock.Lock();
					m_state = WRAP_LOCK_LOCKED;
				} else if(WRAP_LOCK_UNINIT_TIMEOUT == m_state) {
					if(m_lock.TimedLock(m_msec)) {
						m_state = WRAP_LOCK_LOCKED;
					} else {
						throw wrap_timeout_error("Lock timeout!");
					}
				}
				return m_data;
			} else {
				assert(false);
				return util::CScopedPointer<UseType>();
			} 
		}

		util::CScopedPointer<const UseType> GetData() const
		{
			const uint32_t threadId = GetSysCurrentThreadId();
			if(threadId == m_threadId) {
				CScopedWrapLock* pThis = const_cast<
					CScopedWrapLock*>(this);
				if(WRAP_LOCK_UNINIT == m_state) {
					pThis->m_lock.Lock();
					pThis->m_state = WRAP_LOCK_LOCKED;
				} else if(WRAP_LOCK_UNINIT_TIMEOUT == m_state) {
					if(pThis->m_lock.TimedLock(m_msec)) {
						pThis->m_state = WRAP_LOCK_LOCKED;
					} else {
						throw wrap_timeout_error("Lock timeout!");
					}
				}
				return m_data;
			} else {
				assert(false);
				return util::CScopedPointer<const UseType>();
			} 
		}

	private:
		enum eWrapLockState {
			WRAP_LOCK_UNINIT,
			WRAP_LOCK_UNINIT_TIMEOUT,
			WRAP_LOCK_LOCKED,
		};
	private:
		IWrapLock& m_lock;
		util::CAutoPointer<UseType> m_data;
		volatile uint32_t m_threadId;
		enum eWrapLockState m_state;
		uint32_t m_msec;

		CScopedWrapLock(const CScopedWrapLock& orig){}
		CScopedWrapLock& operator=(const CScopedWrapLock &orig) { return *this; }
	};

}; // namespace thd


#endif // __SCOPEDWRAPLOCK_H_

/* end of header file */
