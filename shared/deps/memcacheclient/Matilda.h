/*! @file       Matilda.h
    @version    2.0
    @brief      Adapt internal files for external use
 */

#ifndef INCLUDED_Matilda
#define INCLUDED_Matilda

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// get rid of Windows/Linux inconsistencies
#ifdef _WIN32
# define snprintf   _snprintf
# define PRIu64     "I64u"
#else
# include <stdint.h>
# define PRIu64     "llu"
#endif

// use your preferred assertion library (e.g. boost)
#include <assert.h>
#define CR_ASSERT(x)    assert(x)

// used to export classes from DLLs on Windows
#define CROSSBASE_CLASS_API

// internal logging class, this will be optimized out in release builds
#define CLERROR 0
#define CLINFO  0
#define CLDEBUG 0
#define CLULTRA 0
struct ClTrace {
    inline ClTrace() { }
    inline ClTrace(const char *) { }
    inline ClTrace(const ClTrace &) { }
    inline void Trace(int,...) { }
    inline bool IsThisModuleTracing(int) { return false; }
};

// internal UTF-8/UTF-16 string classes redefined to STL
#define NarrowString    std::string
#define WideString      std::wstring

// internal cross platform library function
namespace xplatform {
    unsigned long GetCurrentTickCount();
}

// close a socket on exec() used on Linux
#define setCloseOnExec(x)   

#endif // INCLUDED_Matilda
