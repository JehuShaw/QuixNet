/*
 * File:   Log.h
 * Author: Jehu Shaw
 *
 */

#ifndef SERVER_LOG_H
#define SERVER_LOG_H

#include "Common.h"
#include "Singleton.h"
#include "CircleQueue.h"
#include "SpinEvent.h"

namespace util {

#define TIME_FORMAT "[%H:%M:%S]"
#define TIME_FORMAT_LENGTH 11
// set default file name
#define NORMAL_LOG_FILE_NAME "normal"
#define DEBUG_LOG_FILE_NAME "debug"
#define WARNING_LOG_FILE_NAME "warning"
#define ERROR_LOG_FILE_NAME "error"

static const long MAX_LOG_FILE_SIZE = 1024 * 1024 * 16;

struct OutputData {
	std::string strfileName;
	std::string strMsg;
}; 

class SHARED_DLL_DECL CLog : public Singleton< CLog > {
public:
	CLog();
	void Init(int32_t fileLogLevel, const std::string strLogPath = "./log/");
	void Dispose();
	// set print log level
	void SetFileLoggingLevel(int32_t level);
	//log level 0
	void OutFile(const char* szFileName, const char* szMsg, bool bPrintTime = true);
	void OutLog(const char* szFileName, const char* szSource, const char * szFormat, ...);
	void OutString(const char* szSource, const char * szFormat, ...);
	void OutError(const char* szSource, const char * szFormat, ...);
	void OutBasic(const char* szSource, const char * szFormat, ...);
	//log level 1
	void OutDetail(const char* szSource, const char * szFormat, ...);
	//log level 2
	void OutDebug(const char* szSource, const char * szFormat, ...);

private:
	void Run();

	static time_t Time(char *buffer);

	static INLINE char DCD(char in){
		char out = in;
		out -= 13;
		out ^= 131;
		return out;
	} 

	static void DCDS(char *str) {
		unsigned long i = 0;
		size_t len = strlen(str);

		for(i = 0; i < len; ++i )
			str[i] = DCD(str[i]);

	}

	static void PDCDS(const char *str, char *buf) {
		strcpy(buf, str);
		DCDS(buf);
	}

private:
	volatile long m_fileLogLevel;
	thd::CSpinEvent m_event;
	volatile bool m_bStarted;
	thd::CCircleQueue<struct OutputData, 64> m_queue;
	std::string m_strLogPath;


#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 ) || defined( _WIN64 )
friend unsigned int _stdcall threadLoop(void * arguments);
#else
friend void * threadLoop(void * arguments);
#endif

};

#define LogInit(fileLogLevel, strLogPath) util::CLog::Pointer()->Init(fileLogLevel, strLogPath)
#define LogRelease util::CLog::Pointer()->Dispose(); util::CLog::Pointer()->Release
#define LogLevel(level) util::CLog::Pointer()->SetFileLoggingLevel(level)
#define PrintFile(szFileName, szMsg, bPrintTime) util::CLog::Pointer()->OutFile(szFileName, szMsg, bPrintTime)
#define PrintLog(szFileName, fmt, ...) util::CLog::Pointer()->OutLog(szFileName, NULL, fmt, ## __VA_ARGS__)
#define PrintError(fmt, ...) util::CLog::Pointer()->OutError(NULL, fmt, ## __VA_ARGS__)
#define PrintString(fmt, ...) util::CLog::Pointer()->OutString(NULL, fmt, ## __VA_ARGS__)
#define PrintBasic(fmt, ...) util::CLog::Pointer()->OutBasic(NULL, fmt, ## __VA_ARGS__)
#define PrintDetail(fmt, ...) util::CLog::Pointer()->OutDetail(NULL, fmt, ## __VA_ARGS__)
#define PrintDebug(fmt, ...) util::CLog::Pointer()->OutDebug(NULL, fmt, ## __VA_ARGS__)

#define OutputLog(szFileName, fmt, ...) util::CLog::Pointer()->OutLog(szFileName, " [Function:\"%s\", Line:%d, File:\"%s\"] ", fmt, ## __VA_ARGS__, __FUNCTION__, __LINE__, __FILE__)
#define OutputError(fmt, ...) util::CLog::Pointer()->OutError(" [Function:\"%s\", Line:%d, File:\"%s\"] ", fmt, ## __VA_ARGS__, __FUNCTION__, __LINE__, __FILE__)
#define OutputString(fmt, ...) util::CLog::Pointer()->OutString(" [Function:\"%s\", Line:%d, File:\"%s\"] ", fmt, ## __VA_ARGS__, __FUNCTION__, __LINE__, __FILE__)
#define OutputBasic(fmt, ...) util::CLog::Pointer()->OutBasic(" [Function:\"%s\", Line:%d, File:\"%s\"] ", fmt, ## __VA_ARGS__, __FUNCTION__, __LINE__, __FILE__)
#define OutputDetail(fmt, ...) util::CLog::Pointer()->OutDetail(" [Function:\"%s\", Line:%d, File:\"%s\"] ", fmt, ## __VA_ARGS__, __FUNCTION__, __LINE__, __FILE__)
#define OutputDebug(fmt, ...) util::CLog::Pointer()->OutDebug(" [Function:\"%s\", Line:%d, File:\"%s\"] ", fmt, ## __VA_ARGS__, __FUNCTION__, __LINE__, __FILE__)
}

#endif /* SERVER_LOG_H */
