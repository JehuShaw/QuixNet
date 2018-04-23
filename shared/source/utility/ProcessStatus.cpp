#include "ProcessStatus.h"

#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )
#include <windows.h>
#include <psapi.h>

#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "Psapi.lib")

#else
#include "sys/times.h"

#endif

#include <assert.h>
#include "AtomicLock.h"

#define MAX_NUMBER_SIZE 65



namespace util {

#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )
	// Convert to single value.
	inline static uint64_t file_time_2_utc(const FILETIME* ftime) {
		if(NULL == ftime) {
			assert(ftime);
			return 0;
		}

		LARGE_INTEGER li;
		li.LowPart = ftime->dwLowDateTime;
		li.HighPart = ftime->dwHighDateTime;
		return li.QuadPart;
	}

	// Get the number of cpu core.
	static unsigned long get_processor_number() {
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return (unsigned long)info.dwNumberOfProcessors;
	}

	const static unsigned long g_numProcessors = get_processor_number();

	// CPU currently used by current process
	int CpuUsage()
	{
		volatile static uint64_t s_lastTime = 0;
		volatile static uint64_t s_lastSysTime = 0;

		FILETIME now;
		FILETIME creationTime;
		FILETIME exitTime;
		FILETIME kernelTime;
		FILETIME userTime;
		uint64_t systemTime;
		uint64_t time;
		int64_t sysTimeDelta;
		int64_t timeDelta;

		int percent = 0;

		GetSystemTimeAsFileTime(&now);

		if(!GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime,
			&kernelTime, &userTime))
		{
			// We don't assert here because in some cases (such as in the Task Manager)
			// we may call this function on a process that has just exited but we have
			// not yet received the notification.
			return -1;
		}
		systemTime = (file_time_2_utc(&kernelTime) + file_time_2_utc(&userTime))
			/ g_numProcessors;
		time = file_time_2_utc(&now);

		if((s_lastSysTime == 0) || (s_lastTime == 0)) {
			// First call, just set the last values.
			atomic_xchg64(&s_lastSysTime, systemTime);
			atomic_xchg64(&s_lastTime, time);
			return -1;
		}

		sysTimeDelta = systemTime - s_lastSysTime;
		timeDelta = time - s_lastTime;

		if(timeDelta == 0) {
			return -1;
		}

		// We add time_delta / 2 so the result is rounded.
		percent = (int)((sysTimeDelta * 100 + timeDelta / 2) / timeDelta);
		atomic_xchg64(&s_lastSysTime, systemTime);
		atomic_xchg64(&s_lastTime, time);

