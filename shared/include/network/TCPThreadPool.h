
/*
 * File:   TCPThreadPool.h
 * Author: Jehu Shaw
 *
 * Created on 2014.01.17
 */

#ifndef _TCPTHREADPOOL_H__
#define _TCPTHREADPOOL_H__

#if !(defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 ))

#include "NetworkTrace.h"
#include "TCPThreadList.h"

namespace ntwk
{

    typedef int (*do_func_t) (void *argv);

    class TCPThreadInfo;

    class TCPThreadPool
    {
    public:
        /*
         * Create a thread pool that has threadnum threads, and suspend all.
         * tfunc --- is followers function, if return 0, then not task to do. or else is to exit.
         * lfunc --- is leader function,  return need to resume threads num. if less than 0, then exit.
         * argv --- is function parameters, normally , is task manager pointer.
         */
        static TCPThreadPool* Create(int threadNum, do_func_t tfunc, do_func_t lfunc, void *argv);

        /*
         * Release thread pool.
         */
        void Release();

    private:

        inline void threadlistResumeNumThread(int num)
        {
            TCPThreadInfo *th;
            while (num > 0)
            {
                th = m_suspendList.PopFront();
                if (!th)
                    break;
                if (pthread_mutex_trylock(&th->m_mutex) == 0)
                {
                    atomic_inc(&m_activityNum);
                    th->Resume();
                    pthread_mutex_unlock(&th->m_mutex);
                }
                else
                {
                    m_suspendList.PushBack(th);
                }
                --num;
            }
        }

        inline void threadlistResumeAllThread()
        {
            TCPThreadInfo *th;
            int num = 0;
            usleep(300000);
            for (;;)
            {
                th = m_suspendList.PopFront();
                if (!th)
                    break;
                atomic_inc(&m_activityNum);
                th->Resume();
                ++num;
            }
        }

        /* called when the thread needs to exit. */
        inline void thread_do_exit(TCPThreadInfo* th)
        {
            if(NULL == th) {
                return;
            }

            /* if is leader. */
            if (th->GetIsLeader())
            {
                NWTrace("[leader as exit] threadid:%d exit...", th->GetThreadId());

                /* change to followers. */
                th->ChangeToFollower();

                /* change leader falg. */
                atomic_xchg(&m_hasLeader, 0);

                /* pop all the suspend thread that resume it. */
                threadlistResumeAllThread();
            }

            /* exit thread number increase 1. */
            atomic_inc(&m_exitNum);

            atomic_dec(&m_activityNum);

#ifdef SHOW_EXIT_STATE
            printf("suspendlist threadNum:%d, activityNum:%d, hasLeader:%d, exitNum:%d\n"
                , m_suspendList.GetThreadNum(), m_activityNum, m_hasLeader, m_exitNum);
#endif

            /* release own. */
            th->Release();
            delete th;
        }

    private:
        // callback
        static void* thread_proc(void *param);

    private:
        // suspend thread list.
        TCPThreadList m_suspendList;
        // Initialized using locks.
        pthread_mutex_t m_setupMutex;
        // task function.
        do_func_t m_taskFun;
        // leader function.
        do_func_t m_leadFun;
        // function parameters.
        void* m_argv;
        // thread num.
        long m_threadNum;
        // exit thread number.
        volatile long m_exitNum;
        // if 0, not has leader, if 1, has leader.
        volatile long m_hasLeader;
        // activity thread number.
        volatile long m_activityNum;
    };

}
#endif	/* !(defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )) */

#endif  /* _TCPTHREADPOOL_H__ */

