/* 
 * File:   ClientModule.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2014_4_28, 13:15
 */
#include "ClientModule.h"
#include "Log.h"
#include "NodeDefines.h"
#include "msg_login_login.pb.h"


using namespace mdl;
using namespace util;


CClientModule::CClientModule(const char* name)
	: mdl::CModule(name) {
}

CClientModule::~CClientModule() {
}

void CClientModule::OnRegister(){
	OutputBasic("OnRegister");
}

void CClientModule::OnRemove(){
    OutputBasic("OnRemove");
}

std::vector<int> CClientModule::ListNotificationInterests()
{
	std::vector<int> interests;
	return interests;
}

IModule::InterestList CClientModule::ListProtocolInterests()
{
	InterestList interests;
	interests.push_back(BindMethod<CClientModule>(
		P_CMD_CTL_LOGIN, &CClientModule::HandleLogin));
	return interests;
}

void CClientModule::HandleNotification(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{

}

void CClientModule::HandleLogin(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{

}




