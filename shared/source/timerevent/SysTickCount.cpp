#include "Common.h"
#if COMPILER == COMPILER_MICROSOFT
#include <windows.h>
#include "SysTickCount.h"
#include "AtomicLock.h"


typedef ULONGLONG (WINAPI *GETTICKCOUNT64)(void);
#ifdef _UNICODE
const static GETTICKCOUNT64 pGetTickCount64 = (GETTICKCOUNT64)GetProcAddress(GetModuleHandle(L"Kernel32.dll"), "GetTickCount64");
#else
const static GETTICKCOUNT64 pGetTickCount64 = (GETTICKCOUNT64)GetProcAddress(GetModuleHandle("Kernel32.dll"), "GetTickCount64");
#endif

uint64_t GetSysTickCount(void) {
	BUILD_BUG_ON(sizeof(uint64_t) < sizeof(ULONGLONG));
	if(pGetTickCount64)
	{
		return pGetTickCount64();
	}
	else
	{
		static volatile LONGLONG s_u64Count = 0;
		LONGLONG curCount1, curCount2;
		LONGLONG tmp;

		curCount1 = atomic_cmpxchg64(&s_u64Count, 0, 0);
		curCount2 = curCount1 & 0xFFFFFFFF00000000;
		curCount2 |= GetTickCount();

		if((ULONG)curCount2 < (ULONG)curCount1) {
			curCount2 += 0x100000000;
		}

		tmp = atomic_cmpxchg64(&s_u64Count, curCount2, curCount1);
		if(tmp == curCount1) {
			return curCount2;
		}
		else {
			return tmp;
		}
	}
}

#else
#include <inttypes.h>
#include "SysTickCount.h"
#include <time.h>

uint64_t GetSysTickCount(void) {

	//#if defined ( __GNUC__ )
	//	unsigned long long ret;
	//	__asm__ __volatile__ ("int $0x2a" : "=A"(ret) : : );
	//	return ret;
	//#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
	//#endif
}
#endif

