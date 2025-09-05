/*
 * File:   TimerEvent.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_17, 9:45
 */

#ifndef TIMEREVENT_H
#define	TIMEREVENT_H

#include "Common.h"
#include <map>
#include <set>
#include "CircleQueue.h"
#include "CallBack.h"
#include "AutoPointer.h"
#include "SpinEvent.h"

namespace evt
{

// 10 second
#define TIMER_WAIT_RESULT_TIMEOUT_SECOND 10000

	typedef enum eTimerOperater {
		TIMER_OPERATER_INCREASE,
		TIMER_OPERATER_DECREASE,
		TIMER_OPERATER_SET,
		TIMER_OPERATER_RESET,
		TIMER_OPERATER_SET_INTERVAL,
		TIMER_OPERATER_SET_ATTIME,
	} eTimerOperater;

    enum {
        ELAPSE_TYPE_DELAY,
        ELAPSE_TYPE_INTERVAL,
        ELAPSE_TYPE_ATTIME,
		ELAPSE_TYPE_GETTIMER,
    };

	typedef struct sAtTime {
		unsigned char month;	/* months since January - [1,12] */
		unsigned char day;		/* day of the month - [1,31] */
		unsigned char week;		/* days since Sunday - [1,7] */
		unsigned char hour;		/* hours since midnight - [0,23] */
		unsigned char minute;	/* minutes after the hour - [0,59] */
		unsigned char second;	/* seconds after the minute - [0,59] */
	} AtTime;

	typedef union uTimerData {
		unsigned int delay;
		unsigned int interval;
		struct sAtTime attime;

		uTimerData() { memset(&attime, 0, sizeof(attime)); }
		uTimerData(unsigned int orig) { memset(&attime, 0, sizeof(attime)); delay = orig; }
		uTimerData(AtTime orig) : attime(orig) {}
		uTimerData(const uTimerData& orig) : attime(orig.attime) {}

		uTimerData& operator = (const uTimerData& right) {
			this->attime = right.attime;
			return *this;
		}
		uTimerData& operator = (unsigned int right) {
			memset(&attime, 0, sizeof(attime));
			delay = right;
			return *this;
		}

		uTimerData& operator = (AtTime right) {
			attime = right;
			return *this;
		}
	} timer_data_t;

	const static timer_data_t timer_data_null;

    class CTimeModify {
    public:
        id64_t id;                           // id
		timer_data_t data;
		short operater;
		bool result;
		thd::CSpinEvent spinEvent;
    };

	class CTimeDelete {
	public:
		id64_t id;                          // id
		bool result;
		thd::CSpinEvent spinEvent;
	};

    inline static unsigned char TMWdayToAtTimeWeek(unsigned char tmWday) {
        if(0 == tmWday) {
            return 7;
        } else {
            return tmWday;
        }
    }

    inline static unsigned char AtTimeWeekToTMWday(unsigned char atTimeWeek) {
        if(7 == atTimeWeek) {
            return 0;
        } else {
            return atTimeWeek;
        }
    }

    class CTimeKey {
    public:
		CTimeKey()
			: playtime(0)
			, data()
			, delay(0)
		{
		}

        bool operator == (const CTimeKey& right) const;
        bool operator != (const CTimeKey& right) const;
        bool operator > (const CTimeKey& right) const;
        bool operator < (const CTimeKey& right) const;

		uint64_t playtime;						// record timer start tick
		timer_data_t data;						// store interval or attime data
		unsigned int delay;						// each delay time

        static uint64_t lastAdd;
    };

    class CTimer {
    public:
		CTimer() 
			: id(0)
			, key()
			, bUnique(true)
			, utype(0)
			, result(false)
			, workerThread(false) {}

        CTimer(const CTimer& orig)
			: id(orig.id)
			, key(orig.key)
			, method(orig.method)
            , bUnique(orig.bUnique)
			, utype(orig.utype)
			, result(orig.result)
			, workerThread(orig.workerThread)
		{
        }

