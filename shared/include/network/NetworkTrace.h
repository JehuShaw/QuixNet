/* 
 * File:   NetworkTrace.h
 * Author: Jehu Shaw
 *
 * Created on March 22, 2011, 4:43 PM
 */

#ifndef NETWORKTRACE_H
#define	NETWORKTRACE_H

#include "Common.h"

namespace ntwk
{
    // User defined trace functions
    typedef void (*NWTraceMethodPtr)(const char* message);

    // Pointer to trace function (default implementation just prints message to stderr)
    extern SHARED_DLL_DECL NWTraceMethodPtr g_nwTraceMethod;
    // Set trace enable
    extern SHARED_DLL_DECL bool g_nwTraceEnable;

    extern SHARED_DLL_DECL void NWTrace(const char* message, ...);

	#ifdef DEBUG
    #define TRACE_MSG NWTrace
    #else
    #define TRACE_MSG(fmt, ...)
    #endif
}
#endif	/* NETWORKTRACE_H */

