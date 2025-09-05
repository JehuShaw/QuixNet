
//------------------------------------------------------------------------------
//
// Parameterized Lock-free Queue
// 
// If efficient running is required at both ends of the queue, 
// it is recommended to use CCircleQueue because CCircleQueue has a relatively high running efficiency.
// However, LockFreeQueue has the function of thread sequential processing to avoid thread starvation,
// while CCircleQueue does not have this function
//
//------------------------------------------------------------------------------

#ifndef LOCK_FREE_QUEUE_H
#define LOCK_FREE_QUEUE_H

#include "AtomicCas.h"
#include "LockFreeNode.h"

namespace thd {

#ifdef _MSC_VER
#ifdef _WIN64
#define BYTE_ALIGN __declspec(align(16))
#elif defined(_WIN32)
#define BYTE_ALIGN __declspec(align(8))
#elif defined(_WIN16)
#define BYTE_ALIGN __declspec(align(4))
#endif
#else
#if __WORDSIZE == 64
#define BYTE_ALIGN __attribute__((aligned(16)))
#elif __WORDSIZE == 32
#define BYTE_ALIGN __attribute__((aligned(8)))
#elif  __WORDSIZE == 16
#define BYTE_ALIGN __attribute__((aligned(4)))
#endif
#endif

    template<typename T>
#ifdef _MSC_VER
    class BYTE_ALIGN CLockFreeQueue {
#else
    class CLockFreeQueue {
#endif
        // NOTE: the order of these members is assumed by CAS2.
        CLockFreeNode<T>* volatile _pHead;
        volatile uintptr_t  _cPops;
        CLockFreeNode<T>* volatile _pTail;
        volatile uintptr_t  _cPushes;

    public:
        void Push(CLockFreeNode<T>* pNode);
        CLockFreeNode<T>* Pop();

        CLockFreeQueue(CLockFreeNode<T>* pDummy) : _cPops(0), _cPushes(0)
        {
            _pHead = _pTail = pDummy;
        }
#ifdef _MSC_VER
    };
#else
    }BYTE_ALIGN;
#endif

    template<typename T> void CLockFreeQueue<T>::Push(CLockFreeNode<T>* pNode) {
        pNode->pNext = NULL;

        uintptr_t cPushes;
        CLockFreeNode<T>* pTail;

        do {
            cPushes = _cPushes;
            pTail = _pTail;

            // NOTE: The Queue has the same consideration as the Stack.  If _pTail is
            // freed on a different thread, then this code can cause an access violation.

            // If the node that the tail points to is the last node
            // then update the last node to point at the new node.
            if (CAS(&(_pTail->pNext), reinterpret_cast<CLockFreeNode<T> *>(NULL), pNode))
            {
                break;
            }
            else
            {
                // Since the tail does not point at the last node,
                // need to keep updating the tail until it does.
                CAS2(&_pTail, pTail, cPushes, _pTail->pNext, cPushes + 1);
            }
        } while (true);

        // If the tail points to what we thought was the last node
        // then update the tail to point to the new node.
        CAS2(&_pTail, pTail, cPushes, pNode, cPushes + 1);
    }

    template<typename T> CLockFreeNode<T>* CLockFreeQueue<T>::Pop() {
        T value{};
        CLockFreeNode<T>* pHead = NULL;

        do {
            uintptr_t cPops = _cPops;
            uintptr_t cPushes = _cPushes;
            pHead = _pHead;
            CLockFreeNode<T>* pNext = pHead->pNext;

            // Verify that we did not get the pointers in the middle
            // of another update.
            if (cPops != _cPops)
            {
                continue;
            }
            // Check if the queue is empty.
            if (pHead == _pTail)
            {
                if (NULL == pNext)
                {
                    pHead = NULL; // queue is empty
                    break;
                }
                // Special case if the queue has nodes but the tail
                // is just behind. Move the tail off of the head.
                CAS2(&_pTail, pHead, cPushes, pNext, cPushes + 1);
            }
            else if (NULL != pNext)
            {
                value = pNext->value;
                // Move the head pointer, effectively removing the node
                if (CAS2(&_pHead, pHead, cPops, pNext, cPops + 1))
                {
                    break;
                }
            }
        } while (true);

        if (NULL != pHead)
        {
            pHead->value = value;
        }
        return pHead;
    }
}

#endif // LOCK_FREE_QUEUE_H