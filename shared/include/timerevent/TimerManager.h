/* 
 * File:   TimerManager.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_17, 9:45
 */

#ifndef _TIMERMANAGER_H
#define	_TIMERMANAGER_H

#include "Common.h"
#include "CThreads.h"
#include "TimerEvent.h"
#include "SpinLock.h"
#include "Singleton.h"

namespace evt
{
	const unsigned int MININUM_TIMEOUT_QUEUE_SIZE = 32;

    class SHARED_DLL_DECL CTimerManager 
		: private thd::CThread
		, private CTimerEvent
		, public util::Singleton<CTimerManager> 
	{
    public:
		CTimerManager() : outgoingMessages(MININUM_TIMEOUT_QUEUE_SIZE), bExit(false) {}

		~CTimerManager() {}

		bool SetTimeout(id64_t id, unsigned int delay,
			util::CAutoPointer<util::CallbackBase> method, bool bWaitResult = false,
			bool bUniqueId = true, bool bWorkerThread = true) {
				return CTimerEvent::SetTimeout(id, delay, method, bWaitResult, bUniqueId, bWorkerThread);
		}

		bool SetInterval(id64_t id, unsigned int interval,
			util::CAutoPointer<util::CallbackBase> method, unsigned int delay = 0,
			bool bWaitResult = false, bool bUniqueId = true, bool bWorkerThread = true) {
				return CTimerEvent::SetInterval(id, interval, method, delay, bWaitResult, bUniqueId, bWorkerThread);
		}

		bool SetAtTime(id64_t id, const AtTime& atTime,
			util::CAutoPointer<util::CallbackBase> method, bool bWaitResult = false,
			bool bUniqueId = true, bool bWorkerThread = true) {
				return CTimerEvent::SetAtTime(id, atTime, method, bWaitResult, bUniqueId, bWorkerThread);
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

		void Loop() {
			CTimerEvent::Loop();
		}

	private:
		virtual bool Run();

		virtual void OnShutdown();

		virtual void OnTimeout(bool bWorkerThread, id64_t id, util::CAutoPointer<util::CallbackBase>& method);

		void SetCallback(const util::CAutoPointer<util::CallbackBase>& method) {
			util::CAutoPointer<util::CallbackBase>* pPt = outgoingMessages.WriteLock();
			*pPt = method;
			outgoingMessages.WriteUnlock();
		}

		bool GetCallback(util::CAutoPointer<util::CallbackBase>& outMethod) {
			util::CAutoPointer<util::CallbackBase>* pPt = outgoingMessages.ReadLock();
			if(NULL != pPt) {
				outMethod = *pPt;
				outgoingMessages.ReadUnlock();
				return true;
			}
			return false;
		}

	private:
		thd::CCircleQueue<util::CAutoPointer<util::CallbackBase> > outgoingMessages;

		volatile bool bExit;
    };
    
}
#endif	/* _TIMERMANAGER_H */

