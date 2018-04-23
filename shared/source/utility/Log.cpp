/*
 * File:   Log.cpp
 * Author: Jehu Shaw
 *
 */

#include "Log.h"
#include "AtomicLock.h"
#include "SpinLock.h"

#include <cstdarg>
#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )
#include <io.h>
#include <direct.h>
#include <process.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

inline static off_t filelength(int fd) {
    if(fd == -1) {
        return -1;
    }
    struct stat fileStat;
    if(-1 == fstat(fd, &fileStat)) {
        return -1;
    }
    // deal returns.
    return fileStat.st_size;
}
#endif

using namespace std;

namespace util {

#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )
unsigned int _stdcall threadLoop(void * arguments){
	CLog * sts = (CLog *) arguments;
	sts->Run();
	return 0;
}
#else
void * threadLoop(void * arguments){
	CLog * sts = (CLog *) arguments;
	sts->Run();
	return 0;
}
#endif
}

using namespace util;

CLog::CLog():m_bStarted(false)
, m_queue(MININUM_LOG_QUEUE_SIZE) {

}

CLog::~CLog() {
	atomic_xchg8(&m_bStarted, false);
	m_event.Wait();
}

void CLog::Init(int32_t fileLogLevel, const string strLogPath)
{
	if(atomic_cmpxchg8(&m_bStarted, true, false) != false) {
		return;
	}
	m_strLogPath = strLogPath;
	TrimStringEx(m_strLogPath, '\"');

	if(access(m_strLogPath.c_str(), 0) == -1) {
#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )
		mkdir(m_strLogPath.c_str());
#else
		mkdir(m_strLogPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
	}

	SetFileLoggingLevel(fileLogLevel);

#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )
	HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, &threadLoop,
		(LPVOID)this, 0, NULL);
	CloseHandle(handle);
#else
	pthread_t tid;
	pthread_create(&tid, NULL, &threadLoop, (void*)this);
	pthread_detach(tid);
#endif

}

void CLog::SetFileLoggingLevel(int32_t level)
{
	//log level -1 is no more allowed
	if(level >= 0) {
		m_fileLogLevel = level;
	}
}

void CLog::OutFile(const char* szFileName, const char* szMsg, bool bPrintTime/* = true*/)
{
	if(NULL == szFileName || NULL == szMsg) {
		return;
	}

	struct OutputData* pOutput = m_queue.WriteLock();
	pOutput->strfileName = szFileName;

	if(bPrintTime) {
		char time_buffer[TIME_FORMAT_LENGTH];
		Time(time_buffer);
		pOutput->strMsg = time_buffer;
	}

	pOutput->strMsg += szMsg;
	m_queue.WriteUnlock();
}

void CLog::OutLog(const char* szFileName, const char* szSource, const char * szFormat, ...) {

	if(!m_bStarted)
		return;

	char buf[32768];
	va_list ap;

	va_start(ap, szFormat);
	if(NULL == szSource) {
		vsnprintf(buf, 32768, szFormat, ap);
	} else {
		std::string strFormat(szFormat);
		strFormat += szSource;
		vsnprintf(buf, 32768, strFormat.c_str(), ap);
	}
	va_end(ap);

	OutFile(szFileName, buf);
}

void CLog::OutString(const char* szSource, const char * szFormat, ...)
{
	if(!m_bStarted)
		return;

	char buf[32768];
	va_list ap;

	va_start(ap, szFormat);
	if(NULL == szSource) {
		vsnprintf(buf, 32768, szFormat, ap);
	} else {
		std::string strFormat(szFormat);
		strFormat += szSource;
		vsnprintf(buf, 32768, strFormat.c_str(), ap);
	}
	va_end(ap);

	OutFile(NORMAL_LOG_FILE_NAME, buf);
}

void CLog::OutError(const char* szSource, const char * szFormat, ...)
{
	if(!m_bStarted)
		return;

	char buf[32768];
	va_list ap;

	va_start(ap, szFormat);
	if(NULL == szSource) {
		vsnprintf(buf, 32768, szFormat, ap);
	} else {
		std::string strFormat(szFormat);
		strFormat += szSource;
		vsnprintf(buf, 32768, strFormat.c_str(), ap);
	}	
	va_end(ap);

	OutFile(ERROR_LOG_FILE_NAME, buf);
}

