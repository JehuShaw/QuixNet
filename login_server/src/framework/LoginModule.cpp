/*
 * File:   LoginModule.cpp
 * Author: Jehu Shaw
 *
 * Created on 2014_4_28, 13:15
 */
#include "LoginModule.h"
#include "Log.h"
#include "NodeDefines.h"
#include "WorkerOperateHelper.h"
#include "msg_login_login.pb.h"
#include "HttpClientManager.h"
#include "LoginWebRequest.h"
#include "AppConfig.h"
#include "TimestampManager.h"
#include "Md5.h"
#include "LoginServer.h"
#include "BodyBitStream.h"
#include "ControlCentreStubImpEx.h"
#include "LoginServer.h"

using namespace mdl;
using namespace util;
using namespace ntwk;


CLoginModule::CLoginModule(const char* name) : mdl::CModule(name) {

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
	interests.push_back(N_CMD_LOGIN_CHECK_WEB_RESULT);
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
	if(request.IsInvalid()) {
		OutputDebug("request.IsInvalid()");
		return;
	}
	switch(request->GetName()) {
	case N_CMD_LOGIN_CHECK_WEB_RESULT:
		LoginCheckWeb(request->GetBody(), request->GetType());
		break;
	default:
		OutputDebug("default cmd = %d", request->GetName());
		break;
	}

}

void CLoginModule::HandleLogin(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	unsigned int socketIdx = request->GetType();

	if(SERVER_STATUS_STOP == g_serverStatus) {
		::node::DataPacket loginResponse;
		loginResponse.set_cmd(P_CMD_CTL_LOGIN);
		loginResponse.set_result(SERVER_ERROR_SERVER_STOP);

		CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
		pLoginServer->SendToClientByIdx(socketIdx, loginResponse);
		return;
	}

	CWeakPointer<::node::DataPacket> pDpResponse(GetWorkerResponsePacket(reply));

	CWeakPointer<::node::DataPacket> pDpRequest(GetWorkerRequestPacket(request));
	if(pDpRequest.IsInvalid()) {
		OutputError("pDpRequest.IsInvalid()");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(FALSE);
		}
		return;
	}

	assert(pDpRequest->has_data());

	login::LoginRequest loginRequest;
	if(!ParseWorkerData(loginRequest, pDpRequest)) {
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(FALSE);
		}
		return;
	}

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	char szBuffer[eBUF_SIZE_512] = {'\0'};

	snprintf(szBuffer, sizeof(szBuffer), I64FMTD"%s%s", loginRequest.account(),
		loginRequest.sessionkey().c_str(), AUTH_KEY);
	szBuffer[sizeof(szBuffer) - 1] = '\0';

	MD5_CTX md5;
	std::string strAuthKey(md5.MakePassMD5(szBuffer));

	snprintf(szBuffer, sizeof(szBuffer), "%s?accountId="I64FMTD"&sessionKey=%s&authKey=%s",
		pConfig->GetString(APPCONFIG_LOGINCHECKWEB).c_str(),
		loginRequest.account(),
		loginRequest.sessionkey().c_str(),
		strAuthKey.c_str());
	szBuffer[sizeof(szBuffer) - 1] = '\0';

    OutputBasic("URL = %s", szBuffer);
	CLoginWebRequest* pLoginRequest = CLoginWebRequest::Create(
		szBuffer, loginRequest.account(), socketIdx);
	CHttpClientManager::Pointer()->AddRequest(pLoginRequest);
}

void CLoginModule::LoginCheckWeb(CWeakPointer<CBodyBitStream> request, unsigned int socketIdx)
{
	if(request.IsInvalid()) {
		OutputError("request.IsInvalid()");
		return;
	}

	bool bSuccess = request->ReadBool();
	uint64_t u64Account = request->ReadUInt64();

	if(!bSuccess) {
		OutputError("!bSuccess");

		::node::DataPacket loginResponse;
		loginResponse.set_cmd(P_CMD_CTL_LOGIN);
		loginResponse.set_result(FALSE);

		CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
		pLoginServer->SendToClientByIdx(socketIdx, loginResponse);
		return;
	}

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strServantConnect(pConfig->GetString(APPCONFIG_SERVANTCONNECT));
	std::string strProxyName(pConfig->GetString(APPCONFIG_PROXYSERVERNAME));
	if(strServantConnect.empty()) {
		OutputError("strMasterConnect.empty()");

		::node::DataPacket loginResponse;
		loginResponse.set_cmd(P_CMD_CTL_LOGIN);
		loginResponse.set_result(FALSE);

		CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
		pLoginServer->SendToClientByIdx(socketIdx, loginResponse);
		return;
	}

	if(strProxyName.empty()) {
		OutputError("strProxyName.empty()");

		::node::DataPacket loginResponse;
		loginResponse.set_cmd(P_CMD_CTL_LOGIN);
		loginResponse.set_result(FALSE);

		CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
		pLoginServer->SendToClientByIdx(socketIdx, loginResponse);
		return;
	}

	uint64_t u64UserId = ID_NULL;
	std::string strCreationTime;
	uint32_t u32ServerRegion = ID_NULL;
	CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
	if(SERVER_SUCCESS != pCtrlCenStubImpEx->CreateUserId(strServantConnect, u64Account,
		u64UserId, u32ServerRegion, strCreationTime))
	{
		OutputError("TRUE != controlCentreStubEx.CreateUserId");

		::node::DataPacket loginResponse;
		loginResponse.set_cmd(P_CMD_CTL_LOGIN);
		loginResponse.set_result(FALSE);

		CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
		pLoginServer->SendToClientByIdx(socketIdx, loginResponse);
		return;
	}
	//////////////////////////////////////////////////////////////////////////
	uint16_t u16ServerRegion = (uint16_t)u32ServerRegion;

	std::string strProxyIP;
	if(IsFixedRegion(u32ServerRegion)) {
		pCtrlCenStubImpEx->GetLowLoadByRegion(strServantConnect,
			strProxyName, u16ServerRegion, strProxyIP);
	} else {
		pCtrlCenStubImpEx->GetLowLoadNode(strServantConnect,
			strProxyName, strProxyIP, u16ServerRegion);
	}

	if(strProxyIP.empty()) {
		OutputError("strProxyIP.empty()");

		::node::DataPacket loginResponse;
		loginResponse.set_cmd(P_CMD_CTL_LOGIN);
		loginResponse.set_result(FALSE);

		CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
		pLoginServer->SendToClientByIdx(socketIdx, loginResponse);
		return;
	}

	CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
    pLoginServer->InsertSocketId(u64UserId, socketIdx);
	//////////////////////////////////////////////////////////////////////////
	char szMd5Buf[eBUF_SIZE_128] = {'\0'};
	snprintf(szMd5Buf, sizeof(szMd5Buf), I64FMTD""I64FMTD"%s", u64Account,
		u64UserId, strCreationTime.c_str());
	szMd5Buf[sizeof(szMd5Buf) - 1] = '\0';

	MD5_CTX md5;
	std::string strSessionKey(md5.MakePassMD5(szMd5Buf));

	login::LoginResponse loginResponse;
	loginResponse.set_account(u64Account);
	loginResponse.set_userid(u64UserId);
	loginResponse.set_endpoint(strProxyIP);
	loginResponse.set_sessionkey(strSessionKey);

	::node::DataPacket packet;
	packet.set_cmd(P_CMD_CTL_LOGIN);
	packet.set_result(TRUE);
	SerializeWorkerData(packet, loginResponse);

	pLoginServer->SendToClientByIdx(socketIdx, packet);
}


