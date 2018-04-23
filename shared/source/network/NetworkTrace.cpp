/*
 * File:   NetworkTrace.cpp
 * Author: Jehu Shaw
 *
 * Created on March 22, 2011, 4:43 PM
 */

#include "NetworkTrace.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

namespace ntwk
{
    static void stderrTrace(const char* msg)
    {
        fputs(msg, stderr);
        fflush(stderr);
    }

    NWTraceMethodPtr g_nwTraceMethod = stderrTrace;
    bool g_nwTraceEnable = true;

    void NWTrace(const char* message, ...)
    {
        if (g_nwTraceEnable) {
            va_list args;
            va_start (args, message);
			char szBuff[1024];
			{
				char szTemp[1024];
				snprintf(szTemp,sizeof(szTemp)- 256,"%s [file:%s, function:%s, line:%ld]\n"
					, message, __FILE__, __FUNCTION__, (long)__LINE__);
				
				vsnprintf(szBuff, sizeof(szBuff)-1, szTemp, args);
			}
            (*g_nwTraceMethod)(szBuff);
            va_end(args);
        }
    }
}

