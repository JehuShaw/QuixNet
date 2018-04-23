/* 
 * File:   ClientModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_4_28, 13:15
 */

#ifndef _CLIENTMODULE_H_
#define	_CLIENTMODULE_H_

#include "ModuleManager.h"

class CClientModule:public mdl::CModule {
public:
    CClientModule(const char* name);
    virtual ~CClientModule();

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

#endif	/* _CLIENTMODULE_H_ */

