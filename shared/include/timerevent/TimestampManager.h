/* 
 * File:   TimestampManager.h
 * Author: Jehu Shaw
 *
 * Created on 2011_7_26, 11:09
 */

#ifndef TIMESTAMPMANAGER_H
#define	TIMESTAMPMANAGER_H

#include "Common.h"
#if COMPILER == COMPILER_MICROSOFT
#include <Mmsystem.h>
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <string>
#include "Singleton.h"

namespace evt {

// convert UCT to UNIX epoch
// UNIX epoch, in UCT secs 
#define UNIXEPOCH 2208988800UL

#define TM_YEAR_BASE (1900)

// Get sine 1900.1.1 timestamp 
class SHARED_DLL_DECL CTimestampManager
	: public util::Singleton<CTimestampManager>
{
public:
	CTimestampManager();

	~CTimestampManager();

	time_t GetTimestamp() {
		return m_uTimestamp;
	}

	void GetTM(struct tm* pTm) {
		time_t nNow = GetTimestamp();
		localtime_r(&nNow, pTm);
	}

    static double DiffTime(time_t _Time1, time_t _Time2) {
        return difftime(_Time1, _Time2);
    }

    static void GetTM(struct tm* pTm, time_t nNow) {
        localtime_r(&nNow, pTm);
    }

	size_t GetLocalDateTimeStr(char* szTimeBuf, size_t timeBufSize) {
		struct tm curTm = {0,0,0,0,0,0,0,0,0};
		GetTM(&curTm);
		return strftime(szTimeBuf, timeBufSize, "%Y-%m-%d %H:%M:%S", &curTm);
	}

    std::string GetLocalDateTimeStr() {
        char szTimeBuf[33] = {'\0'};
        struct tm curTm = {0,0,0,0,0,0,0,0,0};
        GetTM(&curTm);
        strftime(szTimeBuf, sizeof(szTimeBuf) - 1, "%Y-%m-%d %H:%M:%S", &curTm);
        return std::string(szTimeBuf);
    }

    static std::string TimestampToString(time_t timestamp) {
        struct tm curTm = {0,0,0,0,0,0,0,0,0};
        GetTM(&curTm, timestamp);
        char szTimeBuf[33] = {'\0'};
        strftime(szTimeBuf, sizeof(szTimeBuf) - 1, "%Y-%m-%d %H:%M:%S", &curTm);
        return std::string(szTimeBuf);
    }

    static std::string TMToString(const struct tm* pTm) {
        if(NULL == pTm) {
            return std::string();
        }
        char szTimeBuf[33] = {'\0'};
        strftime(szTimeBuf, sizeof(szTimeBuf) - 1, "%Y-%m-%d %H:%M:%S", pTm);
        return std::string(szTimeBuf);
    }

    static bool StringToTM(const char* szTimeBuf, struct tm* pTm) {
        if(NULL == szTimeBuf || NULL == pTm) {
            return false;
        }
        
        if(sscanf(szTimeBuf,"%4d-%2d-%2d %2d:%2d:%2d",&pTm->tm_year,&pTm->tm_mon
            ,&pTm->tm_mday,&pTm->tm_hour,&pTm->tm_min,&pTm->tm_sec) < 6) 
        {
            return false;
        }
        {
            int c = int(pTm->tm_year / 100), y = pTm->tm_year - 100 * c;  
            int w = int(c / 4) - 2*c +y +int(y/4) +(26 * (pTm->tm_mon + 1)/10 ) + pTm->tm_mday - 1;  
            w = ( w % 7 + 7 ) % 7;  
            pTm->tm_wday = w;
        }
        {
            int a[12]={31,28,31,30,31,30,31,31,30,31,30,31};
            int b[12]={31,29,31,30,31,30,31,31,30,31,30,31};
            int i,sum=0;
            if((pTm->tm_year % 4 == 0 && pTm->tm_year % 100 != 0)
                ||(pTm->tm_year % 400 == 0)) 
            {
                for(i = 0; i < pTm->tm_mon - 1; ++i) {
                    sum += b[i];
                }
            } else {
                for(i = 0; i < pTm->tm_mon - 1; ++i) {
                    sum += a[i];
                }
            }
            sum += pTm->tm_mday;
            pTm->tm_yday = sum - 1;
        }
        pTm->tm_year -= TM_YEAR_BASE;
        pTm->tm_mon -= 1;
        return true;
    }

    static time_t StringToTimestamp(const char* szTimeBuf) {
        if(NULL == szTimeBuf) {
            return 0;
        }
        struct tm t;
        if(sscanf(szTimeBuf,"%4d-%2d-%2d %2d:%2d:%2d",&t.tm_year,&t.tm_mon
            ,&t.tm_mday,&t.tm_hour,&t.tm_min,&t.tm_sec) < 6) 
        {
            return 0;
        }
        t.tm_year -= TM_YEAR_BASE;
        t.tm_mon -= 1;
        return mktime(&t);
    }

    static time_t TMtoTimestamp(struct tm* pTm) {
        return mktime(pTm);
    }

    static bool IsToday(const std::string& strDayTime) {
        struct tm curTM;
        memset(&curTM,0,sizeof(curTM));
		CTimestampManager::PTR_T pTsMgr(CTimestampManager::Pointer());
        pTsMgr->GetTM(&curTM);
        struct tm lastTM;
        memset(&lastTM,0,sizeof(lastTM));
        if(StringToTM(strDayTime.c_str(), &lastTM)) {
            if(curTM.tm_year == lastTM.tm_year
                && curTM.tm_mon == lastTM.tm_mon
                && curTM.tm_mday == lastTM.tm_mday)
            {
                return true;
            }
        }
        return false;
    }

    static time_t GetUnixTimeOnline();

private:
	static volatile time_t m_uTimestamp;

private:
	bool startTick(void);
    bool stopTick(void);

#if defined(_WIN32) || defined(_WIN64)

	MMRESULT m_iTimerID;
	static void WINAPI onTimeFunc(UINT wTimerID, UINT msg,
		DWORD dwUser, DWORD dwl, DWORD dw2);
#else
	struct itimerval m_ovalue;
        void (*oldOnTimeFunc)(int);
	static void onTimeFunc(int signo);
#endif
};

}

#endif	/* TIMESTAMPMANAGER_H */

