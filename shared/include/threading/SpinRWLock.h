/*
 * File:   SpinRWLock.h
 * Author: Jehu Shaw
 *
 * Created on 2013年10月1日, 上午10:32
 */

#ifndef __SPINRWLOCK_H_
#define	__SPINRWLOCK_H_

#include "SpinLock.h"
#include "IRWLock.h"

namespace thd {

	class CSpinRWLock : private CSpinLock, public IRWLock {
	public:
		CSpinRWLock()
			: CSpinLock()
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
				if(!Lockable()) {
					Unlock();
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
			Lock();

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
			if(!TimedLock(msec, startTime)) {
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

		void UnlockWrite() throw() {
			if(!AtomicIncWaitDone()) {
				return;
			}
			Unlock();
			AtomicDecWaitDone();
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
			if(!TryLock()) {
				AtomicDecWaitDone();
				return false;
			}

			if(m_readers) {
				/* Oops, a reader started */
				Unlock();
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
				if(Lockable()) {
					/* Speculatively take read lock */
					AtomicIncReader();

					/* Success? */
					if(Lockable()) {
						AtomicDecWaitDone();
						return;
					}

					/* Failure - undo, and wait until we can try again */
					AtomicDecReader();
				}

				for(int i = 0; !Lockable(); ++i) {
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
				if(Lockable()) {
					/* Speculatively take read lock */
					AtomicIncReader();

					/* Success? */
					if(Lockable()) {
						AtomicDecWaitDone();
						return true;
					}

					/* Failure - undo, and wait until we can try again */
					AtomicDecReader();
				}

				for(int i = 0; !Lockable(); ++i) {
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

		void UnlockRead() throw() {
			if(!AtomicIncWaitDone()) {
				return;
			}
			AtomicDecReader();
			AtomicDecWaitDone();
		}

		bool TryLockRead() throw() {
			if(!AtomicIncWaitDone()) {
				return false;
			}
			/* Speculatively take read lock */
			AtomicIncReader();

			/* Success? */
			if(Lockable()) {
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
			/* Try lock */	
			for(int k = 0; !this->TryLock(); ++k) {
				cpu_relax(k);
			}

			/* I'm no longer a reader */
			AtomicDecReader();

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
			uint64_t startTime = GetSysTickCount();
			/* Try lock */	
			for(int k = 0; !this->TryLock(); ++k) {
				if((int64_t)(GetSysTickCount() - 
					startTime) >= (int64_t)msec)
				{
					AtomicDecWaitDone();
					return false;
				}
				cpu_relax(k);
			}

			/* I'm no longer a reader */
			AtomicDecReader();

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
			if(Lockable()) {
				AtomicDecWaitDone();
				return false;
			}

			/* Speculatively take read lock */
			AtomicIncReader();

			/* Release write lock. */
			Unlock();
			AtomicDecWaitDone();
			return true;
		}

		bool Using() throw() {
			return !Lockable() || 0 != m_readers;
		}

	private:
		inline void AtomicIncReader() throw() {
			uint32_t readers;
			do {
				readers = (uint32_t)m_readers;
				if(readers == (uint32_t)-1) {
					return;
				}
			} while (atomic_cmpxchg(&m_readers, readers + 1, readers) != readers);
		}
		inline void AtomicDecReader() throw() {
			uint32_t readers;
			do {
				readers = (uint32_t)m_readers;
				if(readers == 0) {
					return;
				}
			} while (atomic_cmpxchg(&m_readers, readers - 1, readers) != readers);
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
				waitDone + 1, waitDone) != waitDone);
			return true;
		}
		inline void AtomicDecWaitDone() throw() {
			int32_t waitDone;
			do {
				waitDone = (int32_t)m_waitDone;
				if(waitDone == 0) {
					return;
				}
			} while (atomic_cmpxchg(&m_waitDone, (waitDone < 0 ?
				(waitDone + 1) : (waitDone - 1)), waitDone) != waitDone);
		}

		inline bool AtomicSetWaitDone() throw() {
			int32_t waitDone;
			do {
				waitDone = (int32_t)m_waitDone;
				if(waitDone < 0) {
					return false;
				}
			} while (atomic_cmpxchg(&m_waitDone,
				-(waitDone + 1), waitDone) != waitDone);
			return true;
		}

		inline bool CheckWaitDone() throw() {
			return (int32_t)m_waitDone != -1;
		}

		volatile uint32_t m_readers;
		volatile int32_t m_waitDone;

	private:
		CSpinRWLock(const CSpinRWLock& orig) {}
		CSpinRWLock& operator=(const CSpinRWLock& right) { return *this; }
	};
}

#endif  // __SPINRWLOCK_H_
