/*
 * File:   ThreadController.h
 * Author: Jehu Shaw
 *
 */

#ifndef THREADCONTROLLER_H
#define THREADCONTROLLER_H

#include "Common.h"

namespace thd {

#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ ) || defined( _WIN64 )

	class SHARED_DLL_DECL ThreadController
	{
		HANDLE m_hThread;
		HANDLE m_hSema;
	public:
		ThreadController() : m_hThread(NULL), m_hSema(NULL) {}

		void Setup(HANDLE hThread)
		{
			m_hThread = hThread;
			m_hSema = CreateSemaphore(NULL, 0, 2147483647, NULL);
		}

		void Suspend()
		{
			// We can't be suspended by someone else. That is a big-no-no and will lead to crashes.
			ASSERT(GetCurrentThreadId() == GetThreadId(m_hThread));

			WaitForSingleObject(m_hSema, INFINITE);
		}

		bool Resume()
		{
			// This SHOULD be called by someone else.
			ASSERT(GetCurrentThreadId() != GetThreadId(m_hThread));

			return ReleaseSemaphore(m_hSema, 1, NULL) != FALSE;
		}

		void Join()
		{
			WaitForSingleObject(m_hThread, INFINITE);
		}

		uint32_t GetId()
		{
			return GetThreadId(m_hThread);
		}
	};

#else
#ifndef HAVE_DARWIN
#include <semaphore.h>

	class ThreadController
	{
		sem_t m_hSema;
		pthread_t m_hThread;
	public:

		ThreadController()
		{
#if PLATFORM != PLATFORM_APPLE
			ASSERT(sizeof(m_hThread) <= 32);
#endif
		}

		~ThreadController()
		{
			sem_destroy(&m_hSema);
		}

		void Setup(pthread_t h)
		{
			m_hThread = h;
			sem_init(&m_hSema, PTHREAD_PROCESS_PRIVATE, 0);
		}

		void Suspend()
		{
			ASSERT(pthread_equal(pthread_self(), m_hThread));
			sem_wait(&m_hSema);
		}

		bool Resume()
		{
			ASSERT(!pthread_equal(pthread_self(), m_hThread));
			return sem_post(&m_hSema) == 0;
		}

		void Join()
		{
			// waits until the thread finishes then returns
			pthread_join(m_hThread, NULL);
		}

		uint32_t GetId()
		{
#if PLATFORM == PLATFORM_APPLE
			return (uint32_t)pthread_mach_thread_np(m_hThread);
#else
			return (uint32_t)m_hThread;
#endif
		}
	};

#else

	class ThreadController
	{
		pthread_cond_t cond;
		pthread_mutex_t mutex;
		pthread_t m_hThread;
	public:
		void Setup(pthread_t h)
		{
			m_hThread = h;
			pthread_mutex_init(&mutex, NULL);
			pthread_cond_init(&cond, NULL);
		}
		~ThreadController()
		{
			pthread_mutex_destroy(&mutex);
			pthread_cond_destroy(&cond);
		}
		void Suspend()
		{
			pthread_cond_wait(&cond, &mutex);
		}
		bool Resume()
		{
			return pthread_cond_signal(&cond) == 0;
		}
		void Join()
		{
			pthread_join(m_hThread, NULL);
		}
		INLINE uint32_t GetId() {
#if PLATFORM == PLATFORM_APPLE
			return (uint32_t)pthread_mach_thread_np(m_hThread);
#else
			return (uint32_t)m_hThread;
#endif
		}
	};

#endif

#endif

}


#endif // THREADCONTROLLER_H
