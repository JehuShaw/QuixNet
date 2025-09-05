
/*
 * File:   TCPThreadList.h
 * Author: Jehu Shaw
 *
 * Created on 2014.01.17
 */

#ifndef TCPTHREADLIST_H
#define TCPTHREADLIST_H

#if !(defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 ) || defined( _WIN64 ))

#include "TCPThreadInfo.h"
#include "SpinLock.h"
#include "ScopedLock.h"

namespace ntwk
{

    class TCPThreadList
    {
    public:
        /// Init.
        inline void Init()
        {
            m_head = NULL;
            m_tail = NULL;
            m_threadNum = 0;
        }

        /// Release.
        inline void Release()
        {
            m_head = NULL;
            m_tail = NULL;
        }

        /// Get current thread numbers.
        inline int GetThreadNum()
        {
            return m_threadNum;
        }

        /// Pop
        inline TCPThreadInfo* PopFront()
        {
            struct TCPThreadInfo *th;
            if(true) { // scoped
                thd::CScopedLock scopedSpinLock(m_listLock);
                th = m_head;
                if (m_head) {

                    m_head = m_head->m_next;
                    if (th == m_tail) {

                        m_tail = NULL;
                        assert(m_head == NULL);
                    }
                }
            }// scoped

            if (th) {

                atomic_dec(&m_threadNum);
                th->m_next = NULL;
            }
            return th;
        }

        /// Push back
        inline void PushBack(TCPThreadInfo* th)
        {
            if(true) { // scoped
                thd::CScopedLock scopedSpinLock(m_listLock);
                th->m_next = NULL;
                if (m_tail) {
                    m_tail->m_next = th;
                }
                else
                {
                    assert(m_head == NULL);
                    m_head = th;
                }
                m_tail = th;
            } // scoped

            atomic_inc(&m_threadNum);
        }

    private:
	    TCPThreadInfo* m_head;
	    TCPThreadInfo* m_tail;
	    thd::CSpinLock m_listLock;
	    volatile long m_threadNum;
    };

}

#endif /* !(defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 ) || defined( _WIN64 )) */

#endif /* TCPTHREADLIST_H */