		return percent;
	}

	// Physical Memory currently used by current process
	int64_t MemoryUsage()
	{
		PROCESS_MEMORY_COUNTERS pmc;
		if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
		{
			return pmc.WorkingSetSize;
		}
		return 0;
	}

	// Virtual Memory currently used by current process
	int64_t VirtualMemoryUsage()
	{
		PROCESS_MEMORY_COUNTERS pmc;
		if(GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
		{
			return pmc.PagefileUsage;
		}
		return 0;
	}

	int64_t IoReadBytes()
	{
		IO_COUNTERS io_counter;
		if(GetProcessIoCounters(GetCurrentProcess(), &io_counter))
		{
			return io_counter.ReadTransferCount;
		}
		return 0;
	}

	int64_t IoWriteBytes()
	{
		IO_COUNTERS io_counter;
		if(GetProcessIoCounters(GetCurrentProcess(), &io_counter))
		{
			return io_counter.WriteTransferCount;
		}
		return 0;
	}

#else

	static int64_t parseLine(char* line) {
		if(NULL == line) {
			assert(line);
			return 0;
		}
		//int i = strlen(line);
		while (*line < '0' || *line > '9') ++line;
		//line[i-3] = '\0';
		return atoll(line);
	}

	volatile static int64_t g_lastCPU, g_lastSysCPU, g_lastUserCPU;

	int initVariable(){
		FILE* file;
		struct tms timeSample;
		char line[128];

		atomic_xchg64(&g_lastCPU, times(&timeSample));
		atomic_xchg64(&g_lastSysCPU, timeSample.tms_stime);
		atomic_xchg64(&g_lastUserCPU, timeSample.tms_utime);

		file = fopen("/proc/cpuinfo", "r");
		int numProcessors = 0;
		while(fgets(line, 128, file) != NULL){
			if(strncmp(line, "processor", 9) == 0) {
				++numProcessors;
			}
		}
		fclose(file);
		return numProcessors;
	}
	const static int g_numProcessors = initVariable();

	// CPU currently used by current process
	int CpuUsage()
	{
		struct tms timeSample;
		clock_t now;
		double percent;

		now = times(&timeSample);
		if (now <= g_lastCPU || timeSample.tms_stime < g_lastSysCPU ||
			timeSample.tms_utime < g_lastUserCPU){
				//Overflow detection. Just skip this value.
				percent = -1.0;
		}
		else{
			percent = (timeSample.tms_stime - g_lastSysCPU) +
				(timeSample.tms_utime - g_lastUserCPU);
			percent /= (now - g_lastCPU);
			percent /= g_numProcessors;
			percent *= 100;
		}
		atomic_xchg64(&g_lastCPU, now);
		atomic_xchg64(&g_lastSysCPU, timeSample.tms_stime);
		atomic_xchg64(&g_lastUserCPU, timeSample.tms_utime);

		return percent;
	}

	// Physical Memory currently used by current process
	int64_t MemoryUsage()
	{
		FILE* file = fopen("/proc/self/status", "r");
		if(NULL == file) {
			assert(file);
			return 0;
		}
		int64_t result = -1;
		char line[128];

		while(fgets(line, 128, file) != NULL) {
			if(strncmp(line, "VmRSS:", 6) == 0) {
				result = parseLine(line);
				break;
			}
		}
		fclose(file);
		return result;
	}

	// Virtual Memory currently used by current process
	int64_t VirtualMemoryUsage()
	{
		FILE* file = fopen("/proc/self/status", "r");
		if(NULL == file) {
			assert(file);
			return 0;
		}
		int64_t result = -1;
		char line[128];

		while (fgets(line, 128, file) != NULL){
			if (strncmp(line, "VmSize:", 7) == 0){
				result = parseLine(line);
				break;
			}
		}
		fclose(file);
		return result;
	}

	int64_t IoReadBytes()
	{
		FILE* file = fopen("/proc/self/io", "r");
		if(NULL == file) {
			assert(file);
			return 0;
		}
		int64_t result = -1;
		char line[128];

		while (fgets(line, 128, file) != NULL){
			if (strncmp(line, "read_bytes:", 11) == 0){
				result = parseLine(line);
				break;
			}
		}
		fclose(file);
		return result;
	}

	int64_t IoWriteBytes()
	{
		FILE* file = fopen("/proc/self/io", "r");
		if(NULL == file) {
			assert(file);
			return 0;
		}
		int64_t result = -1;
		char line[128];

		while (fgets(line, 128, file) != NULL){
			if (strncmp(line, "write_bytes:", 12) == 0){
				result = parseLine(line);
				break;
			}
		}
		fclose(file);
		return result;
	}
#endif


/// 获取当前进程的cpu usage
std::string GetCPUUsageStr() {
	char szBuf[MAX_NUMBER_SIZE] = {'\0'};
	snprintf(szBuf, sizeof(szBuf), "%d", CpuUsage());
	return std::string(szBuf);
}
/// 获取当前进程内存使用量
std::string GetMemoryUsageStr() {
	char szBuf[MAX_NUMBER_SIZE] = {'\0'};
	int64_t memoryKB = MemoryUsage() / 1024;
	snprintf(szBuf, sizeof(szBuf), SI64FMTD, memoryKB);
	return std::string(szBuf);
}
/// 获取当前进程虚拟内存使用量
std::string GetVirtualMemoryUsageStr() {
	char szBuf[MAX_NUMBER_SIZE] = {'\0'};
	int64_t memoryKB = VirtualMemoryUsage() / 1024;
	snprintf(szBuf, sizeof(szBuf), SI64FMTD, memoryKB);
	return std::string(szBuf);
}
/// 获取当前进程总共读IO的字节数
std::string GetIOReadKBStr() {
	char szBuf[MAX_NUMBER_SIZE] = {'\0'};
	int64_t ioReadKB = IoReadBytes() / 1024;
	snprintf(szBuf, sizeof(szBuf), SI64FMTD, ioReadKB);
	return std::string(szBuf);
}
/// 获取当前进程总共写IO的字节数
std::string GetIOWriteKBStr() {
	char szBuf[MAX_NUMBER_SIZE] = {'\0'};
	int64_t ioWriteKB = IoWriteBytes() / 1024;
	snprintf(szBuf, sizeof(szBuf), SI64FMTD, ioWriteKB);
	return std::string(szBuf);
}

}
