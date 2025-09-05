/*
 * File:   ControlUserModule.cpp
 * Author: Jehu Shaw
 *
 * Created on 2020_12_1, 17:15
 */

#ifndef CONTROLUSERMODULE_H
#define	CONTROLUSERMODULE_H

#include "ModuleManager.h"

class CControlUserModule : public mdl::CModule
{

public:
	CControlUserModule(const char* name);

	virtual void OnRegister();

	virtual void OnRemove();

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual void HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

private:
	void CaseMasterToUser(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);
};

#endif	/* CONTROLUSERMODULE_H */

