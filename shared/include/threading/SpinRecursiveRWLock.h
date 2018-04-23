/*
 * File:   SpinRecursiveRWLock.h
 * Author: Jehu Shaw
 *
 * Created on 2013年10月1日, 上午10:32
 */

#ifndef __SPINRECURSIVERWLOCK_H_
#define	__SPINRECURSIVERWLOCK_H_

#include "SpinLock.h"
#include "IRWLock.h"
#include "SysCurrentThreadId.h"
#include "XqxTable2S.h"
#include "XqxTableIndexS.h"
#include "XqxTable1Store.h"

namespace thd {

#define MAX_READ_THREAD_SIZE 256

	class CSpinRecursiveRWLock : private CSpinLock, public IRWLock {
	public:
		CSpinRecursiveRWLock(uint32_t readThreadSize = MAX_READ_THREAD_SIZE)
			: CSpinLock()
			, m_readers(0)
			, m_threadId(0)
			, m_recursionCount(0)
			, m_waitDone(0)
		{
#if COMPILER == COMPILER_MICROSOFT
			BUILD_BUG_ON(sizeof(uint32_t) < sizeof(unsigned long));
#endif
			m_thisIdx = s_instcIdxs.Add();
			assert(XQXTABLEINDEXS_INDEX_NIL != m_thisIdx);
		}

        ~CSpinRecursiveRWLock() {
			if(!AtomicSetWaitDone()) {
				return;
			}
			// remove thread context.
			thread_table_t::iterator it(s_thdContext.Begin());
			for(; s_thdContext.End() != it; ++it) {
				thread_table_t::value_t itValue(it.GetValue());
				if(XQXTABLE2S_INDEX_NIL != itValue.nIndex && itValue.pObject) {
					store_table_t::value_t thdReaders(itValue.pObject->Find(m_thisIdx));
					if(XQXTABLE1STORE_INDEX_NIL != thdReaders.nIndex) {
						itValue.pObject->Remove(m_thisIdx);
					}
				}
			}
			s_instcIdxs.Remove(m_thisIdx);
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
			m_thisIdx = XQXTABLEINDEXS_INDEX_NIL;
			m_recursionCount = 0;
			atomic_xchg(&m_threadId, 0);
        }

