/* 
 * File:   PlayerModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef PLAYERMODULE_H
#define	PLAYERMODULE_H

#include "ModuleManager.h"

class CharacterPacket;

class CPlayerModule:public mdl::CModule {
public:
    CPlayerModule(const char* name, uint16_t serverRegion);
    virtual ~CPlayerModule();

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

    void HandleRename(const util::CWeakPointer<mdl::INotification>& request,
        util::CWeakPointer<mdl::IResponse>& reply);

	void TestCallback();
	void TestCallback2();
	void TestCallback3();
	void TestCallback4();
	util::CAutoPointer<CPlayerModule> m_pThis;
private:
	uint16_t m_u16ServerRegion;
};

#endif	/* PLAYERMODULE_H */

