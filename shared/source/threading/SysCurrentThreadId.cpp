#include "Common.h"
#include "SysCurrentThreadId.h"

#if COMPILER != COMPILER_MICROSOFT && PLATFORM != PLATFORM_APPLE
#ifdef USE_GETTID
#include <unistd.h>
#include <sys/syscall.h>
#endif
#endif


uint32_t GetSysCurrentThreadId(void)
{
#if COMPILER == COMPILER_MICROSOFT
	BUILD_BUG_ON(sizeof(uint32_t) < sizeof(DWORD));
	return (uint32_t)GetCurrentThreadId();
#elif PLATFORM == PLATFORM_APPLE
	BUILD_BUG_ON(sizeof(uint32_t) < sizeof(mach_port_t));
	return (uint32_t)pthread_mach_thread_np(pthread_self());
#else
#ifdef USE_GETTID
	#ifdef SYS_gettid
		BUILD_BUG_ON(sizeof(uint32_t) < sizeof(pid_t));
		return (uint32_t)syscall(SYS_gettid);
	#else
		#error "SYS_gettid unavailable on this system"
	#endif
#else
	BUILD_BUG_ON(sizeof(uint32_t) < sizeof(pthread_t));
	return (uint32_t)pthread_self();
#endif
#endif
}



