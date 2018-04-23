/*
 * File:   SimpleEvent.h
 * Author: Jehu Shaw
 *
 * Created on 2010_11_12, 9:28
 */

#ifndef SIMPLEEVENT_H
#define	SIMPLEEVENT_H

#include <map>
#include "AgentMethod.h"
#include "SpinRWLock.h"

namespace evt
{
    // T is the key type
	template<class T>
    class SimpleEvent {
    public:
        bool AddEventListener(const T& id, const util::CAutoPointer<MethodRIP1Base>& method);
        int DispatchEvent(const T& id, const util::CWeakPointer<ArgumentBase>& arg);
        bool HasEventListener(const T& id);
        void RemoveEventListener(const T& id);
        void Clear();
	protected:
        typedef std::map<T, util::CAutoPointer<MethodRIP1Base> > event_map_t;
        event_map_t eventmap;
		thd::CSpinRWLock rwTicket;
    };

	template<class T>
	bool SimpleEvent<T>::AddEventListener(const T& id, const util::CAutoPointer<MethodRIP1Base>& method){
		thd::CScopedWriteLock scopedWriteLock(rwTicket);
		std::pair<typename event_map_t::iterator, bool> pairIB(eventmap.insert(
		typename event_map_t::value_type(const_cast<T&>(id), const_cast<
		util::CAutoPointer<MethodRIP1Base>& >(method))));
		return pairIB.second;
	}

	template<class T>
	int SimpleEvent<T>::DispatchEvent(const T& id, const util::CWeakPointer<ArgumentBase>& arg){
        util::CAutoPointer<MethodRIP1Base> pMethod;
        if(true) {
		    thd::CScopedReadLock scopedReadLock(rwTicket);
		    typename event_map_t::iterator it(eventmap.find(const_cast<T&>(id)));
		    if(it != eventmap.end()) {
			    pMethod = it->second;
		    }
        }

        if(pMethod.IsInvalid()) {
            return FALSE;
        }
        return pMethod->Invoke(arg);
	}

	template<class T>
	bool SimpleEvent<T>::HasEventListener(const T& id){
		thd::CScopedReadLock scopedReadLock(rwTicket);
		return eventmap.find(const_cast<T&>(id)) != eventmap.end();
	}

	template<class T>
	void SimpleEvent<T>::RemoveEventListener(const T& id){
		thd::CScopedWriteLock scopedWriteLock(rwTicket);
		eventmap.erase(const_cast<T&>(id));
	}

	template<class T>
	void SimpleEvent<T>::Clear(){
		thd::CScopedWriteLock scopedWriteLock(rwTicket);
		eventmap.clear();
	}
}
#endif	/* SIMPLEEVENT_H */

