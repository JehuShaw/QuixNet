/*! @file       Matilda.cpp
    @version    2.0
    @brief      Internal wrapper
 */

#ifdef _WIN32
# include <windows.h>
#else
# include <time.h>
#endif

#include "Matilda.h"

namespace xplatform {
    unsigned long GetCurrentTickCount()
    {
#ifdef _WIN32
        return ::GetTickCount();
#else
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (unsigned long) (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif
    }
}
