
#ifndef ATOMIC_CAS_H
#define ATOMIC_CAS_H

#include <stdint.h>

#ifdef WINVER
#include <Windows.h>
#elif defined(_MSC_VER)
#include <intrin.h>
#endif

#ifdef _WIN64
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#endif

#ifdef __GNUC__
#define CAS CAS_assembly
#define CAS2 CAS2_assembly
#else

#ifdef WINVER
#define CAS CAS_windows
#define CAS2 CAS2_windows
#elif defined(_MSC_VER)
#define CAS CAS_intrinsic
#define CAS2 CAS2_intrinsic
#else
#define CAS CAS_assembly
#define CAS2 CAS2_assembly
#endif

#endif


// CAS will assume a multi-processor machine (versus multithread on a single processor).
// On a single processor machine, it might not make sense to spin on a CAS because
// if it fails, there is no way to succeed until the another thread runs (which will be
// on the same processor).  In these cases, if CAS is required to succeed, it might make
// more sense to yield the processor to the other thread via Sleep(1).

// Since we are assuming a multi-processor machine, we will need to use 'lock cmpxchg'
// instead of just cmpxchg, as the bus needs to be locked to synchronize the processors
// access to memory.

//
// Define a version of CAS which uses x86 assembly primitives.
//
template<typename T>
bool CAS_assembly(T * volatile * _ptr, T * oldVal, T * newVal)
{
    bool f = false;

#ifdef __GNUC__
#if __WORDSIZE == 64
	__asm__ __volatile__(
		"lock; cmpxchg8b %1;"
		"setz %0;"
		: "=r"(f), "=m"(*(_ptr))
		: "a" ((uintptr_t)oldVal & 0xFFFFFFFF),
		  "b" ((uintptr_t)newVal & 0xFFFFFFFF),
		  "c" ((uintptr_t)newVal >> 32),
		  "d" ((uintptr_t)oldVal >> 32)
		: "memory");
#else
    __asm__ __volatile__(
        "lock; cmpxchgl %%ebx, %1;"
        "setz %0;"
            : "=r"(f), "=m"(*(_ptr))
            : "a"(oldVal), "b" (newVal)
            : "memory");
#endif // __WORDSIZE == 64
#else
#ifdef _WIN64
	_asm
	{
		lea esi,oldVal;
		lea edi,newVal;
		mov eax,[esi];
		mov edx,4[esi];
		mov ebx,[edi];
		mov ecx,4[edi];
		mov esi,_ptr;
		lock cmpxchg8b [esi];
		setz f
	}
#else
    _asm
    {
        mov ecx,_ptr
        mov eax,oldVal
        mov ebx,newVal
        lock cmpxchg [ecx],ebx
        setz f
    }
#endif // _WIN64
#endif // __GNUC__

    return f;
}

//
// Define a version of CAS which uses the Visual C++ InterlockedCompareExchange intrinsic.
//
#ifdef _MSC_VER

// Define intrinsic for InterlockedCompareExchange
extern "C" long __cdecl _InterlockedCompareExchange(long volatile * Dest, long Exchange, long Comp);
#if defined(BOOST_MSVC)
#pragma intrinsic (_InterlockedCompareExchange)
#endif // BOOST_MSVC

#ifdef _WIN64
// Define intrinsic for InterlockedCompareExchange64
extern "C" __int64 __cdecl _InterlockedCompareExchange64(__int64 volatile * Destination, __int64 Exchange, __int64 Comperand);
#if defined(BOOST_MSVC)
#pragma intrinsic (_InterlockedCompareExchange64)
#endif // BOOST_MSVC
#endif


template<typename T>
bool CAS_intrinsic(T * volatile * _ptr, T * oldVal, T * newVal)
{
#ifdef _WIN64
	return _InterlockedCompareExchange64(reinterpret_cast<__int64 volatile *>(_ptr), reinterpret_cast<__int64>(newVal), reinterpret_cast<__int64>(oldVal)) == reinterpret_cast<__int64>(oldVal);
#else
    return _InterlockedCompareExchange(reinterpret_cast<long volatile *>(_ptr), reinterpret_cast<long>(newVal), reinterpret_cast<long>(oldVal)) == reinterpret_cast<long>(oldVal);
#endif // _WIN64
}

#endif  // _MSC_VER

