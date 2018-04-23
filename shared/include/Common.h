/*
 * File:   Common.h
 * Author: Jehu Shaw
 *
 */

#ifndef _COMMON_H
#define _COMMON_H

// Do you want throw a bug
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))

#ifdef WIN32
#pragma warning(disable:4996)
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif
#define _CRT_SECURE_COPP_OVERLOAD_STANDARD_NAMES 1
#pragma warning(disable:4251)		// dll-interface
#endif


#ifdef WIN32
#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif
#else
#define FORCEINLINE inline
#endif
#define INLINE inline

#include "ShareConfig.h"
#include "ShareDll.h"


#include <cstdlib>
#include <cstdio>

#include <cstdarg>
#include <ctime>
#include <cmath>
#include <cerrno>

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#else
#  include <cstring>
#  define MAX_PATH 1024
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef CONFIG_USE_SELECT
#undef FD_SETSIZE
#define FD_SETSIZE 2048
#endif

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#endif

// current platform and compiler
#define PLATFORM_WIN32 0
#define PLATFORM_UNIX  1
#define PLATFORM_APPLE 2
#define PLATFORM_BSD 3

#define UNIX_FLAVOUR_LINUX 1
#define UNIX_FLAVOUR_BSD 2
#define UNIX_FLAVOUR_OTHER 3
#define UNIX_FLAVOUR_OSX 4

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  define PLATFORM PLATFORM_WIN32
#elif defined( __APPLE_CC__ ) || defined( __APPLE__ )
#  define PLATFORM PLATFORM_APPLE
#elif defined(__FreeBSD__) || defined( __OpenBSD__ )
#  define PLATFORM PLATFORM_BSD
#else
#  define PLATFORM PLATFORM_UNIX
#endif

#define COMPILER_MICROSOFT 0
#define COMPILER_GNU	   1
#define COMPILER_BORLAND   2

#ifdef _MSC_VER
#  define COMPILER COMPILER_MICROSOFT
#elif defined( __BORLANDC__ )
#  define COMPILER COMPILER_BORLAND
#elif defined( __GNUC__ )
#  define COMPILER COMPILER_GNU
#else
#  pragma error "FATAL ERROR: Unknown compiler."
#endif

#if PLATFORM == PLATFORM_UNIX || PLATFORM == PLATFORM_APPLE
#ifdef HAVE_DARWIN
#define PLATFORM_TEXT "MacOSX"
#define UNIX_FLAVOUR UNIX_FLAVOUR_OSX
#else
#ifdef USE_KQUEUE
#define PLATFORM_TEXT "FreeBSD"
#define UNIX_FLAVOUR UNIX_FLAVOUR_BSD
#else
#define PLATFORM_TEXT "Linux"
#define UNIX_FLAVOUR UNIX_FLAVOUR_LINUX
#endif
#endif
#endif

#if PLATFORM == PLATFORM_WIN32
#define PLATFORM_TEXT "Win32"
#endif

#ifdef _DEBUG
#define CONFIG "Debug"
#else
#define CONFIG "Release"
#endif

#ifdef X64
    #define ARCH "X64"
#else
    #define ARCH "X86"
#endif


#if PLATFORM == PLATFORM_WIN32
    #define STRCASECMP stricmp
#else
    #define STRCASECMP strcasecmp
#endif

#if PLATFORM == PLATFORM_WIN32
	#define ASYNC_NET
#endif

#ifdef USE_EPOLL
	#define CONFIG_USE_EPOLL
#endif
#ifdef USE_KQUEUE
	#define CONFIG_USE_KQUEUE
#endif
#ifdef USE_SELECT
	#define CONFIG_USE_SELECT
#endif
#ifdef USE_POLL
	#define CONFIG_USE_POLL
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <cstdlib>
#include <set>
#include <list>
#include <string>
#include <map>
#include <queue>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <climits>
#include <cstdlib>

#if defined ( __GNUC__ )
#	define LIKELY( _x ) \
		__builtin_expect( ( _x ), 1 )
#	define UNLIKELY( _x ) \
 		__builtin_expect( ( _x ), 0 )
