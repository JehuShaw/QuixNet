/* 
 * File:   SimpleEventArray.h
 * Author: Jehu Shaw
 *
 * Created on 2010_11_12, 9:28
 */

#ifndef SIMPLEEVENTARRAY_H
#define	SIMPLEEVENTARRAY_H

#include <map>
#include "AgentMethod.h"
#include "SpinRWLock.h"

namespace evt
{
    class SimpleEventArray {
    public:
        SimpleEventArray(int nSize) {
            thd::CScopedWriteLock scopedWriteLock(rwTicket);
			if(nSize < 1) {
				nArraySize = 0;
				pEventArray = NULL;
				return;
			}
            nArraySize = nSize;
            pEventArray = new util::CAutoPointer<MethodRIP1Base> [nArraySize];
        }
        ~SimpleEventArray() {
            thd::CScopedWriteLock scopedWriteLock(rwTicket);
            nArraySize = 0;
            delete[] pEventArray;
            pEventArray =  NULL;
        }
        bool AddEventListener(int id, const util::CAutoPointer<MethodRIP1Base>& method) {
            thd::CScopedWriteLock scopedWriteLock(rwTicket);
            if(NULL == pEventArray) {
                return false;
            }
            if(id < 0 || id >= nArraySize) {
                return false;
            }
            if(!pEventArray[id].IsInvalid()) {
               return false; 
            }
            pEventArray[id] = method; 
            return true;
        }
        int DispatchEvent(int id, const util::CWeakPointer<ArgumentBase>& arg) {
            util::CAutoPointer<MethodRIP1Base> pMethod;
            if(true) {
                thd::CScopedReadLock scopedReadLock(rwTicket);
                if(NULL == pEventArray) {
                    return FALSE;
                }
                if(id < 0 || id >= nArraySize) {
                    return FALSE;
                }
                pMethod = pEventArray[id];
            }
            if(pMethod.IsInvalid()) {
                return FALSE; 
            }
            return pMethod->Invoke(arg);
        }
        bool HasEventListener(int id) {
            thd::CScopedReadLock scopedReadLock(rwTicket);
            if(NULL == pEventArray) {
                return false;
            }
            if(id < 0 || id >= nArraySize) {
                return false;
            }
            if(pEventArray[id].IsInvalid()) {
                return false;
            }
            return true;
        }
        void RemoveEventListener(int id) {
            thd::CScopedWriteLock scopedWriteLock(rwTicket);
            if(NULL == pEventArray) {
                return;
            }
            if(id < 0 || id >= nArraySize) {
                return;
            }
            if(pEventArray[id].IsInvalid()) {
                return;
            }
            pEventArray[id].SetRawPointer(NULL);
        }
	protected:
        util::CAutoPointer<MethodRIP1Base>* pEventArray;
        int nArraySize;
		thd::CSpinRWLock rwTicket;
    };
}
#endif	/* SIMPLEEVENTARRAY_H */

