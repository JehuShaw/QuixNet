/*
 * File:   ControlEventManager.cpp
 * Author: Jehu Shaw
 *
 * Created on 2020_12_1, 17:15
 */

#ifndef CONTROLEVENTMANAGER_H
#define	CONTROLEVENTMANAGER_H

#include "Singleton.h"
#include "SimpleEventArray.h"
#include "ControlCMD.h"

class CControlEventManager
	: public util::Singleton<CControlEventManager>
{
public:
	CControlEventManager();

	template<typename T>
	inline void AddEventListener(eControlCMD cmd, util::CWeakPointer<T> pObj, typename evt::MemberMethodRIP1<T>::MethodType fun) {
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(new evt::MemberMethodRIP1<T>(pObj, fun));
		AddEventListener(cmd, pMethod);
	}

	inline void AddEventListener(eControlCMD cmd, typename evt::GlobalMethodRIP1::MethodType fun) {
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(new evt::GlobalMethodRIP1(fun));
		AddEventListener(cmd, pMethod);
	}

	bool AddEventListener(eControlCMD cmd, const util::CAutoPointer<evt::MethodRIP1Base> method);

	bool HasEventListener(eControlCMD cmd);

	void RemoveEventListener(eControlCMD cmd);

	inline int DispatchEvent(int id, const util::CWeakPointer<evt::ArgumentBase>& arg) {
		return m_arrEvent.DispatchEvent(id, arg);
	}

private:
	evt::SimpleEventArray m_arrEvent;
};

#endif	/* CONTROLEVENTMANAGER_H */