#else
#	define LIKELY( _x ) \
		_x
#	define UNLIKELY( _x ) \
		_x
#endif

#if defined (__GNUC__)
#  define GCC_VERSION (__GNUC__ * 10000 \
					   + __GNUC_MINOR__ * 100 \
					   + __GNUC_PATCHLEVEL__)
#endif


#ifndef WIN32
#ifndef X64
#  if defined (__GNUC__)
#	if GCC_VERSION >= 30400
#         ifdef HAVE_DARWIN
#	      define __fastcall
#         else
#    	      define __fastcall __attribute__((__fastcall__))
#         endif
#	else
#	  define __fastcall __attribute__((__regparm__(3)))
#	endif
#  else
#	define __fastcall __attribute__((__fastcall__))
#  endif
#else
#define __fastcall
#endif
#endif

#ifdef HAVE_STDCXX_0X
#include <unordered_map>
#include <unordered_set>
#elif COMPILER == COMPILER_GNU && __GNUC__ >= 3
#include <ext/hash_map>
#include <ext/hash_set>
#else
#include <hash_map>
#include <hash_set>
#endif



#ifdef _STLPORT_VERSION
#define HM_NAMESPACE std
using std::hash_map;
using std::hash_set;
#elif COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1300
#define HM_NAMESPACE stdext
using stdext::hash_map;
using stdext::hash_set;
#define ENABLE_SHITTY_STL_HACKS 1

// hacky stuff for vc++
#define snprintf _snprintf
#define vsnprintf _vsnprintf
//#define strlen lstrlen
#define localtime_r(_Time, _Tm) localtime_s(_Tm, _Time)
inline static struct tm* gmtime_r(const time_t* _Time, struct tm* _Tm) {
	if(gmtime_s(_Tm, _Time) == 0) {
		return _Tm;
	}
	return NULL;
}

#elif COMPILER == COMPILER_INTEL
#define HM_NAMESPACE std
using std::hash_map;
using std::hash_set;
#elif defined(HAVE_STDCXX_0X)
#define HM_NAMESPACE std
#define hash_map unordered_map
#define hash_set unordered_set
using std::unordered_map;
using std::unordered_set;
#elif COMPILER == COMPILER_GNU && __GNUC__ >= 3
#define HM_NAMESPACE __gnu_cxx
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_set;

namespace __gnu_cxx
{
	template<> struct hash<unsigned long long>
	{
		size_t operator()(const unsigned long long &__x) const { return (size_t)__x; }
	};
	template<typename T> struct hash<T *>
	{
		size_t operator()(T * const &__x) const { return (size_t)__x; }
	};

};

#else
#define HM_NAMESPACE std
using std::hash_map;
#endif

/* Use correct types for x64 platforms, too */
#if COMPILER != COMPILER_GNU
typedef signed __int64 int64_t;
typedef signed __int32 int32_t;
typedef signed __int16 int16_t;
typedef signed __int8 int8_t;
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;

#endif

/*
id type
*/

typedef union id32_t
{
	uint32_t u32;
	struct half
	{
		uint16_t u16_l;
		uint16_t u16_h;
	}h;

	id32_t():u32(0) {}
	id32_t(const id32_t& orig):u32(orig.u32){}
	id32_t(uint64_t value):u32((uint32_t)value){}
	id32_t(int64_t value):u32((uint32_t)value){}
	id32_t(uint32_t value):u32(value){}
	id32_t(int32_t value):u32((uint32_t)value){}
	id32_t(uint16_t value) { h.u16_l = value; h.u16_h = 0; }
	id32_t(int16_t value) { h.u16_l = value; h.u16_h = 0; }
	id32_t(uint8_t value) { h.u16_l = value; h.u16_h = 0; }
	id32_t(int8_t value) { h.u16_l = value; h.u16_h = 0; }

	id32_t(uint16_t low, uint16_t high) { h.u16_l = low; h.u16_h = high; }

	id32_t& operator = (const id32_t& orig) {
		this->u32 = orig.u32;
		return *this;
	}
	id32_t& operator = (uint64_t value) {
		this->u32 = (uint32_t)value;
		return *this;
	}
	id32_t& operator = (uint32_t value) {
		this->u32 = value;
		return *this;
	}
	id32_t& operator = (uint16_t value) {
		this->h.u16_l = value;
		this->h.u16_h = 0;
		return *this;
	}
	id32_t& operator = (uint8_t value) {
		this->h.u16_l = value;
		this->h.u16_h = 0;
		return *this;
	}
	id32_t& operator = (int64_t value) {
		this->u32 = (uint32_t)value;
		return *this;
	}
	id32_t& operator = (int32_t value) {
		this->u32 = (uint32_t)value;
		return *this;
	}
	id32_t& operator = (int16_t value) {
		this->h.u16_l = (uint16_t)value;
		this->h.u16_h = 0;
		return *this;
	}
	id32_t& operator = (int8_t value) {
		this->h.u16_l = value;
		this->h.u16_h = 0;
		return *this;
	}
}id32_t;

