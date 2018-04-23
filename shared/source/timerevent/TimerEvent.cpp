/*
 * File:   TimerEvent.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_17, 9:45
 */

#include "TimerEvent.h"
#include <assert.h>
#include <stdio.h>
#include <stdexcept>
#include <time.h>
#include <math.h>
#include "TimestampManager.h"
#include "SysTickCount.h"
#include "SysCurrentThreadId.h"

using namespace util;

namespace evt {

//#ifdef _WIN32
//UINT_PTR ET::CTimerEvent::iTimerID(NULL);
//#else
//struct itimerval ET::CTimerEvent::ovalue;
//#endif


/**
 * defined TimeKey
 */
uint64_t CTimeKey::lastAdd(0);

//uint32_t volatile CTimerEvent::counter(0);
//////////////////////////////////////////////////////////////////////////
struct MyDate
{
	int year;
	int month;
	int day;
};

int GetAbsDays(const MyDate& x)
{
	int month_day[] = {31,28,31,30,31,30,31,31,30,31,30,31};
	int year = x.year-1;
	int days = year * 365 + year/4 - year/100 + year/400;
	if(x.year%4==0 && x.year%100!=0 || x.year%400==0) month_day[1]++;
	for(int i=0; i < x.month-1; ++i) {
		days += month_day[i];
	}
	days += x.day-1;
	return days;
}

inline int GetDiffDays(const MyDate& a, const MyDate& b)
{
	return GetAbsDays(b) - GetAbsDays(a);
}

inline static uint32_t AtTimeInterval(const sAtTime& atTime, bool bNext) {
    CTimestampManager::PTR_T pTsMgr(CTimestampManager::Pointer());
    tm tmNow = {0,0,0,0,0,0,0,0,0};
    pTsMgr->GetTM(&tmNow);

    return CTimerEvent::AtTimeIntervalFrom(atTime, tmNow, bNext);
}

//////////////////////////////////////////////////////////////////////////

bool CTimeKey::operator==( const CTimeKey& right ) const{
    if((uint64_t)this == (uint64_t)&right){
        return true;
    }
    return false;
}

bool CTimeKey::operator!=( const CTimeKey& right ) const{
    if((uint64_t)this != (uint64_t)&right){
        return true;
    }
    return false;
}

bool CTimeKey::operator > ( const CTimeKey& right ) const{
	if((uint64_t)this == (uint64_t)&right){
		return false;
	}
    int64_t ll = delay - (lastAdd - playtime);
    int64_t rl = right.delay - (lastAdd - right.playtime);
    if(ll > rl){
        return true;
    }else if(ll == rl){
        if((uint64_t)this > (uint64_t)&right){
            return true;
        }
    }
    return false;
}

bool CTimeKey::operator < ( const CTimeKey& right ) const{
	if((uint64_t)this == (uint64_t)&right){
		return false;
	}
    int64_t ll = delay - (lastAdd - playtime);
    int64_t rl = right.delay - (lastAdd - right.playtime);
    if(ll < rl){
        return true;
    }else if(ll == rl){
        if((uint64_t)this < (uint64_t)&right){
            return true;
        }
    }
    return false;
}

/**
 * defined CTimer
 */

int CTimer::GetTimeLeft() const {
	int nTimeLeft = (int)((key.playtime + key.delay) - CTimerEvent::GetTick());
	if(nTimeLeft < 0) {
		nTimeLeft = 0;
	}
	return nTimeLeft;
}

/**
 * defined TimerManager
 */

CTimerEvent::CTimerEvent() : loopThreadId(0), bDispose(false) {
	//startTick();
}

CTimerEvent::~CTimerEvent() {
	//stopTick();
	atomic_xchg8(&bDispose, true);
}

bool CTimerEvent::SetTimeout(id64_t id, unsigned int delay,
	CAutoPointer<CallbackBase> method, bool bWaitResult/* = false*/,
    bool bUniqueId/* = true*/, bool bWorkerThread/* = true*/) {

    if(bDispose) {
        return false;
    }

	if(bWaitResult && loopThreadId == GetSysCurrentThreadId()) {
		CTimer timer;
		timer.id.u64 = id.u64;
		timer.utype = ELAPSE_TYPE_DELAY;
		timer.key.delay = delay;
		timer.key.playtime = GetTick();
		timer.bUnique = bUniqueId;
		timer.method = method;
		timer.workerThread = bWorkerThread;
		return InnerAddTimer(timer);
	} else {
		CTimer* pT = incomingMessages.WriteLock();
		pT->id.u64 = id.u64;
		pT->utype = ELAPSE_TYPE_DELAY;
		pT->key.delay = delay;
		pT->key.playtime = GetTick();
		pT->bUnique = bUniqueId;
		pT->method = method;
		pT->workerThread = bWorkerThread;
		pT->result = false;
		pT->spinEvent.Suspend();
		incomingMessages.WriteUnlock();

		if(bWaitResult) {
			pT->spinEvent.Wait(TIMER_WAIT_RESULT_TIMEOUT_SECOND);
			return pT->result;
		}
	}
	return true;
}

bool CTimerEvent::SetInterval(id64_t id, unsigned int interval,
	CAutoPointer<CallbackBase> method, unsigned int delay/* = 0*/,
	bool bWaitResult/* = false*/, bool bUniqueId/* = true*/,
	bool bWorkerThread/* = true*/) {

    if(bDispose) {
        return false;
    }

	if(0 == interval) {
		interval = 1;
	}

	if(0 == delay) {
		delay = interval;
	}

	if(bWaitResult && loopThreadId == GetSysCurrentThreadId()) {
		CTimer timer;
		timer.id.u64 = id.u64;
		timer.utype = ELAPSE_TYPE_INTERVAL;
		timer.key.delay = delay;
		timer.key.data.interval = interval;
		timer.key.playtime = GetTick();
		timer.bUnique = bUniqueId;
		timer.method = method;
		timer.workerThread = bWorkerThread;
		return InnerAddTimer(timer);
	} else {
		CTimer* pT = incomingMessages.WriteLock();
		pT->id.u64 = id.u64;
		pT->utype = ELAPSE_TYPE_INTERVAL;
		pT->key.delay = delay;
		pT->key.data.interval = interval;
		pT->key.playtime = GetTick();
		pT->bUnique = bUniqueId;
		pT->method = method;
		pT->workerThread = bWorkerThread;
		pT->result = false;
		pT->spinEvent.Suspend();
		incomingMessages.WriteUnlock();

		if(bWaitResult) {
			pT->spinEvent.Wait(TIMER_WAIT_RESULT_TIMEOUT_SECOND);
			return pT->result;
		}
	}
	return true;
}

bool CTimerEvent::SetAtTime(id64_t id, const sAtTime& atTime,
	CAutoPointer<CallbackBase> method, bool bWaitResult/* = false*/,
    bool bUniqueId/* = true*/, bool bWorkerThread/* = true*/) {

    if(bDispose) {
        return false;
    }

	uint64_t startTick = GetTick();
	unsigned int interval = AtTimeInterval(atTime, false);
	if(0 == interval) {
		interval = 1;
	}

	if(bWaitResult && loopThreadId == GetSysCurrentThreadId()) {
		CTimer timer;
		timer.id.u64 = id.u64;
		timer.utype = ELAPSE_TYPE_ATTIME;
		timer.key.delay = interval;
		timer.key.data.attime = atTime;
		timer.key.playtime = startTick;
		timer.bUnique = bUniqueId;
		timer.method = method;
		timer.workerThread = bWorkerThread;
		return InnerAddTimer(timer);
	} else {
		CTimer* pT = incomingMessages.WriteLock();
		pT->id.u64 = id.u64;
		pT->utype = ELAPSE_TYPE_ATTIME;
		pT->key.delay = interval;
		pT->key.data.attime = atTime;
		pT->key.playtime = startTick;
		pT->bUnique = bUniqueId;
		pT->method = method;
		pT->workerThread = bWorkerThread;
		pT->result = false;
		pT->spinEvent.Suspend();
		incomingMessages.WriteUnlock();

		if(bWaitResult) {
			pT->spinEvent.Wait(TIMER_WAIT_RESULT_TIMEOUT_SECOND);
			return pT->result;
		}
	}
	return true;
}

CAutoPointer<CTimer> CTimerEvent::GetTimer(id64_t id, util::CAutoPointer<util::CallbackBase> method) {

	if(bDispose) {
		return CAutoPointer<CTimer>();
	}

	if(loopThreadId == GetSysCurrentThreadId()) {
		CAutoPointer<CTimer> pTimer(InnerFindTimer(id, method));
		if(!pTimer.IsInvalid()) {
			CAutoPointer<CTimer> pNewTimer(new CTimer(*pTimer));
			return pNewTimer;
		}
	} else if(0 != loopThreadId) {
		CTimer* pT = incomingMessages.WriteLock();
		pT->id.u64 = id.u64;
		pT->utype = ELAPSE_TYPE_GETTIMER;
		pT->method = method;
		pT->result = false;
		pT->spinEvent.Suspend();
		incomingMessages.WriteUnlock();

		pT->spinEvent.Wait(TIMER_WAIT_RESULT_TIMEOUT_SECOND);
		if(pT->result) {
			CAutoPointer<CTimer> pNewTimer(new CTimer(*pT));
			return pNewTimer;
		}
	}
	return CAutoPointer<CTimer>();
}

bool CTimerEvent::Modify(id64_t id, eTimerOperater operater, timer_data_t data/* = 0*/, bool bWaitResult/* = false*/) {

    if(bDispose) {
        return false;
    }

	if(bWaitResult && loopThreadId == GetSysCurrentThreadId()) {
		return InnerModifyTimer(id, (short)operater, data);
	} else {
		CTimeModify* pM = modifyMessages.WriteLock();
		pM->id.u64 = id.u64;
		pM->operater = (short)operater;
		pM->data = data;
		pM->result = false;
		pM->spinEvent.Suspend();
		modifyMessages.WriteUnlock();

		if(bWaitResult) {
			pM->spinEvent.Wait(TIMER_WAIT_RESULT_TIMEOUT_SECOND);
			return pM->result;
		}
	}
	return true;
}

bool CTimerEvent::Remove(id64_t id, bool bWaitResult/* = false*/) {

    if(bDispose) {
        return false;
    }

	if(bWaitResult && loopThreadId == GetSysCurrentThreadId()) {
		return InnerRemoveTimer(id);
	} else {
		CTimeDelete* pD = deleteMessages.WriteLock();
		pD->id = id.u64;
		pD->result = false;
		pD->spinEvent.Suspend();
		deleteMessages.WriteUnlock();

		if(bWaitResult) {
			pD->spinEvent.Wait(TIMER_WAIT_RESULT_TIMEOUT_SECOND);
			return pD->result;
		}
	}
	return true;
}

//#ifdef _WIN32
//VOID CALLBACK CTimerEvent::TimerProc (HWND hwnd,
//	UINT message, UINT iTimerID, DWORD dwTime) {
//		atomic_inc(&counter);
//}
//#else
//void CTimerEvent::sig_func(int signo)
//{
//	atomic_inc(&counter);
//	signal(signo,sig_func);
//}
//#endif

//bool CTimerEvent::startTick(){
//#ifdef _WIN32
//	UINT wMsecInterval = 100;
//	iTimerID = SetTimer(NULL, 0, wMsecInterval, &CTimerEvent::TimerProc);
//	if(NULL == iTimerID) {
//		return false;
//	}
//	return true;
//#else
//	//set timer
//	struct itimerval value;
//	value.it_interval.tv_sec = 0;
//	value.it_interval.tv_usec = 100000;
//	value.it_value.tv_sec = 0;
//	value.it_value.tv_usec = 100000;
//    if((old_sigfunc = signal(SIGALRM, sig_func)) != SIG_ERR){
//        if(setitimer(ITIMER_REAL,&value,&ovalue) == 0){
//            return true;
//        }
//    }
//	return false;
//#endif
//}
//
//bool CTimerEvent::stopTick(){
//#ifdef _WIN32
//	return KillTimer(NULL, iTimerID) != FALSE;
//#else
//	struct itimerval value;
//    if(signal(SIGALRM, old_sigfunc) != SIG_ERR){
//        if(setitimer(ITIMER_REAL,&ovalue,&value) == 0){
//            return true;
//        }
//    }
//    return false;
//#endif
//}

CAutoPointer<CTimer> CTimerEvent::InnerFindTimer(id64_t id, CAutoPointer<CallbackBase> method) {
	timer_map_t::iterator it = timerMap.find(id.u64);
	if(timerMap.end() != it) {
		if(method.IsInvalid()) {
			return it->second;
		} else {
			do{
				if(!it->second->method.IsInvalid()) {
					if(it->second->method->Equal(*method)) {
						return it->second;
					}
				}
				++it;
			} while(timerMap.end() != it && it->first == id.u64);
		}
	}
	return CAutoPointer<CTimer>();
}

bool CTimerEvent::InnerAddTimer(const CTimer& timer) {

	timer_map_t::iterator it = timerMap.upper_bound(timer.id.u64);
	timer_map_t::iterator preIt = timerMap.end();
	if(timerMap.begin() != it) {
		preIt = --it;
	}
	if(timerMap.end() != preIt && timer.id.u64 == preIt->first) {
		if(!it->second->bUnique && !timer.bUnique) {
			CAutoPointer<CTimer> timerPointer(new CTimer(timer));
			it = timerMap.insert(it, timer_map_t::value_type(
				timer.id.u64, timerPointer));

			CTimeKey::lastAdd = GetTick();
			timeoutList.insert(it->second);
			return true;
		} else if(it->second->bUnique && !timer.bUnique) {
			// "The key must be unique."
			assert(false);
		} else if(!it->second->bUnique && timer.bUnique) {
			// "The key can't be set unique."
			assert(false);
		}
	} else {
		CAutoPointer<CTimer> timerPointer(new CTimer(timer));
		it = timerMap.insert(it, timer_map_t::value_type(
			timer.id.u64, timerPointer));

		CTimeKey::lastAdd = GetTick();
		timeoutList.insert(it->second);
		return true;
	}
	return false;
}

bool CTimerEvent::InnerRemoveTimer(id64_t id) {

	timer_map_t::iterator it = timerMap.find(id.u64);
	if(timerMap.end() != it) {
		do{
			timeoutList.erase(it->second);
			timerMap.erase(it++);

		} while(timerMap.end() != it && it->first == id.u64);
		return true;
	}
	return false;
}

bool CTimerEvent::ModifyIncrease(id64_t id, unsigned int increase) {
	if(0 == increase) {
		return false;
	}
	timer_map_t::iterator it(timerMap.find(id.u64));
	if(timerMap.end() != it) {
		do{
			timeoutList.erase(it->second);

			CTimeKey& key = it->second->key;
			uint64_t sum = key.delay + increase;
			if(sum >= (uint32_t)-1) {
				key.delay = (uint32_t)-1;
			} else {
				key.delay += increase;
			}

			CTimeKey::lastAdd = GetTick();
			timeoutList.insert(it->second);
			++it;
		} while(timerMap.end() != it && it->first == id.u64);
		return true;
	}
	return false;
}

bool CTimerEvent::ModifyDecrease(id64_t id, unsigned int decrease) {
	if(0 == decrease) {
		return false;
	}
	timer_map_t::iterator it(timerMap.find(id.u64));
	if(timerMap.end() != it) {
		do{
			timeoutList.erase(it->second);

			CTimeKey& key = it->second->key;
			if(decrease < key.delay) {
				key.delay -= decrease;
			} else {
				key.delay = 0;
			}

			CTimeKey::lastAdd = GetTick();
			timeoutList.insert(it->second);
			++it;
		} while(timerMap.end() != it && it->first == id.u64);
		return true;
	}
	return false;
}

bool CTimerEvent::ModifySet(id64_t id, unsigned int delay) {
	timer_map_t::iterator it(timerMap.find(id.u64));
	if(timerMap.end() != it) {
		do{
			CTimeKey& key = it->second->key;
			if(key.delay != delay) {
				timeoutList.erase(it->second);

				key.delay = delay;

				CTimeKey::lastAdd = GetTick();
				timeoutList.insert(it->second);
			}
			++it;
		} while(timerMap.end() != it && it->first == id.u64);
		return true;
	}
	return false;
}

bool CTimerEvent::ModifyReset(id64_t id) {
	timer_map_t::iterator it(timerMap.find(id.u64));
	if(timerMap.end() != it) {
		do{
			uint64_t startTick = GetTick();
			CTimeKey& key = it->second->key;
			if(startTick != key.playtime) {
				timeoutList.erase(it->second);

				key.playtime = startTick;

				CTimeKey::lastAdd = startTick;
				timeoutList.insert(it->second);
			}
			++it;
		} while(timerMap.end() != it && it->first == id.u64);
		return true;
	}
	return false;
}

bool CTimerEvent::ModifySetInterval(id64_t id, unsigned int interval) {
	timer_map_t::iterator it(timerMap.find(id.u64));
	if(timerMap.end() != it) {
		do{
			if(ELAPSE_TYPE_INTERVAL == it->second->utype) {
				CTimeKey& key = it->second->key;
				if(key.data.interval != interval) {
					if(key.delay == key.data.interval) {
						timeoutList.erase(it->second);

						key.data.interval = interval;
						key.delay = interval;

						CTimeKey::lastAdd = GetTick();
						timeoutList.insert(it->second);
					} else {
						key.data.interval = interval;
					}
				}
			}
			++it;
		} while(timerMap.end() != it && it->first == id.u64);
		return true;
	}
	return false;
}

bool CTimerEvent::ModifySetAttime(id64_t id, sAtTime attime) {
	timer_map_t::iterator it(timerMap.find(id.u64));
	if(timerMap.end() != it) {
		do{
			if(ELAPSE_TYPE_ATTIME == it->second->utype) {
				CTimeKey& key = it->second->key;

				if(attime.month != key.data.attime.month
					|| attime.day != key.data.attime.day
					|| attime.week != key.data.attime.week
					|| attime.hour != key.data.attime.hour
					|| attime.minute != key.data.attime.minute
					|| attime.second != key.data.attime.second)
				{
					timeoutList.erase(it->second);

					uint64_t startTick = GetTick();
					unsigned int interval = AtTimeInterval(attime, false);
					if(0 == interval) {
						interval = 1;
					}

					key.data.attime = attime;
					key.playtime = startTick;
					key.delay = interval;

					CTimeKey::lastAdd = startTick;
					timeoutList.insert(it->second);
				}
			}
			++it;
		} while(timerMap.end() != it && it->first == id.u64);
		return true;
	}
	return false;
}

void CTimerEvent::Loop()
{
	if(0 == loopThreadId) {
		 atomic_xchg(&loopThreadId, GetSysCurrentThreadId());
	}
	// add timer
	int nAddSize = incomingMessages.Size();
	for(int i = 0; i < nAddSize; ++i)
	{
		CTimer* pTimer = incomingMessages.ReadLock();
		if(pTimer != 0){
			if(ELAPSE_TYPE_GETTIMER == pTimer->utype) {
				CAutoPointer<CTimer> pInnerTimer(InnerFindTimer(
					pTimer->id, pTimer->method));
				if(!pInnerTimer.IsInvalid()) {
					*pTimer = *pInnerTimer;
					pTimer->result = true;
				} else {
					pTimer->result = false;
				}
			} else {
				pTimer->result = InnerAddTimer(*pTimer);
			}
			pTimer->spinEvent.Resume();
			incomingMessages.ReadUnlock();
			continue;
		}
		break;
	}
	// remove timer
	int nDeleteSize = deleteMessages.Size();
	for(int i = 0; i < nDeleteSize; ++i)
	{
		CTimeDelete* pD = deleteMessages.ReadLock();
		if(pD != 0){
			pD->result = InnerRemoveTimer(pD->id);
			pD->spinEvent.Resume();
			deleteMessages.ReadUnlock();
			continue;
		}
		break;
	}
	// modify timer
	int nModifySize = modifyMessages.Size();
	for(int i = 0; i < nModifySize; ++i)
	{
		CTimeModify* pM = modifyMessages.ReadLock();
		if(pM != 0){
			pM->result = InnerModifyTimer(pM->id, pM->operater, pM->data);
			pM->spinEvent.Resume();
			modifyMessages.ReadUnlock();
			continue;
		}
		break;
	}

	if(!timeoutList.empty()) {
		timeout_list_t::iterator it = timeoutList.begin();
		while(timeoutList.end() != it) {
			CAutoPointer<CTimer> pTimer(*it);
			if((pTimer->key.playtime + pTimer->key.delay) <= GetTick()) {
				timeoutList.erase(it++);

				if(pTimer->utype == ELAPSE_TYPE_INTERVAL) {

					uint64_t startTick = GetTick();
					pTimer->key.delay = pTimer->key.data.interval;
					pTimer->key.playtime = startTick;

					CTimeKey::lastAdd = startTick;
					timeoutList.insert(pTimer);
				} else if(pTimer->utype == ELAPSE_TYPE_ATTIME) {

					uint64_t startTick = GetTick();
					unsigned int interval = AtTimeInterval(pTimer->key.data.attime, true);
					if(0 == interval) {
						interval = 1;
					}
					pTimer->key.delay = interval;
					pTimer->key.playtime = startTick;

					CTimeKey::lastAdd = startTick;
					timeoutList.insert(pTimer);
				} else {
					// remove
					timer_map_t::iterator it(timerMap.find(pTimer->id.u64));
                    if(timerMap.end() != it) {
					    do {
						    if(it->second == pTimer) {
							    timerMap.erase(it);
							    break;
						    }
						    ++it;
					    } while(timerMap.end() != it && it->first == pTimer->id.u64);
                    }
				}

				OnTimeout(pTimer->workerThread, pTimer->id, pTimer->method);
				continue;
			}
			break;
		}
	}
}

uint64_t CTimerEvent::GetTick() {
    return GetSysTickCount()/100;
}

uint32_t CTimerEvent::AtTimeIntervalFrom(
    const struct sAtTime& atTime,
    const struct tm& tmNow,
    bool bNext/* = false*/)
{
	if(atTime.week > 0 && atTime.week < 8) {
		// weak loop
		int nWeek = atTime.week;
		if(7 == nWeek) {
			nWeek = 0;
		}
		int nHour(0);
		if(atTime.hour > 0 && atTime.hour < 24) {
			nHour = atTime.hour;
		}
		int nMinute(0);
		if(atTime.minute > 0 && atTime.minute < 60) {
			nMinute = atTime.minute;
		}
		int nSecond(0);
		if(atTime.second > 0 && atTime.second < 60) {
			nSecond = atTime.second;
		}


		int difWeek = nWeek - tmNow.tm_wday;
		if(difWeek < 0) {
			difWeek += 6;
		} else if(difWeek > 0){
			--difWeek;
		} else if(difWeek == 0) {
			int nFuture = nHour * 10000 + nMinute * 100 + nSecond;
			int nCurrent = tmNow.tm_hour * 10000 + tmNow.tm_min * 100 + tmNow.tm_sec;
			if(nCurrent > nFuture || (bNext && nCurrent == nFuture)) {
				difWeek = 6;
			} else {
				int difHour = nHour - tmNow.tm_hour;
				if(difHour > 0) {
					--difHour;
                    nMinute += 60;
				} else {
					assert(difHour == 0);
				}
				int difMinute = nMinute - tmNow.tm_min;
				if(difMinute > 0) {
					--difMinute;
                    nSecond += 60;
				} else if(difMinute < 0) {
					difMinute += 59;
				}
				int difSecond = nSecond - tmNow.tm_sec;
				if(difSecond < 0) {
					difSecond += 60;
				}
				return (difHour*60*60 + difMinute*60 + difSecond)*10;
			}
		}
		int difHour = 23 - tmNow.tm_hour;
		int difMinute = 59 - tmNow.tm_min;
		int difSecond = 60 - tmNow.tm_sec;
		return (difWeek * 24 * 60 * 60 + nHour * 60 * 60 + nMinute * 60 + nSecond
			+ difHour * 60 * 60 + difMinute * 60 + difSecond) * 10;
	} else {
		// [year, month, day, hour, minute, second] loop
		int nHour(0);
		if(atTime.hour > 0 && atTime.hour < 24) {
			nHour = atTime.hour;
		}
		int nMinute(0);
		if(atTime.minute > 0 && atTime.minute < 60) {
			nMinute = atTime.minute;
		}
		int nSecond(0);
		if(atTime.second > 0 && atTime.second < 60) {
			nSecond = atTime.second;
		}

		struct MyDate futureDay;

		int curMonth = tmNow.tm_mon + 1;
		if(atTime.month > 0 && atTime.month < 13) {
			if(atTime.day > 0 && atTime.day < 32) {
				futureDay.day = atTime.day;
			} else {
				futureDay.day = 1;
			}
			int nFuture = atTime.month*100000000 + futureDay.day*1000000 + nHour * 10000 + nMinute * 100 + nSecond;
			int nCurrent = curMonth*100000000 + tmNow.tm_mday*1000000 + tmNow.tm_hour * 10000 +
				tmNow.tm_min * 100 + tmNow.tm_sec;
			if(nCurrent > nFuture || (bNext && nCurrent == nFuture)) {
				futureDay.year = tmNow.tm_year + 1900 + 1;
			} else {
				futureDay.year = tmNow.tm_year + 1900;
			}
			futureDay.month = atTime.month;

		} else {
			futureDay.year = tmNow.tm_year + 1900;

			if(atTime.day > 0 && atTime.day < 32) {
				int nFuture = atTime.day*1000000 + nHour * 10000 + nMinute * 100 + nSecond;
				int nCurrent = tmNow.tm_mday*1000000 + tmNow.tm_hour * 10000 +
					tmNow.tm_min * 100 + tmNow.tm_sec;
				if(nCurrent > nFuture || (bNext && nCurrent == nFuture)) {
					futureDay.month = curMonth + 1;
				} else {
					futureDay.month = curMonth;
				}
				futureDay.day = atTime.day;
			} else {
				futureDay.month = curMonth;

				int nFuture = nHour * 10000 + nMinute * 100 + nSecond;
				int nCurrent = tmNow.tm_hour * 10000 +
					tmNow.tm_min * 100 + tmNow.tm_sec;
				if(nCurrent > nFuture || (bNext && nCurrent == nFuture)) {
					futureDay.day = tmNow.tm_mday + 1;
				} else {
					futureDay.day = tmNow.tm_mday;
				}
			}
		}

		struct MyDate curDay;
		curDay.year = tmNow.tm_year + 1900;
		curDay.month = curMonth;
		curDay.day = tmNow.tm_mday;

		int difDay = GetDiffDays(curDay, futureDay);
		assert(difDay >= 0);
		if(difDay > 0) {
			--difDay;

			int difHour = 23 - tmNow.tm_hour;
			int difMinute = 59 - tmNow.tm_min;
			int difSecond = 60 - tmNow.tm_sec;
			return (difDay * 24 * 60 * 60 + nHour*60*60 + nMinute * 60 + nSecond
				+ difHour * 60 * 60 + difMinute * 60 + difSecond) * 10;
		} else {
			int difHour = nHour - tmNow.tm_hour;
			if(difHour > 0) {
				--difHour;
                nMinute += 60;
			} else {
				assert(difHour == 0);
			}
			int difMinute = nMinute - tmNow.tm_min;
			if(difMinute > 0) {
				--difMinute;
                nSecond += 60;
			} else if(difMinute < 0) {
				difMinute += 59;
			}
			int difSecond = nSecond - tmNow.tm_sec;
			if(difSecond < 0) {
				difSecond += 60;
			}
			return (difHour*60*60 + difMinute*60 + difSecond)*10;
		}
	}
	return 0;
}

void CTimerEvent::OnTimeout(bool bWorkerThread, id64_t id, CAutoPointer<CallbackBase>& method)
{
	method->Invoke();
}

} // end namespace evt