        CTimer& operator = (const CTimer& input) {
			this->id = input.id;
			this->bUnique = input.bUnique;
			this->utype = input.utype;
            this->key = input.key;
            this->method = input.method;
			this->result = input.result;
			this->workerThread = input.workerThread;
            return *this;
        }

		bool operator == (const CTimer& right) const {
			return this->key == right.key;
		}
		bool operator != (const CTimer& right) const {
			return this->key != right.key;
		}
		bool operator > (const CTimer& right) const {
			return this->key > right.key;
		}
		bool operator < (const CTimer& right) const {
			return this->key < right.key;
		}

        id64_t GetID(void)const { return id; }

		int GetTimeLeft()const;

		uint64_t GetStartTime()const { return key.playtime; }
		sAtTime GetAtTime()const { return key.data.attime; }
		unsigned int GetDelay()const { return key.delay; }
		unsigned int GetInterval()const { return key.data.interval; }
		bool IsUniqueID()const { return bUnique; }
		unsigned char GetElapseType()const { return utype; }
		bool IsWorkerThread()const { return workerThread; }

        util::CAutoPointer<util::CallbackBase> GetCallback()const { return method; }

    private:
		id64_t id;
        CTimeKey key;
		util::CAutoPointer<util::CallbackBase> method;
        friend class CTimerEvent;
		friend struct TimerListCompare;
		bool bUnique;	// If true, can't set the same id.
		unsigned char utype;
		bool result;
		bool workerThread;
		thd::CSpinEvent spinEvent;
    };

	struct TimerListCompare {
		bool operator()(const util::CAutoPointer<CTimer>& pTimer1
			, const util::CAutoPointer<CTimer>& pTimer2)const
		{
			return pTimer1->key < pTimer2->key;
		}
	};

    class SHARED_DLL_DECL CTimerEvent {
    public:
        CTimerEvent();
        virtual ~CTimerEvent();
		/**
		* set a timeout
		* @param id     timer ID
		* @param delay  unit of 0.01 second
		* @param method    callback method
		* @param wait_result  do you want wait the result?
		* @param unique  unique id?
		* @param worker_thread  If true open a new thread invoke the callback method.
		* @param threadOrder If worker_thread open, then thread sequential execution by threadOrder except 0 value.
		*/
        bool SetTimeout(id64_t id, unsigned int delay,
			util::CAutoPointer<util::CallbackBase> method, bool bWaitResult = false,
            bool bUniqueId = true, bool bWorkerThread = true, uint64_t threadOrder = 0);
		/**
		* set a interval timer
		* @param id     timer ID
		* @param interval  unit of 0.01 second
		* @param method    callback method
		* @param delay   first delay time , unit of 0.01 second
		* @param wait_result  do you want wait the result?
		* @param unique  unique id?
		* @param worker_thread  If true open a new thread invoke the callback method.
		* @param threadOrder If worker_thread open, then thread sequential execution by threadOrder except 0 value.
		*/
        bool SetInterval(id64_t id, unsigned int interval,
			util::CAutoPointer<util::CallbackBase> method,
			unsigned int delay = 0, bool bWaitResult = false,
            bool bUniqueId = true, bool bWorkerThread = true,
			uint64_t threadOrder = 0);
		/**
		* set a atTime timer
		* @param id     timer ID
		* @param atTime
		* @param method    callback method
		* @param wait_result  do you want wait the result?
		* @param unique  unique id?
		* @param worker_thread  If true open a new thread invoke the callback method.
		* @param threadOrder If worker_thread open, then thread sequential execution by threadOrder except 0 value.
		*/
        bool SetAtTime(id64_t id, const sAtTime& atTime,
			util::CAutoPointer<util::CallbackBase> method, bool bWaitResult = false,
            bool bUniqueId = true, bool bWorkerThread = true, uint64_t threadOrder = 0);

