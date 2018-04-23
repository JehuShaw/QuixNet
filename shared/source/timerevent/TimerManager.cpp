/* 
 * File:   TimerManager.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2010_9_17, 9:45
 */

#include "TimerManager.h"

using namespace thd;
using namespace util;


namespace evt {

/**
 * defined TimerManager
 */

	void CTimerManager::OnShutdown() {
		atomic_xchg8(&bExit, true);
	}

	void CTimerManager::OnTimeout(bool bWorkerThread, id64_t id, CAutoPointer<CallbackBase>& method)
	{
		if(bWorkerThread) {
			if(bExit) {
				return;
			}
			SetCallback(method);

			ThreadPool.ExecuteTask(this);
		} else {
			method->Invoke();
		}
	}

	bool CTimerManager::Run() {

		int nSize = outgoingMessages.Size();
		for(int i = 0; i < nSize && !bExit; ++i) {
			CAutoPointer<CallbackBase> pMethod;
			if(GetCallback(pMethod)) {
				pMethod->Invoke();
				continue;
			}
			break;
		}
		return false;
	}

} // end namespace evt