void CLog::OutBasic(const char* szSource, const char * szFormat, ...)
{
	if(!m_bStarted)
		return;

	char buf[32768];
	va_list ap;

	va_start(ap, szFormat);
	if(NULL == szSource) {
		vsnprintf(buf, 32768, szFormat, ap);
	} else {
		std::string strFormat(szFormat);
		strFormat += szSource;
		vsnprintf(buf, 32768, strFormat.c_str(), ap);
	}
	va_end(ap);

	OutFile(NORMAL_LOG_FILE_NAME, buf);
}

void CLog::OutDetail(const char* szSource, const char * szFormat, ...)
{
	if(!m_bStarted || m_fileLogLevel < 1)
		return;

	char buf[32768];
	va_list ap;

	va_start(ap, szFormat);
	if(NULL == szSource) {
		vsnprintf(buf, 32768, szFormat, ap);
	} else {
		std::string strFormat(szFormat);
		strFormat += szSource;
		vsnprintf(buf, 32768, strFormat.c_str(), ap);
	}
	va_end(ap);

	OutFile(NORMAL_LOG_FILE_NAME, buf);
}

void CLog::OutDebug(const char* szSource, const char * szFormat, ...)
{
	if(!m_bStarted || m_fileLogLevel < 2)
		return;

	char buf[32768];
	va_list ap;

	va_start(ap, szFormat);
	if(NULL == szSource) {
		vsnprintf(buf, 32768, szFormat, ap);
	} else {
		std::string strFormat(szFormat);
		strFormat += szSource;
		vsnprintf(buf, 32768, strFormat.c_str(), ap);
	}
	va_end(ap);

	OutFile(DEBUG_LOG_FILE_NAME, buf);
}

void CLog::Run()
{
	m_event.Suspend();
	while(m_bStarted) {

		int nSize = m_queue.Size();
		for(int i = 0; i < nSize; ++i) {
			struct OutputData* pOutput = m_queue.ReadLock();
			if(NULL == pOutput) {
				break;
			}

			time_t curTime = time(NULL);
			struct tm tmCur = {0,0,0,0,0,0,0,0,0};
			localtime_r(&curTime, &tmCur);

			char szLogFile[MAX_PATH] ={'\0'};
			snprintf(szLogFile, sizeof(szLogFile), "%s%s(%4d-%02d-%02d).log", m_strLogPath.c_str(),
				pOutput->strfileName.c_str(), tmCur.tm_year + 1900, tmCur.tm_mon+1, tmCur.tm_mday);
			szLogFile[sizeof(szLogFile) - 1] = '\0';

			FILE* file(NULL);
#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )
			if(fopen_s(&file, szLogFile, "a+") != 0) {
				m_queue.CancelReadLock(pOutput);
				continue;
			}
#else
			file = fopen(szLogFile, "a+");
			if(NULL == file) {
				m_queue.CancelReadLock(pOutput);
				continue;
			}
#endif
			fprintf(file, "%s\n", pOutput->strMsg.c_str());
			long logLength = filelength(fileno(file));
			fclose(file);
			// Log file is too large
			if (logLength >= MAX_LOG_FILE_SIZE) {

				char szBackupFile[MAX_PATH] = {'\0'};
				snprintf(szBackupFile, sizeof(szBackupFile), "%s%s(%4d-%02d-%02d-%02d%02d%02d-backup).log",
					m_strLogPath.c_str(), pOutput->strfileName.c_str(), tmCur.tm_year + 1900, tmCur.tm_mon+1,
					tmCur.tm_mday, tmCur.tm_hour, tmCur.tm_min, tmCur.tm_sec);
				szBackupFile[sizeof(szBackupFile) - 1] = '\0';

				rename(szLogFile, szBackupFile);
			}
			m_queue.ReadUnlock();
		}
		Sleep(10);
	}
	m_event.Resume();
}

time_t CLog::Time(char* buffer)
{
	time_t now;
	struct tm timeinfo;

	time( &now );
	localtime_r(&now, &timeinfo);

	if(NULL == buffer) {
		return now;
	}

	strftime(buffer,TIME_FORMAT_LENGTH,TIME_FORMAT, &timeinfo);
	return now;
}