typedef union id64_t
{
	uint64_t u64;
	struct half
	{
		uint32_t u32_l;
		uint32_t u32_h;
	}h;

	id64_t():u64(0) {}
	id64_t(const id64_t& orig):u64(orig.u64){}
	id64_t(uint64_t value):u64(value){}
	id64_t(int64_t value):u64(value){}
	id64_t(uint32_t value) { h.u32_l = value; h.u32_h = 0; }
	id64_t(int32_t value) { h.u32_l = (uint32_t)value; h.u32_h = 0; }
	id64_t(uint16_t value) { h.u32_l = value; h.u32_h = 0; }
	id64_t(int16_t value) { h.u32_l = value; h.u32_h = 0; }
	id64_t(uint8_t value) { h.u32_l = value; h.u32_h = 0; }
	id64_t(int8_t value) { h.u32_l = value; h.u32_h = 0; }
	id64_t(uint32_t low, uint32_t high) { h.u32_l = low; h.u32_h = high; }

	id64_t& operator = (const id64_t& orig) {
		this->u64 = orig.u64;
		return *this;
	}
	id64_t& operator = (uint64_t value) {
		this->u64 = value;
		return *this;
	}
	id64_t& operator = (uint32_t value) {
		this->h.u32_l = value;
		this->h.u32_h = 0;
		return *this;
	}
	id64_t& operator = (uint16_t value) {
		this->h.u32_l = value;
		this->h.u32_h = 0;
		return *this;
	}
	id64_t& operator = (uint8_t value) {
		this->h.u32_l = value;
		this->h.u32_h = 0;
		return *this;
	}
	id64_t& operator = (int64_t value) {
		this->u64 = (uint64_t)value;
		return *this;
	}
	id64_t& operator = (int32_t value) {
		this->h.u32_l = (uint32_t)value;
		this->h.u32_h = 0;
		return *this;
	}
	id64_t& operator = (int16_t value) {
		this->h.u32_l = value;
		this->h.u32_h = 0;
		return *this;
	}
	id64_t& operator = (int8_t value) {
		this->h.u32_l = value;
		this->h.u32_h = 0;
		return *this;
	}
}id64_t;

// Include all threading files
#include <cassert>
#include "threading/Threading.h"

#if COMPILER == COMPILER_MICROSOFT

#define I64FMT "%016I64X"
#define I64FMTD "%I64u"
#define SI64FMTD "%I64d"
#define snprintf _snprintf
#define atoll _atoi64
#define strcasecmp _stricmp

#define strtoll _strtoi64
#define strtoull _strtoui64
#else

#define stricmp strcasecmp
#define strnicmp strncasecmp
#define I64FMT "%016llX"
#define I64FMTD "%llu"
#define SI64FMTD "%lld"

#endif

#define atol(a) strtol(a, NULL, 10)

#ifdef __cplusplus
extern "C" SHARED_DLL_DECL int ltostr(char *buf, long val, int base, int uppercase);
extern "C" SHARED_DLL_DECL int lltostr(char *buf, long long val, int base, int uppercase);
extern "C" SHARED_DLL_DECL int ultostr(char *buf, unsigned long uval, int base, int uppercase);
extern "C" SHARED_DLL_DECL int ulltostr(char *buf, unsigned long long uval, int base, int uppercase);