		/**
		* set a atTime timer
		* @param id     timer ID
		*/
		inline util::CAutoPointer<CTimer> GetTimer(id64_t id) {
			return GetTimer(id, util::CAutoPointer<util::CallbackBase>());
		}

		/**
		* set a atTime timer
		* @param id     timer ID
		* @param method    callback method  If you set repeated id, you must set callback method to find the timer.
		*/
		util::CAutoPointer<CTimer> GetTimer(id64_t id, util::CAutoPointer<util::CallbackBase> method);

		/**
		* modify the interval value
		* @param id       timer ID
		* @param operater
		* @param data  0.01 second
		* @param wait_result  do you want wait the result?
		*/
        bool Modify(id64_t id, eTimerOperater operater, timer_data_t data = 0, bool bWaitResult = false);
		/**
		* remove timer
		* @param id    The timer ID
		* @param wait_result  do you want wait the result?
		*/
        bool Remove(id64_t id, bool bWaitResult = false);

		/**
		* Let it in the thread loop
		*/
		void Loop();

		/**
		 *  0.01 second tick
		 * @return
		 */
		static uint64_t GetTick();

        /**
		 *  past time 0.01 second
		 * @return
		 */
        static uint32_t AtTimeIntervalFrom(
            const struct sAtTime& atTime,
            const struct tm& tmNow,
            bool bNext = false);

	protected:
		virtual void OnTimeout(
			id64_t id,
			util::CAutoPointer<util::CallbackBase>& pMethod,
			bool bWorkerThread);

		util::CAutoPointer<CTimer> InnerFindTimer(id64_t id, util::CAutoPointer<util::CallbackBase> method);

		bool InnerAddTimer(const CTimer& timer);

		bool InnerRemoveTimer(id64_t id);

		inline bool InnerModifyTimer(id64_t id, short operater, timer_data_t data) {

			if((short)TIMER_OPERATER_INCREASE == operater) {
				return ModifyIncrease(id, data.delay);
			} else if((short)TIMER_OPERATER_SET == operater) {
				return ModifySet(id, data.delay);
			} else if((short)TIMER_OPERATER_RESET == operater) {
				return ModifyReset(id);
			} else if((short)TIMER_OPERATER_DECREASE == operater) {
				return ModifyDecrease(id, data.delay);
			} else if((short)TIMER_OPERATER_SET_INTERVAL == operater) {
				return ModifySetInterval(id, data.interval);
			} else if((short)TIMER_OPERATER_SET_ATTIME == operater) {
				return ModifySetAttime(id, data.attime);
			}
			return false;
		}

		bool ModifyIncrease(id64_t id, unsigned int increase);

		bool ModifyDecrease(id64_t id, unsigned int decrease);

		bool ModifySet(id64_t id, unsigned int delay);

		bool ModifyReset(id64_t id);

		bool ModifySetInterval(id64_t id, unsigned int interval);

		bool ModifySetAttime(id64_t id, sAtTime attime);

	protected:
        typedef std::multimap<uint64_t, util::CAutoPointer<CTimer> > timer_map_t;
        timer_map_t timerMap;

		typedef std::set<util::CAutoPointer<CTimer>, struct TimerListCompare> timeout_list_t;
		timeout_list_t timeoutList;

        thd::CCircleQueue<CTimer> incomingMessages;
        thd::CCircleQueue<CTimeDelete> deleteMessages;
        thd::CCircleQueue<CTimeModify> modifyMessages;
		volatile unsigned long loopThreadId;
        volatile bool bDispose;

		//static volatile uint32_t counter;

		//static bool startTick(void);
		//static bool stopTick(void);
//#if defined( _WIN32 ) || defined( _WIN64 )
//		static UINT_PTR iTimerID;
//		static VOID CALLBACK TimerProc (HWND hwnd,
//			UINT message, UINT iTimerID, DWORD dwTime);
//#else
//		static struct itimerval ovalue;
//		static void sig_func(int signo);
//#endif
    };

}
#endif	/* TIMEREVENT_H */

