/*
 * File:   SimpleMultiEvent.h
 * Author: Jehu Shaw
 *
 * Created on 2010_11_12, 9:28
 */

#ifndef SIMPLEMULTIEVENT_H
#define	SIMPLEMULTIEVENT_H

#include <map>
#include "AgentMethod.h"
#include "SpinRWLock.h"

namespace evt
{
    // T is the key type
	template<class T>
    class SimpleMultiEvent {
    public:
        void AddEventListener(const T& id, const util::CAutoPointer<MethodRIP1Base>& method);
        int DispatchEvent(const T& id, util::CWeakPointer<ArgumentBase> arg, int nSuccessResult, int nFailResult);
        bool HasEventListener(const T& id);
		bool HasEventListener(const T& id, const util::CAutoPointer<MethodRIP1Base>& method);
        void RemoveEventListener(const T& id);
		void RemoveEventListener(const T& id, const util::CAutoPointer<MethodRIP1Base>& method);
        void Clear();
	protected:
        typedef std::multimap<T, util::CAutoPointer<MethodRIP1Base> > event_map_t;
        event_map_t eventmap;
		thd::CSpinRWLock rwTicket;
    };

	template<class T>
	void SimpleMultiEvent<T>::AddEventListener(const T& id, const util::CAutoPointer<MethodRIP1Base>& method){
		thd::CScopedWriteLock scopedWriteLock(rwTicket);
		eventmap.insert(typename event_map_t::value_type(const_cast<T&>(id),
			const_cast<util::CAutoPointer<MethodRIP1Base>& >(method)));
	}

	template<class T>
	int SimpleMultiEvent<T>::DispatchEvent(const T& id, util::CWeakPointer<ArgumentBase> arg, int nSuccessResult, int nFailResult){
        std::vector<util::CAutoPointer<MethodRIP1Base> > arrMethods;
        do {
		    thd::CScopedReadLock scopedReadLock(rwTicket);
		    typename event_map_t::iterator it(eventmap.lower_bound(const_cast<T&>(id)));
		    while(it != eventmap.end() && it->first == id) {
				arrMethods.push_back(it->second);
				++it;
		    }
		} while (false);

		int nResult = nFailResult;
		size_t nSize = arrMethods.size();
		for (size_t i = 0; i < nSize; ++i) {
			int nRet = arrMethods[i]->Invoke(arg);
			if (nRet == nSuccessResult) {
				nResult = nRet;
			}
			arg->Reset();
		}
		return nResult;
	}

	template<class T>
	bool SimpleMultiEvent<T>::HasEventListener(const T& id){
		thd::CScopedReadLock scopedReadLock(rwTicket);
		return eventmap.find(const_cast<T&>(id)) != eventmap.end();
	}

	template<class T>
	bool SimpleMultiEvent<T>::HasEventListener(const T& id, const util::CAutoPointer<MethodRIP1Base>& method) {
		do {
			thd::CScopedReadLock scopedReadLock(rwTicket);
			typename event_map_t::iterator it(eventmap.lower_bound(const_cast<T&>(id)));
			while (it != eventmap.end()) {
				if (it->second->Equal(*method) && it->first == id) {
					return true;
				}
				++it;
			}
		} while (false);

		return false;
	}

	template<class T>
	void SimpleMultiEvent<T>::RemoveEventListener(const T& id){
		thd::CScopedWriteLock scopedWriteLock(rwTicket);
		eventmap.erase(const_cast<T&>(id));
	}

	template<class T>
	void SimpleMultiEvent<T>::RemoveEventListener(const T& id, const util::CAutoPointer<MethodRIP1Base>& method) {
		do {
			thd::CScopedWriteLock scopedWriteLock(rwTicket);
			typename event_map_t::iterator it(eventmap.lower_bound(const_cast<T&>(id)));
			while (it != eventmap.end() && it->first == id) {
				if (it->second->Equal(*method)) {
					eventmap.erase(it);
					break;
				}
				++it;
			}
		} while (false);
	}

	template<class T>
	void SimpleMultiEvent<T>::Clear(){
		thd::CScopedWriteLock scopedWriteLock(rwTicket);
		eventmap.clear();
	}
}
#endif	/* SIMPLEMULTIEVENT_H */

