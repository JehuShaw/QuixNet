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
				snprintf(szTemp,sizeof(szTemp) - 1,"%s [NetworkTrace.cpp]\n", message);
				szTemp[1023] = '\0';
				
				vsnprintf(szBuff, sizeof(szBuff) - 1, szTemp, args);
				szBuff[1023] = '\0';
			}
            (*g_nwTraceMethod)(szBuff);
            va_end(args);
        }
    }
}

