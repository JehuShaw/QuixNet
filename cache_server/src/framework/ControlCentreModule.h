/* 
 * File:   ControlCentreModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef _CONTROLCENTREMODULE_H
#define	_CONTROLCENTREMODULE_H

#include "ModuleManager.h"
#include "Singleton.h"
#include "SimpleEventArray.h"
#include "ControlCMD.h"

class CControlCentreModule
	: public mdl::CModule
	, public util::Singleton<CControlCentreModule>
{

public:
	CControlCentreModule();

	bool AddEventListener(eControlCMD cmd, const util::CAutoPointer<evt::MethodRIP1Base> method);

	bool HasEventListener(eControlCMD cmd);

	void RemoveEventListener(eControlCMD cmd);

private:
	virtual void OnRegister();

	virtual void OnRemove();

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual void HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

private:
	void CaseMasterTransport(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void CaseShutdown(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void CaseStart(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void CaseStop(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	evt::SimpleEventArray m_arrEvent;
};

#endif	/* _CONTROLCENTREMODULE_H */