		void LockWrite() throw() {
			if(!AtomicIncWaitDone()) {
				return;
			}
			const uint32_t threadId = GetSysCurrentThreadId();
			if(!Lockable()) {
				if(threadId == (uint32_t)m_threadId) {
					++m_recursionCount;
					AtomicDecWaitDone();
					return;
				}
			}

			/* Get lock */
			Lock();

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
			if(!Lockable()) {
				if(threadId == (uint32_t)m_threadId) {
					++m_recursionCount;
					AtomicDecWaitDone();
					return true;
				}
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

			atomic_xchg(&m_threadId, threadId);
			m_recursionCount = 1;
			AtomicDecWaitDone();
			return true;
		}

		void UnlockWrite() throw() {
			if(!AtomicIncWaitDone()) {
				return;
			}
			if(Lockable()) {
				AtomicDecWaitDone();
				return;
			}

			const uint32_t threadId = GetSysCurrentThreadId();
			if(threadId == (uint32_t)m_threadId) {
				if(m_recursionCount > 1) {
					--m_recursionCount;
					AtomicDecWaitDone();
					return;
				} else {
					m_recursionCount = 0;
					atomic_xchg(&m_threadId, 0);
				}
			}

			Unlock();
			AtomicDecWaitDone();
		}

		bool TryLockWrite() throw() {
			if(!AtomicIncWaitDone()) {
				return false;
			}
			const uint32_t threadId = GetSysCurrentThreadId();
			if(!Lockable()) {
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
			atomic_xchg(&m_threadId, threadId);
			m_recursionCount = 1;
			AtomicDecWaitDone();
			return true;
		}

		void LockRead() throw() {
			if(!AtomicIncWaitDone()) {
				return;
			}
			if(!Lockable()) {
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
			if(!Lockable()) {
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
			if(!Lockable()) {
				const uint32_t threadId = GetSysCurrentThreadId();
				if(threadId == (uint32_t)m_threadId) {
					AtomicDecWaitDone();
					return;
				}
			}

			if(DecThreadReader()) {
				AtomicDecWaitDone();
				return;
			}

			AtomicDecReader();
			AtomicDecWaitDone();
		}

		bool TryLockRead() throw() {
			if(!AtomicIncWaitDone()) {
				return false;
			}
			if(!Lockable()) {
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
			if(Lockable()) {
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
			if(!Lockable()) {
				if(threadId == (uint32_t)m_threadId) {
					AtomicDecWaitDone();
					return;
				}
			}

			/* Try lock */
			for(int k = 0; !this->TryLock(); ++k) {
				cpu_relax(k);
			}

			/* I'm no longer a reader */
			if(ClearThreadReader()) {
				AtomicDecReader();
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
			if(!Lockable()) {
				if(threadId == (uint32_t)m_threadId) {
					AtomicDecWaitDone();
					return true;
				}
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
			if(ClearThreadReader()) {
				AtomicDecReader();
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
			if(Lockable()) {
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

		static size_t& GetThreadIndex()  {
#if COMPILER == COMPILER_MICROSOFT
			__declspec(thread) static size_t s_thdIndex = XQXTABLE2S_INDEX_NIL;
#elif COMPILER == COMPILER_GNU
			__thread static size_t s_thdIndex = XQXTABLE2S_INDEX_NIL;
#elif COMPILER == COMPILER_BORLAND
			static size_t __thread s_thdIndex = XQXTABLE2S_INDEX_NIL;
#else
#error "not support";
#endif
			return s_thdIndex;
		}

		bool IncThreadReader() {
			bool bSkipLock = false;
			size_t& thdIndex = GetThreadIndex();
			if(XQXTABLE2S_INDEX_NIL == thdIndex) {
				store_table_t tmp;
				int32_t thdReaders = 1;
				tmp.Add(m_thisIdx, thdReaders);
				thdIndex = s_thdContext.Add(tmp);
			} else {
				store_table_t* pStore(s_thdContext.Find(thdIndex).pObject);
				if(pStore) {
					store_table_t::value_t thdReaders(pStore->Find(m_thisIdx));
					if(XQXTABLE1STORE_INDEX_NIL == thdReaders.nIndex) {
						thdReaders.object = 1;
						assert(pStore->Add(m_thisIdx, thdReaders.object));
					} else {
						pStore->Change(m_thisIdx, ++thdReaders.object);
						if(thdReaders.object > 1) {
							bSkipLock = true;
						}
					}
				} else {
					assert(pStore);
				}
			}
			return bSkipLock;
		}

		bool DecThreadReader() {
			bool bSkipUnlock = false;
			size_t& thdIndex = GetThreadIndex();
			if(XQXTABLE2S_INDEX_NIL != thdIndex) {
				store_table_t* pStore(s_thdContext.Find(thdIndex).pObject);
				if(pStore) {
					store_table_t::value_t thdReaders(pStore->Find(m_thisIdx));
					if(XQXTABLE1STORE_INDEX_NIL != thdReaders.nIndex && thdReaders.object > 0) {
						pStore->Change(m_thisIdx, --thdReaders.object);
						if(thdReaders.object > 0) {
							bSkipUnlock = true;
						}
					}
				} else {
					assert(pStore);
				}
			}
			return bSkipUnlock;
		}

		bool ClearThreadReader() {
			bool bClear = false;
			size_t& thdIndex = GetThreadIndex();
			if(XQXTABLE2S_INDEX_NIL != thdIndex) {
				store_table_t* pStore(s_thdContext.Find(thdIndex).pObject);
				if(pStore) {
					store_table_t::value_t thdReaders(pStore->Find(m_thisIdx));
					if(XQXTABLE1STORE_INDEX_NIL != thdReaders.nIndex && thdReaders.object > 0) {
						pStore->Change(m_thisIdx, 0);
						bClear = true;
					}
				} else {
					assert(pStore);
				}
			}
			return bClear;
		}

		bool HasThreadReader() {
			bool hasLock = false;
			size_t& thdIndex = GetThreadIndex();
			if(XQXTABLE2S_INDEX_NIL != thdIndex) {
				store_table_t* pStore(s_thdContext.Find(thdIndex).pObject);
				if(pStore) {
					store_table_t::value_t thdReaders(pStore->Find(m_thisIdx));
					if(XQXTABLE1STORE_INDEX_NIL != thdReaders.nIndex && thdReaders.object > 0) {
						hasLock = true;
					}
				} else {
					assert(pStore);
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
		volatile uint32_t m_threadId;
		volatile int32_t m_recursionCount;
		size_t m_thisIdx;
		typedef util::CXqxTable1Store<int32_t> store_table_t;
		typedef util::CXqxTable2S<store_table_t> thread_table_t;
		typedef util::CXqxTableIndexS instc_table_t;
		SHARED_DLL_DECL static thread_table_t s_thdContext;
		SHARED_DLL_DECL static instc_table_t s_instcIdxs;
		volatile int32_t m_waitDone;

	private:
		CSpinRecursiveRWLock(const CSpinRecursiveRWLock& orig) {}
		CSpinRecursiveRWLock& operator=(const CSpinRecursiveRWLock& right) { return *this; }

	};

}

#endif  // __SPINRECURSIVERWLOCK_H_
