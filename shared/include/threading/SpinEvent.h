/*
 * File:   SpinEvent.h
 * Author: Jehu Shaw
 *
 * Created on 2013年10月1日, 上午10:32
 */

#ifndef SPINEVENT_H
#define	SPINEVENT_H

#include "Common.h"
#include "AtomicLock.h"
#include "SysTickCount.h"

namespace thd {


	class CSpinEvent {
	public:
		CSpinEvent(void) : m_flag(FALSE) {}
		~CSpinEvent(void) { Resume(); }

		void Wait() {
			// wait
			for(int i = 0; TRUE == m_flag; ++i) {
				cpu_relax(i);
			}
		}

		bool Wait(uint32_t msec) {
			// wait
			uint64_t startTime = GetSysTickCount();
			for(int i = 0; TRUE == m_flag; ++i) {
				cpu_relax(i);
				if((int64_t)(GetSysTickCount() - 
					startTime) >= (int64_t)msec)
				{
					return false;
				}
			}
			return true;
		}

		void Suspend() {
			atomic_xchg8(&m_flag, TRUE);
		}

		void Resume() {
			atomic_xchg8(&m_flag, FALSE);
		}

	private:
		volatile char m_flag;
	};

}

#endif  // SPINEVENT_H
