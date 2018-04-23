
/*
 * File:   TCPThreadInfo.h
 * Author: Jehu Shaw
 *
 * Created on 2014.01.17
 */

#ifndef TCPTHREADINFO_H_
#define TCPTHREADINFO_H_

#if !(defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 ))

#include <pthread.h>

namespace ntwk
{

    class TCPThreadInfo
    {
    public:
        /// initialize.
        inline void Init()
        {
            pthread_mutex_init(&m_mutex, NULL);
            pthread_cond_init(&m_cond, NULL);
            m_isLeader = false;
            m_next = NULL;
        }

        /// release.
        inline void Release()
        {
            pthread_mutex_destroy(&m_mutex);
            pthread_cond_destroy(&m_cond);
            m_next = NULL;
        }

        /// set thread handle.
        inline void SetHandle(pthread_t h)
        {
            m_handle = h;
        }

        /// is leader?
        inline bool GetIsLeader()
        {
            return m_isLeader;
        }

        /// change to leader.
        inline void ChangeToLeader()
        {
            m_isLeader = true;
        }

        /// change to follower.
        inline void ChangeToFollower()
        {
            m_isLeader = false;
        }

        /// suspend.
        inline void Suspend()
        {
            pthread_cond_wait(&m_cond, &m_mutex);
        }

        /// resume.
        inline void Resume()
        {
            pthread_cond_signal(&m_cond);
        }
        /// join.
        inline void Join()
        {
            pthread_join(m_handle, NULL);
        }
        /// get thread id.
        inline int GetThreadId()
        {
            return m_threadId;
        }
        /// set thread id.
        inline void SetThreadId(int nThreadId)
        {
            m_threadId = nThreadId;
        }
    private:
        friend class TCPThreadList;
        friend class TCPThreadPool;
        // thread handle.
	    pthread_t m_handle;
        // condition variable.
	    pthread_cond_t m_cond;
        // mutex lock.
	    pthread_mutex_t m_mutex;
        // leader flag.
	    bool m_isLeader;
        // thread id.
	    int m_threadId;
        // link list.
	    TCPThreadInfo* m_next;
    };

}

#endif /* !(defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )) */

#endif /* TCPTHREADINFO_H_ */

