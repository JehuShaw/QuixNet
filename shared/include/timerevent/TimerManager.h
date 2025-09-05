/* 
 * File:   TimerManager.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_17, 9:45
 */

#ifndef TIMERMANAGER_H
#define	TIMERMANAGER_H

#include "Common.h"
#include "ThreadBase.h"
#include "TimerEvent.h"
#include "SpinLock.h"
#include "Singleton.h"

namespace evt
{
	const unsigned int MININUM_TIMEOUT_QUEUE_SIZE = 32;

    class CTimerManager 
		: private CTimerEvent
		, public util::Singleton<CTimerManager> 
	{
    public:
		CTimerManager() {}

		~CTimerManager() {}

		bool SetTimeout(id64_t id, unsigned int delay,
			util::CAutoPointer<util::CallbackBase> method,
			bool bWaitResult = false, bool bUniqueId = true, bool bWorkerThread = true,
			uint64_t threadOrder = 0)
		{
			return CTimerEvent::SetTimeout(id, delay, method, bWaitResult, bUniqueId, bWorkerThread, threadOrder);
		}

		bool SetTimeoutOrder(id64_t id, unsigned int delay,
			util::CAutoPointer<util::CallbackBase> method,
			uint64_t threadOrder = 0)
		{
			return CTimerEvent::SetTimeout(id, delay, method, false, true, true, threadOrder);
		}

		bool SetInterval(id64_t id, unsigned int interval,
			util::CAutoPointer<util::CallbackBase> method, unsigned int delay = 0,
			bool bWaitResult = false, bool bUniqueId = true, bool bWorkerThread = true,
			uint64_t threadOrder = 0)
		{
			return CTimerEvent::SetInterval(id, interval, method, delay, bWaitResult, bUniqueId, bWorkerThread, threadOrder);
		}

		bool SetIntervalOrder(id64_t id, unsigned int interval,
			util::CAutoPointer<util::CallbackBase> method, unsigned int delay = 0,
			uint64_t threadOrder = 0)
		{
			return CTimerEvent::SetInterval(id, interval, method, delay, false, true, true, threadOrder);
		}

		bool SetAtTime(id64_t id, const AtTime& atTime,
			util::CAutoPointer<util::CallbackBase> method, bool bWaitResult = false,
			bool bUniqueId = true, bool bWorkerThread = true, uint64_t threadOrder = 0)
		{
			return CTimerEvent::SetAtTime(id, atTime, method, bWaitResult, bUniqueId, bWorkerThread, threadOrder);
		}

		bool SetAtTimeOrder(id64_t id, const AtTime& atTime,
			util::CAutoPointer<util::CallbackBase> method,
			uint64_t threadOrder = 0)
		{
			return CTimerEvent::SetAtTime(id, atTime, method, false, true, true, threadOrder);
		}

		util::CAutoPointer<CTimer> GetTimer(id64_t id) {
			return CTimerEvent::GetTimer(id, util::CAutoPointer<util::CallbackBase>());
		}

		util::CAutoPointer<CTimer> GetTimer(id64_t id, util::CAutoPointer<util::CallbackBase> method) {
			return CTimerEvent::GetTimer(id, method);
		}

		bool Modify(id64_t id, eTimerOperater operater, timer_data_t data = 0, bool bWaitResult = false) {
			return CTimerEvent::Modify(id, operater, data, bWaitResult);
		}

		bool Remove(id64_t id, bool bWaitResult = false) {
			return CTimerEvent::Remove(id, bWaitResult);
		}

		inline void Loop() {
			CTimerEvent::Loop();
		}

	private:
		class CCallbackArgument : public thd::ThreadBase, public util::PoolBase<CCallbackArgument> {
		public:
			CCallbackArgument(util::CAutoPointer<util::CallbackBase>& pMethod)

				: m_pMethod(pMethod)
			{}

			virtual bool OnRun() {
				m_pMethod->Invoke();
				return true;
			}

			virtual void OnShutdown() {}

		private:
			util::CAutoPointer<util::CallbackBase> m_pMethod;
		};

		virtual void OnTimeout(
			id64_t id,
			util::CAutoPointer<util::CallbackBase>& pMethod,
			bool bWorkerThread)
		{
			if (bWorkerThread) {
				thd::ThreadPool.ExecuteTask(new CCallbackArgument(pMethod));
			} else {
				pMethod->Invoke();
			}
		}
    };
    
}
#endif	/* TIMERMANAGER_H */

