/*
 * File:   AtomicLock.h
 * Author: Jehu Shaw
 *
 * Created on 2013_10_1 10:32
 */

#ifndef ATOMICLOCK_H
#define	ATOMICLOCK_H

#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ ) || defined( _WIN64 )

//////////////////////////////////////////////////////////////////////////
#include <intrin.h>
//////////////////////////////////////////////////////////////////////////
extern "C" __declspec(dllimport) void __stdcall Sleep( unsigned long ms );
//////////////////////////////////////////////////////////////////////////
extern "C" long __cdecl _InterlockedExchange( long volatile *, long );
extern "C" long __cdecl _InterlockedExchangeAdd( long volatile *, long );
extern "C" long __cdecl _InterlockedIncrement( long volatile * );
extern "C" long __cdecl _InterlockedDecrement( long volatile * );
extern "C" long __cdecl _InterlockedCompareExchange( long volatile *, long, long );

extern "C" short __cdecl _InterlockedExchange16( short volatile *, short );
extern "C" short __cdecl _InterlockedExchangeAdd16( short volatile *, short );
extern "C" short __cdecl _InterlockedIncrement16( short volatile * );
extern "C" short __cdecl _InterlockedDecrement16( short volatile * );
extern "C" short __cdecl _InterlockedCompareExchange16( short volatile *, short, short );

extern "C" char __cdecl _InterlockedExchange8(char volatile *, char);
extern "C" char __cdecl _InterlockedExchangeAdd8( char volatile *, char );
extern "C" char __cdecl _InterlockedCompareExchange8(char volatile *, char, char);
# if defined(_M_IA64) || defined(_M_AMD64) || defined(__x86_64__) || defined(__x86_64)
extern "C" void* __cdecl _InterlockedExchangePointer( void* volatile *, void* );
#endif
extern "C" __int64 __cdecl _InterlockedCompareExchange64(__int64 volatile *, __int64, __int64);

//////////////////////////////////////////////////////////////////////////
#define atomic_xchg(P, V) _InterlockedExchange((long volatile *)(P), (long)(V))
#define atomic_xadd(P, V) _InterlockedExchangeAdd((long volatile *)(P), (long)(V))
#define atomic_cmpxchg(P, O, N) _InterlockedCompareExchange((long volatile *)(P), (long)(N), (long)(O))
#define atomic_inc(P) _InterlockedIncrement((long volatile *)(P))
#define atomic_dec(P) _InterlockedDecrement((long volatile *)(P))

#define atomic_xchg16(P, V) _InterlockedExchange16((short volatile *)(P), (short)(V))
#define atomic_xadd16(P, V) _InterlockedExchangeAdd16((short volatile *)(P), (short)(V))
#define atomic_cmpxchg16(P, O, N) _InterlockedCompareExchange16((short volatile *)(P), (short)(N), (short)(O))
#define atomic_inc16(P) _InterlockedIncrement16((short volatile *)(P))
#define atomic_dec16(P) _InterlockedDecrement16((short volatile *)(P))

#define atomic_xchg8(P, V) _InterlockedExchange8((char volatile *)(P), (char)(V))
#define atomic_xadd8(P, V) _InterlockedExchangeAdd8((char volatile *)(P), (char)(V))
#define atomic_cmpxchg8(P, O, N) _InterlockedCompareExchange8((char volatile *)(P), (char)(N), (char)(O))
#define atomic_inc8(P) _InterlockedExchangeAdd8((char volatile *)(P), 1)
#define atomic_dec8(P) _InterlockedExchangeAdd8((char volatile *)(P), -1)

#if defined(_M_IA64) || defined(_M_AMD64) || defined(__x86_64__) || defined(__x86_64)
#define atomic_xchgptr(P, V) _InterlockedExchangePointer((void* volatile *)(P), (void*) (V))
#else
#define atomic_xchgptr(P, V) _InterlockedExchange((long volatile *)(P), (long)(V))
#endif

#define atomic_cmpxchg64(P, O, N) _InterlockedCompareExchange64((long long volatile *)(P), (long long)(N), (long long)(O))

static __forceinline __int64 InterlockedIncre64(
    __int64 volatile *Addend)
{
    __int64 Old;

    do {
        Old = *Addend;
    } while (_InterlockedCompareExchange64(Addend,
        Old + 1,
        Old) != Old);

    return Old + 1;
}

#define atomic_inc64(P) InterlockedIncre64((__int64 volatile *)(P))

static __forceinline __int64 InterlockedXChg64(__int64 volatile *Target, __int64 Value) 
{
    __int64 Old;

    do {
        Old = *Target;
    } while (_InterlockedCompareExchange64(Target,
        Value,
        Old) != Old);

    return Old;
}

#define atomic_xchg64(P, V) InterlockedXChg64((__int64 volatile *)(P), (__int64)(V))

static __forceinline long InterlockedOrFetch (volatile long *lock, long value)
{
    long old, newvalue;
    do {
        old = *lock;
        newvalue = old | value;
    } while (_InterlockedCompareExchange(lock, newvalue, old) != old);

    return newvalue;
}
#define atomic_or_fetch(P, V) InterlockedOrFetch((volatile long *)(P), (long)(V))

