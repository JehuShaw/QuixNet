/* 
 * File:   LoginModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_4_28, 13:15
 */

#ifndef LOGINMODULE_H
#define	LOGINMODULE_H

#include "ModuleManager.h"

class CBodyBitStream;

namespace google { namespace protobuf { class Message;} }

class CLoginModule : public mdl::CModule {
public:
    CLoginModule(const char* name);
    virtual ~CLoginModule();

	virtual void OnRegister();

	virtual void OnRemove();

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual void HandleNotification(
		const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

private:
	void HandleLogin(
		const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void LoginCheckWeb(util::CWeakPointer<CBodyBitStream> request);

	void HandleCreate(
		const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void HandleEnter(
		const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void HandleRelogin(
		const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	eServerError GetCharacterInfo(
		const std::string& strServantConnect,
		const std::string& strAgentName,
		uint64_t userId,
		uint64_t account,
		uint32_t serverRegion,
		::google::protobuf::Message& outMessage,
		bool bKick = false);
};

#endif	/* LOGINMODULE_H */