#else
extern SHARED_DLL_DECL int ltostr(char *buf, long val, int base, int uppercase);
extern SHARED_DLL_DECL int lltostr(char *buf, long long val, int base, int uppercase);
extern SHARED_DLL_DECL int ultostr(char *buf, unsigned long uval, int base, int uppercase);
extern SHARED_DLL_DECL int ulltostr(char *buf, unsigned long long uval, int base, int uppercase);

#endif

#define STRINGIZE(a) #a

// fix buggy MSVC's for variable scoping to be reliable =S
#define for if(true) for

#if COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1400
#pragma float_control(push)
#pragma float_control(precise, on)
#endif

// fast int abs
INLINE static int32_t int32abs( const int32_t value )
{
	return (value ^ (value >> 31)) - (value >> 31);
}

// fast int abs and recast to unsigned
INLINE static uint32_t int32abs2uint32( const int32_t value )
{
	return (uint32_t)(value ^ (value >> 31)) - (value >> 31);
}

/// Fastest Method of float2int32
INLINE static int32_t float2int32(const float value)
{
#if !defined(X64) && COMPILER == COMPILER_MICROSOFT && !defined(USING_BIG_ENDIAN)
	int32_t i;
	__asm {
		fld value
		frndint
		fistp i
	}
	return i;
#else
	union { int32_t asInt[2]; double asDouble; } n;
	n.asDouble = value + 6755399441055744.0;

	return n.asInt [0];
#endif
}

/// Fastest Method of double2int32
INLINE static int32_t double2int32(const double value)
{
#if !defined(X64) && COMPILER == COMPILER_MICROSOFT && !defined(USING_BIG_ENDIAN)
	int32_t i;
	__asm {
		fld value
		frndint
		fistp i
	}
	return i;
#else
  union { int32_t asInt[2]; double asDouble; } n;
  n.asDouble = value + 6755399441055744.0;

  return n.asInt [0];
#endif
}

#if COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1400
#pragma float_control(pop)
#endif


#include <sys/timeb.h>

INLINE static time_t CurrentTime()
{
#if COMPILER == COMPILER_MICROSOFT
	struct _timeb tb;
	::_ftime(&tb);
	return(static_cast<time_t>(tb.time));
#else
	struct timeval tv;
	struct timezone tz;
	::gettimeofday(&tv, &tz);
	return(static_cast<time_t>(tv.tv_sec));
#endif
}

INLINE static int64_t CurrentTimeMillis()
{
#if COMPILER == COMPILER_MICROSOFT
	struct _timeb tb;
	::_ftime(&tb);
	return((static_cast<int64_t>(tb.time) * 1000)
		+ static_cast<int64_t>(tb.millitm));
#else
	struct timeval tv;
	struct timezone tz;
	::gettimeofday(&tv, &tz);
	return((static_cast<int64_t>(tv.tv_sec) * 1000)
		+ static_cast<int64_t>(tv.tv_usec / 1000));
#endif
}

#ifndef WIN32
#define FALSE   0
#define TRUE	1
#endif

#ifndef WIN32
#define Sleep(ms) usleep(1000*ms)
#endif


INLINE static void StrToLower(std::string& str)
{
	for(size_t i = 0; i < str.length(); ++i) {
		str[i] = (char)tolower(str[i]);
	}
}

INLINE static void StrToUpper(std::string& str)
{
	for(size_t i = 0; i < str.length(); ++i) {
		str[i] = (char)toupper(str[i]);
	}
}