static __forceinline long InterlockedAndFetch (volatile long *lock, long value)
{
    long old, newvalue;
    do {
        old = *lock;
        newvalue = old & value;
    } while (_InterlockedCompareExchange(lock, newvalue, old) != old);

    return newvalue;
}
#define atomic_and_fetch(P, V) InterlockedAndFetch((volatile long *)(P), (long)(V))

static __forceinline long InterlockedAddFetch (volatile long *lock, long value)
{
	long old, newvalue;
	do {
		old = *lock;
		newvalue = old + value;
	} while (_InterlockedCompareExchange(lock, newvalue, old) != old);

	return newvalue;
}
#define atomic_add_fetch(P,V) InterlockedAddFetch((volatile long *)(P), (long)(V))

static __forceinline long InterlockedSubFetch (volatile long *lock, long value)
{
	long old, newvalue;
	do {
		old = *lock;
		newvalue = old - value;
	} while (_InterlockedCompareExchange(lock, newvalue, old) != old);

	return newvalue;
}
#define atomic_sub_fetch(P,V) InterlockedSubFetch((volatile long *)(P), (long)(V))

//////////////////////////////////////////////////////////////////////////

#if defined( _MSC_VER ) && _MSC_VER >= 1310

extern "C" void _ReadWriteBarrier();
#pragma intrinsic( _ReadWriteBarrier )

#define memory_barrier() _ReadWriteBarrier()

#endif

#if defined(_WIN64)
#define smt_pause _mm_pause
#else
#define smt_pause() __asm { rep nop }
#endif


inline void cpu_relax( unsigned k )
{
    if( k < 4 )
    {
    }
	else if( k < 16)
	{
		smt_pause();
	}
    else if( k < 32 )
    {
        Sleep( 0 );
    }
    else
    {
        Sleep( 1 );
    }
}

#elif defined(__GNUC__)

#if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

# define ARM_BARRIER "dmb"

#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__)

# define ARM_BARRIER "mcr p15, 0, r0, c7, c10, 5"

#else

# define ARM_BARRIER ""

#endif

#define memory_barrier() __asm__ __volatile__( ARM_BARRIER : : : "memory" )

#define atomic_xchg __sync_lock_test_and_set
#define atomic_xadd __sync_fetch_and_add
#define atomic_cmpxchg(P, O, N) (__sync_val_compare_and_swap((P), (O), (N)))
#define atomic_inc(P) __sync_add_and_fetch((P), 1)
#define atomic_dec(P) __sync_sub_and_fetch((P), 1)

#define atomic_xchg16 __sync_lock_test_and_set
#define atomic_xadd16 __sync_fetch_and_add
#define atomic_cmpxchg16(P, O, N) (__sync_val_compare_and_swap((P), (O), (N)))
#define atomic_inc16(P) __sync_add_and_fetch((P), 1)
#define atomic_dec16(P) __sync_sub_and_fetch((P), 1)

#define atomic_xchg8 __sync_lock_test_and_set
#define atomic_xadd8 __sync_fetch_and_add
#define atomic_cmpxchg8(P, O, N) (__sync_val_compare_and_swap((P), (O), (N)))
#define atomic_inc8(P) __sync_add_and_fetch((P), 1)
#define atomic_dec8(P) __sync_sub_and_fetch((P), 1)

#define atomic_xchgptr __sync_lock_test_and_set
#define atomic_inc64(P) __sync_add_and_fetch((P), 1)
#define atomic_xchg64 __sync_lock_test_and_set
#define atomic_cmpxchg64(P, O, N) (__sync_val_compare_and_swap((P), (O), (N)))

#define atomic_or_fetch __sync_or_and_fetch
#define atomic_and_fetch __sync_and_and_fetch
#define atomic_sub_fetch __sync_sub_and_fetch
#define atomic_fetch_xor __sync_fetch_and_xor

#if defined( __APPLE__ ) || defined( __ANDROID__ ) || defined( ANDROID )
#define smt_pause() __asm__ __volatile__( "nop" : : : "memory" )
#else
#define smt_pause() __asm__ __volatile__( "rep; nop" : : : "memory" )
#endif

//#if defined(_POSIX_THREADS) && (_POSIX_THREADS+0 >= 0)

#include <sched.h>
#include <time.h>

inline void cpu_relax( unsigned k )
{
    if( k < 4 )
    {
    }
	else if(k < 16)
	{
		smt_pause();
	}
    else if( k < 32 || k & 1 )
    {
        sched_yield();
    }
    else
    {
        // g++ -Wextra warns on {} or {0}
        struct timespec rqtp = { 0, 0 };

        // POSIX says that timespec has tv_sec and tv_nsec
        // But it doesn't guarantee order or placement

        rqtp.tv_sec = 0;
        rqtp.tv_nsec = 1000;

        nanosleep( &rqtp, 0 );
    }
}
//#endif
#endif

#endif  // ATOMICLOCK_H
