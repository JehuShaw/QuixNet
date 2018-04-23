
/*
 * File:   TCPThreadPool.cpp
 * Author: Jehu Shaw
 *
 * Created on 2011.10.15
 */

#if !(defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 ))

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "TCPThreadList.h"
#include "TCPThreadPool.h"

using namespace ntwk;

#ifndef min
#define min(a, b) (((a) < (b))? (a) : (b))
#endif



struct tempinfo
{
	TCPThreadPool* mgr;
	TCPThreadInfo* th;
};

void* TCPThreadPool::thread_proc (void *param)
{
    int tasknum;
    int num;
    struct tempinfo info;
    memcpy(&info, param, sizeof(info));
    free(param);
#if defined( __APPLE__ )
	mach_port_t tid = pthread_mach_thread_np(pthread_self());
	info.th->SetThreadId(tid);
#else
    info.th->SetThreadId(pthread_self());
#endif
    pthread_mutex_lock(&info.th->m_mutex);

    pthread_mutex_lock(&info.mgr->m_setupMutex);
    NWTrace("start thread:%u, argv thread hand:%x", pthread_self(), info.th->m_handle);
    pthread_mutex_unlock(&info.mgr->m_setupMutex);
	for (;;)
	{
		/* if is leader. */
		if (info.th->GetIsLeader())
		{
			/* do leader function,
			 * if return value less than 0, then exit.
			 * if return value greater than 0, then is need resume thread num.
			 * if return value is equal 0, then overtime.
			 */
			tasknum = info.mgr->m_leadFun(info.mgr->m_argv);
			if (tasknum > 0)
			{
				NWTrace("[new task list] leader:%u, tasknum:%d, suspendList num:%d, activityNum:%d, hasLeader:%d", info.th->m_threadId
                    , tasknum, info.mgr->m_suspendList.GetThreadNum(), info.mgr->m_activityNum, info.mgr->m_hasLeader);
				/* resume thread to do the task. */

				/* change to followers. */
				info.th->ChangeToFollower();

				/* change leader falg. */
				atomic_xchg(&info.mgr->m_hasLeader, 0);

				/* tasknum--, because leader will also do task. */
				tasknum--;
				num = min(tasknum, info.mgr->m_suspendList.GetThreadNum());
				info.mgr->threadlistResumeNumThread(num);
			}
			else if (tasknum < 0)
			{
				NWTrace("[thread pool exit] leader:%d exit... suspendList num:%d, activityNum:%d, hasLeader:%d", info.th->m_threadId
						, info.mgr->m_suspendList.GetThreadNum(), info.mgr->m_activityNum, info.mgr->m_hasLeader);

				info.mgr->thread_do_exit(info.th);
				/* return. */
				return NULL;
			}
		}
		else
		{
			/* do task function, the return value is not equal to 0, then exit. */
			if (info.mgr->m_taskFun(info.mgr->m_argv) != 0)
			{
				NWTrace("[thread exit] threadi:%u exit", info.th->m_threadId);
				info.mgr->thread_do_exit(info.th);
				/* return. */
				return NULL;
			}

			/* If own is the last activity of the followers, set own to leader. */
			if (atomic_dec(&info.mgr->m_activityNum) == 0)
			{
				/* competition leader. */
				if (atomic_cmpxchg(&info.mgr->m_hasLeader, 1, 0) == 0)	/* if old is 0, then set 1, return old value. */
				{
					NWTrace("[change to leader] threadid:%u, suspendList num:%d, activityNum:%d, hasLeader:%d", info.th->m_threadId
                        , info.mgr->m_suspendList.GetThreadNum(), info.mgr->m_activityNum, info.mgr->m_hasLeader);

					/*  change own to leader. */
					info.th->ChangeToLeader();
					atomic_inc(&info.mgr->m_activityNum);
				}
				else
				{
					NWTrace("thread change leader failed:%u, activitynum:%ld, has_leader:%ld\n", pthread_self()
                        , info.mgr->m_activityNum, info.mgr->m_hasLeader);
					assert(false && "is last activitynum, but not change leader..., error!");
				}
			}
			else
			{
				info.mgr->m_suspendList.PushBack(info.th);

				NWTrace("[suspend thread] threadid:%u, self suspend. suspendlist num:%d, activitynum:%d, has_leader:%d"
                    , info.th->m_threadId, info.mgr->m_suspendList.GetThreadNum(), info.mgr->m_activityNum, info.mgr->m_hasLeader);

				/* suspend. */
				info.th->Suspend();
				NWTrace("[for resume] threadid:%u", info.th->m_threadId);
			}
		}
	}
}

