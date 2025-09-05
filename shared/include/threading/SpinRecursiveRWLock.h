/*
 * File:   SpinRecursiveRWLock.h
 * Author: Jehu Shaw
 *
 * Created on 2013年10月1日, 上午10:32
 */

#ifndef SPINRECURSIVERWLOCK_H
#define SPINRECURSIVERWLOCK_H

#include "SpinLock.h"
#include "IRWLock.h"
#include "SysCurrentThreadId.h"
#include "ThreadIndexManager.h"
#include "XqxTable1StoreS.h"


namespace thd {

#define MAX_READ_THREAD_SIZE 256

	class CSpinRecursiveRWLock : public IRWLock {
	public:
		CSpinRecursiveRWLock(uint32_t readThreadSize = MAX_READ_THREAD_SIZE)
			: m_lock()
			, m_readers(0)
			, m_threadId(0)
			, m_recursionCount(0)
			, m_waitDone(0)
		{
#if COMPILER == COMPILER_MICROSOFT
			BUILD_BUG_ON(sizeof(uint32_t) < sizeof(unsigned long));
#endif
		}

        ~CSpinRecursiveRWLock() {
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
			m_recursionCount = 0;
			atomic_xchg(&m_threadId, 0);
        }

		void LockWrite() throw() {
			if(!AtomicIncWaitDone()) {
				return;
			}
			const uint32_t threadId = GetSysCurrentThreadId();
			if(!m_lock.Lockable()) {
				if(threadId == (uint32_t)m_threadId) {
					++m_recursionCount;
					AtomicDecWaitDone();
					return;
				}
			}

			/* Get lock */
			m_lock.Lock();

			/* Wait for readers to finish */
			for(int i = 0; m_readers; ++i) {
				cpu_relax(i);
			}

			atomic_xchg(&m_threadId, threadId);
			m_recursionCount = 1;
			AtomicDecWaitDone();
		}