//
// Define a version of CAS which uses the Windows API InterlockedCompareExchange.
//
#ifdef WINVER
template<typename T>
bool CAS_windows(T * volatile * _ptr, T * oldVal, T * newVal)
{
#ifdef _WIN64
#if WINVER >= 0x0600
	return InterlockedCompareExchange64(reinterpret_cast<__int64 volatile *>(_ptr), reinterpret_cast<__int64>(newVal), reinterpret_cast<__int64>(oldVal)) == reinterpret_cast<__int64>(oldVal);
#endif // WINVER >= 0x0600
#else 
    return InterlockedCompareExchange(reinterpret_cast<long volatile *>(_ptr), reinterpret_cast<long>(newVal), reinterpret_cast<long>(oldVal)) == reinterpret_cast<long>(oldVal);
#endif // _WIN64
}
#endif //WINVER


// Define a version of CAS2 which uses x86 assembly primitives.
//
template<typename T>
bool CAS2_assembly(T * volatile * _ptr, T * old1, uintptr_t old2, T * new1, uintptr_t new2)
{
    bool f = false;
#ifdef __GNUC__
#if __WORDSIZE == 64
	__asm__ __volatile__(
		"lock; cmpxchg16b %1;"
		"setz %0;"
		: "=r"(f), "=m"(*(_ptr))
		: "a"(old1), "b" (new1), "c" (new2), "d" (old2)
		: "memory");
#else
    __asm__ __volatile__(
        "lock; cmpxchg8b %1;"
        "setz %0;"
            : "=r"(f), "=m"(*(_ptr))
            : "a"(old1), "b" (new1), "c" (new2), "d" (old2)
            : "memory");
#endif // __WORDSIZE == 64
#else
#ifdef _WIN64
	_asm
	{
		mov rsi,_ptr;
		mov rax,old1
		mov rdx,old2
		mov rbx,new1
		mov rcx,new2
		lock cmpxchg16b [rsi]
		setz f
	}
#else
    _asm
    {
        mov esi,_ptr
        mov eax,old1
        mov edx,old2
        mov ebx,new1
        mov ecx,new2
        lock cmpxchg8b [esi]
        setz f
    }
#endif // _WIN64
#endif // __GNUC__
    return f;
}

//
// Define a version of CAS2 which uses the Visual C++ InterlockedCompareExchange64 intrinsic.
//
#ifdef _MSC_VER

#if(_MSC_VER >= 1500)
extern "C" unsigned char __cdecl _InterlockedCompareExchange128(__int64 volatile * Destination, __int64 ExchangeHigh, __int64 ExchangeLow, __int64 * ComparandResult);
#if defined(BOOST_MSVC)
#pragma intrinsic (_InterlockedCompareExchange128)
#endif
#endif

template<typename T>
bool CAS2_intrinsic(T * volatile * _ptr, T * old1, uintptr_t old2, T * new1, uintptr_t new2)
{
#ifdef _WIN64
#if(_MSC_VER >= 1500)
	__int64 Comperand[2] = { reinterpret_cast<__int64>(old1) , static_cast<__int64>(old2) };
	return _InterlockedCompareExchange128(reinterpret_cast<__int64 volatile *>(_ptr), new2, (__int64)new1, Comperand) == (unsigned char)TRUE;
#endif // _MSC_VER >= 1500
#else
    __int64 Comperand = reinterpret_cast<__int32>(old1) | (static_cast<__int64>(old2) << 32);
    __int64 Exchange  = reinterpret_cast<__int32>(new1) | (static_cast<__int64>(new2) << 32);

    return _InterlockedCompareExchange64(reinterpret_cast<__int64 volatile *>(_ptr), Exchange, Comperand) == Comperand;
#endif // _WIN64
}

#endif  // _MSC_VER

//
// Define a version of CAS2 which uses the Windows API InterlockedCompareExchange64.
// InterlockedCompareExchange64 requires Windows Vista
//
#if WINVER >= 0x0600

template<typename T>
bool CAS2_windows(T * volatile * _ptr, T * old1, uintptr_t old2, T * new1, uintptr_t new2)
{
#ifdef _WIN64
#if WINVER >= 0x0602
	__int64 Comperand[2] = { reinterpret_cast<__int64>(old1) , static_cast<__int64>(old2) };
	return InterlockedCompareExchange128(reinterpret_cast<__int64 volatile *>(_ptr), new2, reinterpret_cast<__int64>(new1), Comperand) == static_cast<unsigned char>(TRUE);
#endif  // WINVER >= 0x0602
#else
    __int64 Comperand = reinterpret_cast<__int32>(old1) | (static_cast<__int64>(old2) << 32);
    __int64 Exchange  = reinterpret_cast<__int32>(new1) | (static_cast<__int64>(new2) << 32);

    return InterlockedCompareExchange64(reinterpret_cast<__int64 volatile *>(_ptr), Exchange, Comperand) == Comperand;
#endif // _WIN64
}
#endif  // WINVER >= 0x0600

#endif // ATOMIC_CAS_H