/*
 * Create a thread pool that has threadnum threads, and suspend all.
 * tfunc --- is task function, if return 0, then not task to do. or else is to exit.
 * lfunc --- is leader function,  return need to resume threads num. if less than 0, then exit.
 * argv --- is function parameters, normally , is task manager pointer.
 */
TCPThreadPool* TCPThreadPool::Create(int threadNum, do_func_t tfunc, do_func_t lfunc, void *argv)
{
    assert(threadNum > 0 && tfunc != NULL && lfunc != NULL && argv != NULL);
    if (threadNum <= 0 || !tfunc || !lfunc || !argv) {
	return NULL;
    }

    TCPThreadPool* mgr(new TCPThreadPool());
    if(NULL == mgr) {
	return NULL;
    }

    /* initialize */
    mgr->m_suspendList.Init();
    pthread_mutex_init(&mgr->m_setupMutex, NULL);
    mgr->m_taskFun = tfunc;
    mgr->m_leadFun = lfunc;
    mgr->m_argv = argv;
    mgr->m_threadNum = threadNum;
    mgr->m_exitNum = 0;
    mgr->m_hasLeader = 0;
    mgr->m_activityNum = threadNum;

    pthread_mutex_lock(&mgr->m_setupMutex);

    pthread_t thandle;
    TCPThreadInfo* tinfo;
    struct tempinfo* info;
    /* create thread. */
    for(int i = 0; i < threadNum; ++i)
    {
	tinfo = new TCPThreadInfo();
	if(NULL == tinfo) {
            NWTrace("create threadinfo failed, function malloc return null!");
            if(0 == i) {
                delete mgr;
            }
            mgr = NULL;
            break;
        }
        info = (struct tempinfo *)malloc(sizeof(struct tempinfo));
        if(NULL == info) {
            NWTrace("create tempinfo failed, function malloc return null!");
            delete tinfo;
            tinfo = NULL;
            if(0 == i) {
                delete mgr;
            }
            mgr = NULL;
            break;
        }
        tinfo->Init();
        tinfo->SetThreadId(i);

        info->th = tinfo;
        info->mgr = mgr;
        if(pthread_create(&thandle, NULL, &thread_proc, (void *)info) != 0) {
            NWTrace("pthread_create create thread error!, errno:%d", errno);
            delete tinfo;
            tinfo = NULL;
            if(0 == i) {
                delete mgr;
            }
            mgr = NULL;
            break;
        }
    }

    pthread_mutex_unlock(&mgr->m_setupMutex);

    return mgr;
}


/* release thread pool. */
void TCPThreadPool::Release()
{
	/* pop all the suspend thread that resume it. */
	threadlistResumeAllThread();

	while (m_exitNum != m_threadNum)
	{
		assert(m_exitNum >= 0);
		assert(m_exitNum <= m_threadNum);
		usleep(100000);	/* 100 ms */
	}

#ifdef SHOW_EXIT_STATE
	printf("thread num:%d, suspendNum:%d, activityNum:%d, hasLeader:%d, exitNum:%d\n"
        , m_threadNum, m_suspendList.GetThreadNum(), m_activityNum, m_hasLeader, m_exitNum);
#endif

	assert(m_activityNum == 0);
	assert(m_hasLeader == 0);
	assert(m_exitNum == m_threadNum);

	m_suspendList.Release();
	pthread_mutex_destroy(&m_setupMutex);

    delete this;
}

#endif /* !(defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )) */

