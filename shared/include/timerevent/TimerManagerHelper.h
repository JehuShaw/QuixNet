/* 
 * File:   TimerManagerHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_17, 9:45
 */

#ifndef TIMERMANAGERHELPER_H
#define	TIMERMANAGERHELPER_H

#include "TimerManager.h"
#include "GuidFactory.h"
#include "CallBack.h"

using namespace util;

namespace evt
{
	FORCEINLINE uint64_t SetTimeout(
		unsigned int delay,
		CAutoPointer<CallbackBase> method,
		bool bWaitResult = false,
		bool bUniqueId = true,
		bool bWorkerThread = true)
	{
		uint64_t id = CGuidFactory::Pointer()->CreateGuid();

		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		if(pTMgr->SetTimeout(id, delay, method,
			bWaitResult, bUniqueId, bWorkerThread))
		{
			return id;
		}
		return 0;
	}

	FORCEINLINE bool SetTimeoutEx(
		id64_t id,
		unsigned int delay,
		CAutoPointer<CallbackBase> method,
		bool bWaitResult = false,
		bool bUniqueId = true,
		bool bWorkerThread = true)
	{
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		return pTMgr->SetTimeout(id, delay, method,
			bWaitResult, bUniqueId, bWorkerThread);
	}

	FORCEINLINE uint64_t SetInterval(
		unsigned int interval,
		CAutoPointer<CallbackBase> method,
		unsigned int delay = 0,
		bool bWaitResult = false,
		bool bUniqueId = true,
		bool bWorkerThread = true)
	{
		uint64_t id = CGuidFactory::Pointer()->CreateGuid();

		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		if(pTMgr->SetInterval(id, interval, method, delay,
			bWaitResult, bUniqueId, bWorkerThread))
		{
			return id;
		}
		return 0;
	}

	FORCEINLINE bool SetIntervalEx(
		id64_t id,
		unsigned int interval,
		CAutoPointer<CallbackBase> method,
		unsigned int delay = 0,
		bool bWaitResult = false,
		bool bUniqueId = true,
		bool bWorkerThread = true)
	{
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		return pTMgr->SetInterval(id, interval, method, delay,	
			bWaitResult, bUniqueId, bWorkerThread);
	}

	FORCEINLINE uint64_t SetAtTime(
		const AtTime& atTime,
		CAutoPointer<CallbackBase> method,
		bool bWaitResult = false,
		bool bUniqueId = true,
		bool bWorkerThread = true)
	{
		uint64_t id = CGuidFactory::Pointer()->CreateGuid();

		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		if(pTMgr->SetAtTime(id, atTime, method,
			bWaitResult, bUniqueId, bWorkerThread))
		{
			return id;
		}
		return 0;
	}

	FORCEINLINE bool SetAtTimeEx(
		id64_t id,
		const AtTime& atTime,
		CAutoPointer<CallbackBase> method,
		bool bWaitResult = false,
		bool bUniqueId = true,
		bool bWorkerThread = true)
	{
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		return pTMgr->SetAtTime(id, atTime, method,
			bWaitResult, bUniqueId, bWorkerThread);
	}

	FORCEINLINE CAutoPointer<CTimer> GetTimer(id64_t id)
	{
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		return pTMgr->GetTimer(id, CAutoPointer<CallbackBase>());
	}

	FORCEINLINE CAutoPointer<CTimer> GetTimer(
		id64_t id,
		CAutoPointer<CallbackBase> method)
	{
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		return pTMgr->GetTimer(id, method);
	}

	FORCEINLINE bool Modify(
		id64_t id,
		eTimerOperater operater,
		timer_data_t data = 0,
		bool bWaitResult = false)
	{
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		return pTMgr->Modify(id, operater, data, bWaitResult);
	}

	FORCEINLINE bool Remove(id64_t id, bool bWaitResult = false)
	{
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		return pTMgr->Remove(id, bWaitResult);
	}
}
#endif	/* TIMERMANAGERHELPER_H */

