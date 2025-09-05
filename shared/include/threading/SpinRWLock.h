/*
 * File:   SpinRWLock.h
 * Author: Jehu Shaw
 *
 * Created on 2013年10月1日, 上午10:32
 */

#ifndef SPINRWLOCK_H
#define	SPINRWLOCK_H

#include "SpinLock.h"
#include "IRWLock.h"

namespace thd {

	class CSpinRWLock : public IRWLock {
	public:
		CSpinRWLock()
			: m_lock()
			, m_readers(0)
			, m_waitDone(0)
		{
#if COMPILER == COMPILER_MICROSOFT
			BUILD_BUG_ON(sizeof(uint32_t) < sizeof(unsigned long));
#endif
		}

        ~CSpinRWLock() {
			if(!AtomicSetWaitDone()) {
				return;
			}
			// Wait all done.
			for(int i = 0; CheckWaitDone(); ++i) {
				if(!m_lock.Lockable()) {
					m_lock.Unlock();
				}
				if(m_readers) {
					atomic_xchg(&m_readers, 0);
				}
				cpu_relax(i);
			}
        }

		void LockWrite() throw() {
			if(!AtomicIncWaitDone()) {
				return;
			}
			/* Get lock */
			m_lock.Lock();

			/* Wait for readers to finish */
			for(int i = 0; m_readers; ++i) {
				cpu_relax(i);
			}
			AtomicDecWaitDone();
		}

		bool TimedLockWrite(uint32_t msec) throw() {
			if(!AtomicIncWaitDone()) {
				return true;
			}
			uint64_t startTime = GetSysTickCount();
			/* Get lock */
			if(!m_lock.TimedLock(msec, startTime)) {
				AtomicDecWaitDone();
				return false;
			}

			/* Wait for readers to finish */
			for(int i = 0; m_readers; ++i) {
				if((int64_t)(GetSysTickCount() - 
					startTime) >= (int64_t)msec)
				{
					AtomicDecWaitDone();
					return false;
				}
				cpu_relax(i);
			}
			AtomicDecWaitDone();
			return true;
		}

		bool TryLockWrite() throw() {
			if(!AtomicIncWaitDone()) {
				return false;
			}
			/* Want no readers */
			if(m_readers) {
				AtomicDecWaitDone();
				return false;
			}

			/* Try to get write lock */
			if(!m_lock.TryLock()) {
				AtomicDecWaitDone();
				return false;
			}

			if(m_readers) {
				/* Oops, a reader started */
				m_lock.Unlock();
				AtomicDecWaitDone();
				return false;
			}

			/* Success! */
			AtomicDecWaitDone();
			return true;
		}

		void LockRead() throw() {
			if(!AtomicIncWaitDone()) {
				return;
			}
			while (true) {
				/* Success? */
				if(m_lock.Lockable()) {
					/* Speculatively take read lock */
					AtomicIncReader();

					/* Success? */
					if(m_lock.Lockable()) {
						AtomicDecWaitDone();
						return;
					}

					/* Failure - undo, and wait until we can try again */
					AtomicDecReader();
				}

				for(int i = 0; !m_lock.Lockable(); ++i) {
					cpu_relax(i);
				}
			}
			AtomicDecWaitDone();
		}

		bool TimedLockRead(uint32_t msec) throw() {
			if(!AtomicIncWaitDone()) {
				return true;
			}
			uint64_t startTime = GetSysTickCount();
			while (true) {
				/* Success? */
				if(m_lock.Lockable()) {
					/* Speculatively take read lock */
					AtomicIncReader();

					/* Success? */
					if(m_lock.Lockable()) {
						AtomicDecWaitDone();
						return true;
					}

					/* Failure - undo, and wait until we can try again */
					AtomicDecReader();
				}

				for(int i = 0; !m_lock.Lockable(); ++i) {
					if((int64_t)(GetSysTickCount() - 
						startTime) >= (int64_t)msec)
					{
						AtomicDecWaitDone();
						return false;
					}
					cpu_relax(i);
				}
			}
			AtomicDecWaitDone();
			return false;
		}

		void Unlock() throw() {
			if(!AtomicIncWaitDone()) {
				return;
			}
			if (!m_lock.Lockable()) {
				m_lock.Unlock();
			} else {
				AtomicDecReader();
			}
			AtomicDecWaitDone();
		}

