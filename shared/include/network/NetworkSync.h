/* 
 * File:   NetworkSync.h
 * Author: Jehu Shaw
 *
 * Created on 2011Âπ?Êú?7Êó? ‰∏ãÂçà4:21
 */

#ifndef NETWORKSYNC_H
#define	NETWORKSYNC_H

namespace ntwk
{
#if defined(_WIN64) || defined(_WIN32)

#include<WinBase.h>

    class Mutex {
        CRITICAL_SECTION cs;
      public:
        Mutex() {
            InitializeCriticalSection(&cs);
        }
        ~Mutex() {
            DeleteCriticalSection(&cs);
        }
        void Lock() {
            EnterCriticalSection(&cs);
        }
        void Unlock() {
            LeaveCriticalSection(&cs);
        }
    };
#else

#include <assert.h>
#include <stddef.h>
    class Mutex {
            pthread_mutex_t cs;
      public:
        Mutex() {
#ifdef _RELEASE
            pthread_mutex_init(&cs, NULL);
    #else
    #ifdef VXWORKS
            memset(&cs, '\0', sizeof(cs));
    #endif // VXWORKS
            int rc = pthread_mutex_init(&cs, NULL);
            assert(rc == 0);
#endif

        }
        ~Mutex() {
#ifdef _RELEASE
            pthread_mutex_destroy(&cs);
    #else
            int rc = pthread_mutex_destroy(&cs);
            assert(rc == 0);
#endif

        }

        void Lock() {

#ifdef _RELEASE
            pthread_mutex_lock(&cs);
#else
            int rc = pthread_mutex_lock(&cs);
            assert(rc == 0);
#endif

        }
        void Unlock() {
#ifdef _RELEASE
            pthread_mutex_unlock(&cs);
#else
            int rc = pthread_mutex_unlock(&cs);
            assert(rc == 0);
#endif
        }
    };
#endif
    // Common decls for all platforms
    class CriticalSection {
      private:
        Mutex& mutex;
      public:
        CriticalSection(Mutex& guard) : mutex(guard) {
            mutex.Lock();
        }
        ~CriticalSection() {
            mutex.Unlock();
        }
    };

}
#endif	/* NETWORKSYNC_H */

