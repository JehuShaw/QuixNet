/* 
 * File:   LoginModule.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2014_4_28, 13:15
 */
#include "LoginModule.h"
#include "Log.h"
#include "NodeDefines.h"
#include "msg_login_login.pb.h"
#include "InnerOperateHelper.h"

#include "GameClient.h"

using namespace mdl;
using namespace util;


CLoginModule::CLoginModule(const char* name)
	: CModule(name) {
}

CLoginModule::~CLoginModule() {
}

void CLoginModule::OnRegister(){
    OutputBasic("OnRegister");
}

void CLoginModule::OnRemove(){
    OutputBasic("OnRemove");
}

std::vector<int> CLoginModule::ListNotificationInterests()
{
	std::vector<int> interests;
	return interests;
}

IModule::InterestList CLoginModule::ListProtocolInterests()
{
	InterestList interests;
	interests.push_back(BindMethod<CLoginModule>(
		P_CMD_CTL_LOGIN, &CLoginModule::HandleLogin));
	return interests;
}

void CLoginModule::HandleNotification(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{

}

void CLoginModule::HandleLogin(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	if(request.IsInvalid()) {
		return;
	}

	CWeakPointer<::node::DataPacket> pRequest(GetInnerRequestPacket(request));
	if(pRequest.IsInvalid()) {
		return;
	}

	if(pRequest->result() != TRUE) {
		printf("server rely login fail.\n");
		return;
	}

	login::LoginResponse loginResponse;
	if(!ParseInnerData(loginResponse, pRequest)) {
		return;
	}
	
	CGameClient::PTR_T pGameClient(CGameClient::Pointer());
	pGameClient->LoginProxy(loginResponse.endpoint().c_str(),
		loginResponse.account(), loginResponse.userid(),
		loginResponse.sessionkey().c_str());

}