		bool TimedLockWrite(uint32_t msec) throw() {
			if(!AtomicIncWaitDone()) {
				return true;
			}
			const uint32_t threadId = GetSysCurrentThreadId();
			if(!m_lock.Lockable()) {
				if(threadId == (uint32_t)m_threadId) {
					++m_recursionCount;
					AtomicDecWaitDone();
					return true;
				}
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

			atomic_xchg(&m_threadId, threadId);
			m_recursionCount = 1;
			AtomicDecWaitDone();
			return true;
		}

		bool TryLockWrite() throw() {
			if(!AtomicIncWaitDone()) {
				return false;
			}
			const uint32_t threadId = GetSysCurrentThreadId();
			if(!m_lock.Lockable()) {
				if(threadId == (uint32_t)m_threadId) {
					AtomicDecWaitDone();
					return true;
				}
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
			atomic_xchg(&m_threadId, threadId);
			m_recursionCount = 1;
			AtomicDecWaitDone();
			return true;
		}

		void LockRead() throw() {
			if(!AtomicIncWaitDone()) {
				return;
			}
			if(!m_lock.Lockable()) {
				const uint32_t threadId = GetSysCurrentThreadId();
				if(threadId == (uint32_t)m_threadId) {
					AtomicDecWaitDone();
					return;
				}
			}

			if(IncThreadReader()) {
				AtomicDecWaitDone();
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
			if(!m_lock.Lockable()) {
				const uint32_t threadId = GetSysCurrentThreadId();
				if(threadId == (uint32_t)m_threadId) {
					AtomicDecWaitDone();
					return true;
				}
			}

			if(IncThreadReader()) {
				AtomicDecWaitDone();
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
			if(!m_lock.Lockable()) {
				const uint32_t threadId = GetSysCurrentThreadId();
				if (threadId == (uint32_t)m_threadId) {
					if (m_recursionCount > 1) {
						--m_recursionCount;
						AtomicDecWaitDone();
						return;
					} else {
						m_recursionCount = 0;
						atomic_xchg(&m_threadId, 0);
					}
				}
				m_lock.Unlock();
			} else {
				if (DecThreadReader()) {
					AtomicDecWaitDone();
					return;
				}
				AtomicDecReader();
			}
			AtomicDecWaitDone();
		}

		bool TryLockRead() throw() {
			if(!AtomicIncWaitDone()) {
				return false;
			}
			if(!m_lock.Lockable()) {
				const uint32_t threadId = GetSysCurrentThreadId();
				if(threadId == (uint32_t)m_threadId) {
					AtomicDecWaitDone();
					return false;
				}
			}

			if(HasThreadReader()) {
				AtomicDecWaitDone();
				return false;
			}
			/* Speculatively take read lock */
			AtomicIncReader();

			/* Success? */
			if(m_lock.Lockable()) {
				IncThreadReader();
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
			const uint32_t threadId = GetSysCurrentThreadId();
			if(!m_lock.Lockable()) {
				if(threadId == (uint32_t)m_threadId) {
					AtomicDecWaitDone();
					return;
				}
			}

			/* I'm no longer a reader */
			if (ClearThreadReader()) {
				AtomicDecReader();
			}

			/* Try lock */
			for(int k = 0; !m_lock.TryLock(); ++k) {
				cpu_relax(k);
			}

			/* Wait for all other readers to finish */
			for(int i = 0; m_readers; ++i) {
				cpu_relax(i);
			}

			atomic_xchg(&m_threadId, threadId);
			m_recursionCount = 1;
			AtomicDecWaitDone();
		}

		bool TimedUpgradeLockRead(uint32_t msec) throw() {
			if(!AtomicIncWaitDone()) {
				return false;
			}
			const uint32_t threadId = GetSysCurrentThreadId();
			if(!m_lock.Lockable()) {
				if(threadId == (uint32_t)m_threadId) {
					AtomicDecWaitDone();
					return true;
				}
			}

			/* I'm no longer a reader */
			if (ClearThreadReader()) {
				AtomicDecReader();
			}

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

			atomic_xchg(&m_threadId, threadId);
			m_recursionCount = 1;
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

			if(!IncThreadReader()) {
				/* Speculatively take read lock */
				AtomicIncReader();
			}

			/* Release write lock. */
			UnlockWrite();
			AtomicDecWaitDone();
			return true;
		}

		bool Using() throw() {
			return !m_lock.Lockable() || 0 != m_readers;
		}

	private:
		void UnlockWrite() throw() {
			if (!AtomicIncWaitDone()) {
				return;
			}
			if (m_lock.Lockable()) {
				AtomicDecWaitDone();
				return;
			}

			const uint32_t threadId = GetSysCurrentThreadId();
			if (threadId == (uint32_t)m_threadId) {
				if (m_recursionCount > 1) {
					--m_recursionCount;
					AtomicDecWaitDone();
					return;
				}
				else {
					m_recursionCount = 0;
					atomic_xchg(&m_threadId, 0);
				}
			}

			m_lock.Unlock();
			AtomicDecWaitDone();
		}

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

		bool IncThreadReader() {
			bool bSkipLock = false;
			uint64_t thdIndex = CThreadIndexManager::Pointer()->Get();
			if(m_thdContext.Has(thdIndex)) {
				int32_t readerCount = m_thdContext.Find(thdIndex) + 1;
				m_thdContext.Change(thdIndex, readerCount);
				if(readerCount > 1) {
					bSkipLock = true;
				}
			} else {
				m_thdContext.Add(thdIndex, 1);
			}

			return bSkipLock;
		}

		bool DecThreadReader() {
			bool bSkipUnlock = false;
			uint64_t thdIndex = CThreadIndexManager::Pointer()->Get();
			if(m_thdContext.Has(thdIndex)) {
				int32_t readerCount = m_thdContext.Find(thdIndex);
				if(readerCount > 0) {
					--readerCount;
					m_thdContext.Change(thdIndex, readerCount);
					if(readerCount > 0) {
						bSkipUnlock = true;
					}
				}
			}
			return bSkipUnlock;
		}

		bool ClearThreadReader() {
			bool bClear = false;
			uint64_t thdIndex = CThreadIndexManager::Pointer()->Get();
			if(m_thdContext.Has(thdIndex)) {
				int32_t readerCount = m_thdContext.Find(thdIndex);
				if(readerCount > 0) {
					m_thdContext.Change(thdIndex, 0);
					bClear = true;
				}
			}
			return bClear;
		}

		bool HasThreadReader() {
			bool hasLock = false;
			uint64_t thdIndex = CThreadIndexManager::Pointer()->Get();
			if(m_thdContext.Has(thdIndex)) {
				int32_t readerCount = m_thdContext.Find(thdIndex);
				if(readerCount > 0) {
					hasLock = true;
				}
			}
			return hasLock;
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

	private:
		CSpinLock m_lock;
		volatile uint32_t m_readers;
		volatile uint32_t m_threadId;
		volatile int32_t m_recursionCount;

		typedef util::CXqxTable1StoreS<int32_t> store_table_t;
		store_table_t m_thdContext;

		volatile int32_t m_waitDone;

	private:
		CSpinRecursiveRWLock(const CSpinRecursiveRWLock& orig) {}
		CSpinRecursiveRWLock& operator=(const CSpinRecursiveRWLock& right) { return *this; }
	};

}

#endif  // SPINRECURSIVERWLOCK_H
