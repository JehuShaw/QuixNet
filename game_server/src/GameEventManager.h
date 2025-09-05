/* 
 * File:   GameEventManager.h
 * Author: Jehu Shaw
 *
 * Created on 2020_05_27, 15:35
 */

#ifndef GAMEEVENTMANAGER_H
#define	GAMEEVENTMANAGER_H

#include "SimpleMultiEvent.h"
#include "Singleton.h"
#include "ShareErrors.h"


class CGameEventManager 
	: public util::Singleton<CGameEventManager>
{
public:
	template<typename T>
	inline void AddEventListener(int cmd, util::CWeakPointer<T> pObj, typename evt::MemberMethodRIP1<T>::MethodType fun) {
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(new evt::MemberMethodRIP1<T>(pObj, fun));
		m_event.AddEventListener(cmd, pMethod);
	}

	inline void AddEventListener(int cmd, typename evt::GlobalMethodRIP1::MethodType fun) {
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(new evt::GlobalMethodRIP1(fun));
		m_event.AddEventListener(cmd, pMethod);
	}

	inline int DispatchEvent(int cmd) {
		return m_event.DispatchEvent(cmd, util::CWeakPointer<evt::ArgumentBase>(), SERVER_SUCCESS, SERVER_FAILURE);
	}

	inline int DispatchEvent(int cmd, const CArgumentBitStream& arg) {
		util::CAutoPointer<evt::ArgumentBase> pArg(&arg, false);
		return m_event.DispatchEvent(cmd, pArg, SERVER_SUCCESS, SERVER_FAILURE);
	}

	inline bool HasEventListener(int cmd) {
		return m_event.HasEventListener(cmd);
	}

	template<class T>
	inline bool HasEventListener(int cmd, util::CWeakPointer<T> pObj, typename evt::MemberMethodRIP1<T>::MethodType fun) {
		evt::MemberMethodRIP1<T> method(pObj, fun);
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(&method, false);
		return m_event.HasEventListener(cmd, pMethod);
	}

	inline bool HasEventListener(int cmd, typename evt::GlobalMethodRIP1::MethodType fun) {
		evt::GlobalMethodRIP1 method(fun);
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(&method, false);
		return m_event.HasEventListener(cmd, pMethod);
	}

	void RemoveEventListener(int cmd) {
		m_event.RemoveEventListener(cmd);
	}
	
	template<class T>
	inline void RemoveEventListener(int cmd, util::CWeakPointer<T> pObj, typename evt::MemberMethodRIP1<T>::MethodType fun) {
		evt::MemberMethodRIP1<T> method(pObj, fun);
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(&method, false);
		m_event.RemoveEventListener(cmd, pMethod);
	}

	inline void RemoveEventListener(int cmd, typename evt::GlobalMethodRIP1::MethodType fun) {
		evt::GlobalMethodRIP1 method(fun);
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(&method, false);
		m_event.RemoveEventListener(cmd, pMethod);
	}

	inline void Clear() {
		return m_event.Clear();
	}

private:
	evt::SimpleMultiEvent<int> m_event;
};


#endif	/* __GAMEEVENTMANAGER_H_ */