		bool TryLockRead() throw() {
			if(!AtomicIncWaitDone()) {
				return false;
			}
			/* Speculatively take read lock */
			AtomicIncReader();

			/* Success? */
			if(m_lock.Lockable()) {
				AtomicDecWaitDone();
				return true;
			}

			/* Failure - undo */
			AtomicDecReader();
			AtomicDecWaitDone();
			return false;

		}

		void UpgradeLockRead() throw() {
			if(!AtomicIncWaitDone()) {
				return;
			}
			/* I'm no longer a reader */
			AtomicDecReader();

			/* Try lock */	
			for(int k = 0; !m_lock.TryLock(); ++k) {
				cpu_relax(k);
			}

			/* Wait for all other readers to finish */
			for(int i = 0; m_readers; ++i) {
				cpu_relax(i);
			}
			AtomicDecWaitDone();
		}

		bool TimedUpgradeLockRead(uint32_t msec) throw() {
			if(!AtomicIncWaitDone()) {
				return false;
			}
			/* I'm no longer a reader */
			AtomicDecReader();

			uint64_t startTime = GetSysTickCount();
			/* Try lock */	
			for(int k = 0; !m_lock.TryLock(); ++k) {
				if((int64_t)(GetSysTickCount() - 
					startTime) >= (int64_t)msec)
				{
					AtomicDecWaitDone();
					return false;
				}
				cpu_relax(k);
			}

			/* Wait for all other readers to finish */
			for(int i = 0; m_readers; ++i) {
				if((int64_t)(GetSysTickCount() - 
					startTime) >= (int64_t)msec)
				{
					AtomicDecWaitDone();
					return false;
				}
				cpu_relax(i);
			}
			AtomicDecWaitDone();
			return true;
		}

		bool DegradeLockWrite() throw() {
			if(!AtomicIncWaitDone()) {
				return false;
			}
			/* It is write lock ? */
			if(m_lock.Lockable()) {
				AtomicDecWaitDone();
				return false;
			}

			/* Speculatively take read lock */
			AtomicIncReader();

			/* Release write lock. */
			m_lock.Unlock();
			AtomicDecWaitDone();
			return true;
		}

		bool Using() throw() {
			return !m_lock.Lockable() || 0 != m_readers;
		}

	private:
		inline void AtomicIncReader() throw() {
			uint32_t readers;
			do {
				readers = (uint32_t)m_readers;
				if(readers == (uint32_t)-1) {
					return;
				}
			} while (atomic_cmpxchg(&m_readers, readers, readers + 1) != readers);
		}
		inline void AtomicDecReader() throw() {
			uint32_t readers;
			do {
				readers = (uint32_t)m_readers;
				if(readers == 0) {
					return;
				}
			} while (atomic_cmpxchg(&m_readers, readers, readers - 1) != readers);
		}

	private:
		inline bool AtomicIncWaitDone() throw() {
			int32_t waitDone;
			do {
				waitDone = (int32_t)m_waitDone;
				if(waitDone < 0) {
					return false;
				}
			} while (atomic_cmpxchg(&m_waitDone,
				waitDone, waitDone + 1) != waitDone);
			return true;
		}
		inline void AtomicDecWaitDone() throw() {
			int32_t waitDone;
			do {
				waitDone = (int32_t)m_waitDone;
				if(waitDone == 0) {
					return;
				}
			} while (atomic_cmpxchg(&m_waitDone, waitDone, 
				(waitDone < 0 ? (waitDone + 1) : (waitDone - 1))) != waitDone);
		}

		inline bool AtomicSetWaitDone() throw() {
			int32_t waitDone;
			do {
				waitDone = (int32_t)m_waitDone;
				if(waitDone < 0) {
					return false;
				}
			} while (atomic_cmpxchg(&m_waitDone,
				waitDone, -(waitDone + 1)) != waitDone);
			return true;
		}

		inline bool CheckWaitDone() throw() {
			return (int32_t)m_waitDone != -1;
		}

		CSpinLock m_lock;
		volatile uint32_t m_readers;
		volatile int32_t m_waitDone;

	private:
		CSpinRWLock(const CSpinRWLock& orig) {}
		CSpinRWLock& operator=(const CSpinRWLock& right) { return *this; }
	};
}

#endif  // SPINRWLOCK_H
