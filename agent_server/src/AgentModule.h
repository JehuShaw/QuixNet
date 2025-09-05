/* 
 * File:   AgentModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef AGENTMODULE_H
#define	AGENTMODULE_H

#include "ModuleManager.h"

class CAgentModule
	: public mdl::CModule
{

public:
	CAgentModule(const char* name);

	virtual void OnRegister();

	virtual void OnRemove();

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual void HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

private:
	void CaseStop(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void CaseGetCharacter(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	static void CloseAllClient();
};

#endif	/* AGENTMODULE_H */

