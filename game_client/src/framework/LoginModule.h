/* 
 * File:   LoginModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_4_28, 13:15
 */

#ifndef LOGINMODULE_H
#define	LOGINMODULE_H

#include "ModuleManager.h"

class CLoginModule:public mdl::CModule {
public:
    CLoginModule(const char* name);
    virtual ~CLoginModule();

	virtual void OnRegister();

	virtual void OnRemove();

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual void HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

private:
	void HandleLogin(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void HandleLogout(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);
};

#endif	/* LOGINMODULE_H */

