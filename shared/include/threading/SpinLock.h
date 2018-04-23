/*
 * File:   SpinLock.h
 * Author: Jehu Shaw
 *
 * Created on 2013年10月1日, 上午10:32
 */

#ifndef SPINLOCK_H_
#define	SPINLOCK_H_

#include "Common.h"
#include "AtomicLock.h"
#include "SysTickCount.h"

namespace thd {


	class CSpinLock : public ILock
	{
	public:
		CSpinLock(void) : m_lock(0) {
#if COMPILER == COMPILER_MICROSOFT
			BUILD_BUG_ON(sizeof(uint32_t) < sizeof(unsigned long));
#endif
		}
		~CSpinLock(void) {
			for(int i = 0; !Lockable(); ++i) {
				Unlock();
				cpu_relax(i);
			}
		}

		void Lock() throw() {

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

		}

		bool TimedLock(uint32_t msec, uint64_t startTime) throw() {

			uint16_t me = (uint16_t)atomic_xadd16(&m_lock.s.users, 1);

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
			return true;
		}

		bool TimedLock(uint32_t msec) throw() {
			return TimedLock(msec, GetSysTickCount());
		}

		void Unlock() throw() {

			memory_barrier();
			m_lock.s.ticket++;
		}

		bool TryLock() throw() {

			uint16_t me = m_lock.s.users;
			union TicketLock ucmp(0);
			ucmp.s.ticket = me;
			ucmp.s.users = me;
			uint32_t cmp = ucmp.u;
			ucmp.s.users = me + 1;
			uint32_t cmpnew = ucmp.u;

			if(atomic_cmpxchg(&m_lock.u, cmpnew, cmp) == cmp) {
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

	private:
		CSpinLock(const CSpinLock& orig) : m_lock(0) {}
		CSpinLock& operator=(const CSpinLock& right) { return *this; }
	};
}

#endif  // SPINLOCK_H_