// returns true if the ip hits the mask, otherwise false
INLINE static bool ParseCIDRBan(unsigned int IP, unsigned int Mask, unsigned int MaskBits)
{
	// CIDR bans are a compacted form of IP / Submask
	// So 192.168.1.0/255.255.255.0 would be 192.168.1.0/24
	// IP's in the 192.168l.1.x range would be hit, others not.
	unsigned char * source_ip = (unsigned char*)&IP;
	unsigned char * mask = (unsigned char*)&Mask;
	int full_bytes = MaskBits / 8;
	int leftover_bits = MaskBits % 8;
	//int byte;

	// sanity checks for the data first
	if( MaskBits > 32 )
		return false;

	// this is the table for comparing leftover bits
	static const unsigned char leftover_bits_compare[9] = {
		0x00,			// 00000000
		0x80,			// 10000000
		0xC0,			// 11000000
		0xE0,			// 11100000
		0xF0,			// 11110000
		0xF8,			// 11111000
		0xFC,			// 11111100
		0xFE,			// 11111110
		0xFF,			// 11111111 - This one isn't used
	};

	// if we have any full bytes, compare them with memcpy
	if( full_bytes > 0 )
	{
		if( memcmp( source_ip, mask, full_bytes ) != 0 )
			return false;
	}

	// compare the left over bits
	if( leftover_bits > 0 )
	{
		if( ( source_ip[full_bytes] & leftover_bits_compare[leftover_bits] ) !=
			( mask[full_bytes] & leftover_bits_compare[leftover_bits] ) )
		{
			// one of the bits does not match
			return false;
		}
	}

	// all of the bits match that were testable
	return true;
}

INLINE static unsigned int MakeIP(const char * str)
{
	unsigned int bytes[4];
	unsigned int res;
	if(sscanf(str, "%u.%u.%u.%u", &bytes[0], &bytes[1], &bytes[2], &bytes[3]) != 4)
		return 0;

	res = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
	return res;
}

extern SHARED_DLL_DECL uint64_t AddressToInteger(const char * str);

extern SHARED_DLL_DECL bool ReviseAddress(std::string& outStr);

// make float precision
INLINE static double Precision(double v, unsigned char c) {
    char buf[150];
    char format[10];
    sprintf(format,"%%.%dlf",c);
    sprintf(buf,format,v);
    return atof(buf);
}

template <class T>
std::basic_string<T>& TrimString(std::basic_string<T>& s)
{
    if (s.empty()) {
        return s;
    }

    typename std::basic_string<T>::iterator c;
    // Erase whitespace before the string
    for (c = s.begin(); c != s.end() && iswspace(*c++););
    s.erase(s.begin(), --c);

    // Erase whitespace after the string
    for (c = s.end(); c != s.begin() && iswspace(*--c););
    s.erase(++c, s.end());

    return s;
}

template <class T>
std::basic_string<T>& TrimStringEx(std::basic_string<T>& s, T c)
{
	if (s.empty()) {
		return s;
	}

	typename std::basic_string<T>::iterator it;
	// Erase whitespace before the string
	for (it = s.begin(); it != s.end() && (*it++) == c;);
	s.erase(s.begin(), --it);

	// Erase whitespace after the string
	for (it = s.end(); it != s.begin() && (*--it) == c;);
	s.erase(++it, s.end());

	return s;
}

INLINE static int ParseInt(const char* szValue) {
	if(NULL == szValue) {
		return 0;
	}
	const char *p = szValue;
	while(*p != '\0') {
		if(*p > '0' && *p  < '9') {
			break;
		}
		p++;
	}
	return atoi(p);
}

INLINE static uint64_t CombineUserId(uint16_t serverId, uint32_t index) {
	uint64_t u64Field = 0;
	u64Field <<= 16;
	u64Field |= serverId;
	u64Field <<= 32;
	u64Field |= index;
	return u64Field;
}

INLINE static void DepartUserId(uint64_t userId, uint16_t& outServId, uint32_t& outIndex) {
	outIndex = userId & 0xFFFFFFFF;
	userId >>= 32;
	outServId = userId & 0xFFFF;
}

INLINE static uint16_t DepartUserId2ServId(uint64_t userId) {
	userId >>= 32;
	return userId & 0xFFFF;
}

INLINE static uint32_t DepartUserId2Index(uint64_t userId) {
	return userId & 0xFFFFFFFF;
}

#endif /*_SERVER_COMMON_H*/
