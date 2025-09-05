/* 
 * File:   ControlCentreModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef CONTROLCENTREMODULE_H
#define	CONTROLCENTREMODULE_H

#include "ModuleManager.h"

class CControlCentreModule : public mdl::CModule
{
public:
	CControlCentreModule(const char* name);

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
};

#endif	/* CONTROLCENTREMODULE_H */

