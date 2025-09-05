/*
 * File:   SpinRecursiveLock.h
 * Author: Jehu Shaw
 *
 * Created on 2013年10月1日, 上午10:32
 */

#ifndef SPINRECURSIVELOCK_H
#define	SPINRECURSIVELOCK_H

#include "Common.h"
#include "AtomicLock.h"
#include "SysCurrentThreadId.h"
#include "SysTickCount.h"

namespace thd {


	class CSpinRecursiveLock : public ILock
	{
	public:
		CSpinRecursiveLock(void)
			: m_lock(0)
			, m_threadId(0)
			, m_recursionCount(0)
		{
#if COMPILER == COMPILER_MICROSOFT
			BUILD_BUG_ON(sizeof(uint32_t) < sizeof(unsigned long));
#endif
		}
		~CSpinRecursiveLock(void) {
			m_recursionCount = 0;
			atomic_xchg(&m_threadId, 0);
			for(int i = 0; !Lockable(); ++i) {
				Unlock();
				cpu_relax(i);
			}
		}

		void Lock() throw() {

			const uint32_t threadId = GetSysCurrentThreadId();
			if(!Lockable()) {
				if(threadId == (uint32_t)m_threadId) {
					++m_recursionCount;
					return;
				}
			}

			uint16_t me = (uint16_t)atomic_xadd16(&m_lock.s.users, 1);

			for(int i = 0;; ++i)
			{
				union TicketLock u(m_lock.u);
				if(u.s.ticket == me) {
					break;
				}
				if(u.s.ticket == u.s.users) {
					break;
				}
				cpu_relax(i);
			}

			atomic_xchg(&m_threadId, threadId);
			m_recursionCount = 1;
		}

		bool TimedLock(uint32_t msec) throw() {

			const uint32_t threadId = GetSysCurrentThreadId();
			if(!Lockable()) {
				if(threadId == (uint32_t)m_threadId) {
					++m_recursionCount;
					return true;
				}
			}

			uint16_t me = (uint16_t)atomic_xadd16(&m_lock.s.users, 1);
			uint64_t startTime = GetSysTickCount();
			for(int i = 0;; ++i) {

				union TicketLock u(m_lock.u);
				if(u.s.ticket == me) {
					break;
				}

				if(u.s.ticket == u.s.users) {
					break;
				}

				if((int64_t)(GetSysTickCount() - 
					startTime) >= (int64_t)msec)
				{
					return false;
				}
				cpu_relax(i);
			}

			atomic_xchg(&m_threadId, threadId);
			m_recursionCount = 1;
			return true;
		}

		void Unlock() throw() {

			if(Lockable()) {
				return;
			}

			const uint32_t threadId = GetSysCurrentThreadId();
			if(threadId == (uint32_t)m_threadId) {
				if(m_recursionCount > 1) {
					--m_recursionCount;
					return;
				} else {
					m_recursionCount = 0;
					atomic_xchg(&m_threadId, 0);
				}
			}

			memory_barrier();
			m_lock.s.ticket++;
		}

		bool TryLock() throw() {

			const uint32_t threadId = GetSysCurrentThreadId();
			if(!Lockable()) {
				if(threadId == (uint32_t)m_threadId) {
					return true;
				}
			}

			uint16_t me = m_lock.s.users;
			union TicketLock ucmp(0);
			ucmp.s.ticket = me;
			ucmp.s.users = me;
			uint32_t cmp = ucmp.u;
			ucmp.s.users = me + 1;
			uint32_t cmpnew = ucmp.u;

			if(atomic_cmpxchg(&m_lock.u, cmp, cmpnew) == cmp) {
				atomic_xchg(&m_threadId, threadId);
				m_recursionCount = 1;
				return true;
			}
			return false;
		}

		bool Lockable() throw() {
			union TicketLock u(m_lock.u);
			return (u.s.ticket == u.s.users);
		}

		bool Using() throw() {
			return !Lockable();
		}

	protected:
		union TicketLock
		{
			volatile uint32_t u;
			struct
			{
				volatile uint16_t ticket;
				volatile uint16_t users;
			} s;

			TicketLock(uint32_t orig) : u(orig) {}
			TicketLock(const TicketLock& orig) : u(orig.u) {}
			TicketLock& operator = (const TicketLock& right) {
				this->u = right.u;
				return *this;
			}
		};

		union TicketLock m_lock;
		volatile uint32_t m_threadId;
		volatile int32_t m_recursionCount;

	private:
		CSpinRecursiveLock(const CSpinRecursiveLock& orig) : m_lock(0) {}
		CSpinRecursiveLock& operator=(const CSpinRecursiveLock& right) { return *this; }
	};
}

#endif  // SPINRECURSIVELOCK_H
