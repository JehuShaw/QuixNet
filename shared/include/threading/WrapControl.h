/* 
 * File:   WrapControl.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef WRAP_CONTROL_H
#define	WRAP_CONTROL_H

#include <stdint.h>
#include "ShareDll.h"
#include "ScopedLock.h"
#include "SpinLock.h"
#include "ThreadBase.h"

namespace thd {

#define EXIT_THREAD_COUNT 0
#define KEEP_THREAD_COUNT 1
#define SLEEP_MSTIME 5
#define WAITING_SLEEP_MSTIME 5

	enum eWaitingResult {
		WAITING_FAIL,
		WAITING_SUCCESS,
		WAITING_EXIT,
		WAITING_DIRECT_ACCESS,
		WAITING_TIMEOUT,
	};

	enum eControlStatus : int8_t {
		CONTROL_TERMINATE,
		CONTROL_RUNNING,
		CONTROL_PAUSING,
		CONTROL_PAUSE,
	};

	enum eSwitchStateResult : int8_t {
		SWITCH_STATE_EXIT,
		SWITCH_STATE_PAUSE,
		SWITCH_STATE_KEEP,
	};

	class SHARED_DLL_DECL CWrapControl : private ThreadBase
	{
	public:
		CWrapControl();

		virtual ~CWrapControl() {
			atomic_xchg8(&m_status, CONTROL_TERMINATE);
			WaitExit();
		}

		CWrapControl(const CWrapControl& orig) 
			: m_threadCount(0),
			m_writeCount(0),
			m_readCount(0),
			m_bWrite(false),
			m_rwMutex(),
			m_status(CONTROL_RUNNING),
			m_waitPause(false) {}

		CWrapControl& operator=(const CWrapControl& right) { return *this; }

		void Dispose() {
			atomic_xchg8(&m_status, CONTROL_TERMINATE);
		}
		
		void Start() {
			if (atomic_cmpxchg8(&m_status, CONTROL_PAUSE, CONTROL_RUNNING) == CONTROL_PAUSE) {
				if (RemainingSize() > 0) {
					InnerTrigger();
				}
			}
		}
		
		void Stop(bool waitPause = false) {
			atomic_xchg8(&m_waitPause, waitPause);
			if (atomic_cmpxchg8(&m_status, CONTROL_RUNNING, CONTROL_PAUSING) == CONTROL_RUNNING) {
				InnerTrigger();
				WaitExit();
			}
		}
		
		inline void Trigger() const {
			if (m_status != CONTROL_RUNNING) {
				return;
			}
			InnerTrigger();
		}

	protected:
		void InnerTrigger() const;

		inline void CheckSizeAndTrigger() const {
			if (RemainingSize() <= 0) {
				return;
			}
			InnerTrigger();
		}

		virtual void OnEmergency() = 0;

		virtual void OnRouite() = 0;
		
		virtual void OnPause() = 0;

		virtual int RemainingSize() const = 0;

		eWaitingResult Waitting(volatile const bool& flag, const CWrapControl* receiver , bool readOnly) const;

		volatile const int8_t& GetStatus() const {
			return m_status;
		}

		bool TryDirectAccess(bool readOnly) const {
			thd::CScopedLock lock(m_rwMutex);
			if (m_writeCount > EXIT_THREAD_COUNT) {
				return false;
			}
			if (readOnly) {
				++m_readCount;
			} else if (m_readCount > EXIT_THREAD_COUNT) {
				return false;
			} else {
				m_bWrite = true;
				++m_writeCount;
			}
			return true;
		}

		bool DirectAccessDone() const {
			thd::CScopedLock lock(m_rwMutex);
			if (m_bWrite && m_writeCount > EXIT_THREAD_COUNT) {
				m_bWrite = false;
				--m_writeCount;
				return false;
			} else if (m_readCount > EXIT_THREAD_COUNT) {
				--m_readCount;
			}
			return true;
		}

		void WaitUpgradeRead() const throw() {
			m_rwMutex.Lock();
			++m_writeCount;
			int i = 0;
			do {
				if (m_writeCount == KEEP_THREAD_COUNT 
					&& (m_readCount == KEEP_THREAD_COUNT 
					|| m_readCount == EXIT_THREAD_COUNT))
				{
					if (m_readCount == KEEP_THREAD_COUNT) {
						--m_readCount;
					}
					m_bWrite = true;
					m_rwMutex.Unlock();
					return;
				}
				m_rwMutex.Unlock();
				cpu_relax(i++);
				m_rwMutex.Lock();
			} while (true);
		}

		bool OnlyReadWaitUpgrade() const throw() {
			m_rwMutex.Lock();
			if (m_writeCount > EXIT_THREAD_COUNT) {
				m_rwMutex.Unlock();
				return false;
			}
			++m_writeCount;
			int i = 0;
			do {
				if (m_readCount == KEEP_THREAD_COUNT 
					|| m_readCount == EXIT_THREAD_COUNT)
				{
					if (m_readCount == KEEP_THREAD_COUNT) {
						--m_readCount;
					}
					m_bWrite = true;
					m_rwMutex.Unlock();
					break;
				}
				m_rwMutex.Unlock();
				cpu_relax(i++);
				m_rwMutex.Lock();
			} while (true);
			return true;
		}

		void WaitDegradeWrite() const throw() {
			int i = 0;
			do {
				m_rwMutex.Lock();
				if (m_writeCount <= KEEP_THREAD_COUNT) {
					if (m_writeCount == KEEP_THREAD_COUNT) {
						--m_writeCount;
						m_bWrite = false;
					}
					++m_readCount;
					m_rwMutex.Unlock();
					return;
				}
				m_rwMutex.Unlock();
				cpu_relax(i++);
			} while (true);
		}

	private:

		eSwitchStateResult SwitchState(int count) const;

		virtual void OnShutdown() {}

		virtual bool OnRun() override;

		int GetRunningCount() const {
			thd::CScopedLock lock(m_rwMutex);
			return m_writeCount;
		}

		bool CheckIdle() const {
			thd::CScopedLock lock(m_rwMutex);
			return m_writeCount <= EXIT_THREAD_COUNT && m_readCount <= EXIT_THREAD_COUNT;
		}

		void WaitExit() const;

	private:
		mutable volatile int m_threadCount;
		mutable int m_writeCount;
		mutable int m_readCount;
		mutable bool m_bWrite;
		mutable thd::CSpinLock m_rwMutex;
		volatile int8_t m_status;
		volatile bool m_waitPause;
	};
}

#endif /* WRAP_CONTROL_H */
