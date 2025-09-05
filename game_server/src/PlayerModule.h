/* 
 * File:   PlayerModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef PLAYERMODULE_H
#define	PLAYERMODULE_H

#include "ModuleManager.h"
#include "ReferObject.h"


class CharacterPacket;

class CPlayerModule : public mdl::CModule {
public:
    CPlayerModule(const char* name);
    virtual ~CPlayerModule();

	virtual void OnRegister();

	virtual void OnRemove();

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual void HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

private:
	void CaseCheckCreateCharacter(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void CaseCreateCharacter(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void CaseGetCharacter(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void HandleLogin(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

    void HandleLogout(const util::CWeakPointer<mdl::INotification>& request,
        util::CWeakPointer<mdl::IResponse>& reply);

	void HandleTime(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	util::CReferObject<CPlayerModule> m_pThis;
};

#endif	/* PLAYERMODULE_H */

