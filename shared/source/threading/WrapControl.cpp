/* 
 * File:   WrapControl.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */


#include "WrapControl.h"
#include "ThreadPool.h"




namespace thd {

	CWrapControl::CWrapControl()
		: m_threadCount(0),
		m_writeCount(0),
		m_readCount(0),
		m_bWrite(false),
		m_rwMutex(),
		m_status(CONTROL_RUNNING),
		m_waitPause(false)
	{
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(int) < sizeof(long));
#endif
	}

	void CWrapControl::InnerTrigger() const
	{
		int count = atomic_inc(&m_threadCount);
		do {
			if (count == KEEP_THREAD_COUNT) {
				ThreadPool.ExecuteTask(const_cast<CWrapControl*>(this));
				return;
			}
			if (atomic_cmpxchg(&m_threadCount, count, count - 1) == count) {
				break;
			}
			count = m_threadCount;
		} while(true);
	}

	eSwitchStateResult CWrapControl::SwitchState(int count) const
	{
		do {
			if (count != EXIT_THREAD_COUNT) {
				return SWITCH_STATE_EXIT;
			}
			if (RemainingSize() <= 0 && m_status != CONTROL_PAUSING) {
				return SWITCH_STATE_EXIT;
			}
			if (atomic_cmpxchg(&m_threadCount, count, count + 1) == count) {
				break;
			}
			count = m_threadCount;
		} while(true);
		if (m_status == CONTROL_PAUSING) {
			return SWITCH_STATE_PAUSE;
		}
		ThreadPool.ExecuteTask(const_cast<CWrapControl*>(this));
		return SWITCH_STATE_KEEP;
	}

	bool CWrapControl::OnRun()
	{
		if (m_status == CONTROL_TERMINATE) {
			atomic_dec(&m_threadCount);
			return false;
		}
		
		if (!TryDirectAccess(true)) {
			eSwitchStateResult ret = SwitchState(atomic_dec(&m_threadCount));
			if (SWITCH_STATE_EXIT == ret) {
				return false;
			}
			if (SWITCH_STATE_PAUSE != ret) {
				Sleep(SLEEP_MSTIME);
				return false;
			}
		}
		
		if (m_status == CONTROL_RUNNING) {
			
			OnRouite();
			
			DirectAccessDone();
			int count = atomic_dec(&m_threadCount);
			
			if (m_status == CONTROL_TERMINATE) {
				return false;
			}
			
			eSwitchStateResult ret = SwitchState(count);
			if (SWITCH_STATE_EXIT == ret) {
				return false;
			}
			if (SWITCH_STATE_PAUSE != ret) {
				Sleep(SLEEP_MSTIME);
				return false;
			}
		}
		
		if (m_status == CONTROL_TERMINATE) {
			DirectAccessDone();
			atomic_dec(&m_threadCount);
			return false;
		}

		if (atomic_cmpxchg8(&m_status, CONTROL_PAUSING, CONTROL_PAUSE) == CONTROL_PAUSING) {
			DirectAccessDone();
			atomic_dec(&m_threadCount);
			return false;
		}

		if (m_waitPause) {
			OnPause();
			atomic_dec(&m_threadCount);
		} else {
			atomic_dec(&m_threadCount);
			OnPause();
		}
		
		DirectAccessDone();
		return false;
	}

	eWaitingResult CWrapControl::Waitting(volatile const bool& flag, const CWrapControl* receiver, bool readOnly) const
	{
		while (flag) {

			const_cast<CWrapControl*>(this)->OnEmergency();

			if (receiver && receiver != this && receiver->TryDirectAccess(readOnly)) {
				return WAITING_DIRECT_ACCESS;
			}

			bool saveReadOnly = DirectAccessDone();

			Sleep(WAITING_SLEEP_MSTIME);

			for (int i = 0; !TryDirectAccess(saveReadOnly); ++i) {
				cpu_relax(i);
			}
		}
		return WAITING_SUCCESS;
	}

	void CWrapControl::WaitExit() const
	{
		for (int i = 0; !CheckIdle(); ++i) {
			cpu_relax(i);
		}
	}

}