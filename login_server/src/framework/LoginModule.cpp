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
#include "msg_login.pb.h"
#include "msg_node_create_character.pb.h"
#include "msg_node_get_character.pb.h"
#include "msg_node_switch.pb.h"
#include "HttpClientManager.h"
#include "LoginWebRequest.h"
#include "AppConfig.h"
#include "TimestampManager.h"
#include "Md5.h"
#include "LoginServer.h"
#include "BodyBitStream.h"
#include "ControlCentreStubImpEx.h"
#include "LoginServer.h"
#include "LoginPlayer.h"

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
	interests.push_back(BindMethod<CLoginModule>(
		P_CMD_CTL_CREATE, &CLoginModule::HandleCreate));
	interests.push_back(BindMethod<CLoginModule>(
		P_CMD_CTL_ENTER, &CLoginModule::HandleEnter));
	interests.push_back(BindMethod<CLoginModule>(
		P_CMD_CTL_RELOGIN, &CLoginModule::HandleRelogin));
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
		LoginCheckWeb(request->GetBody());
		break;
	default:
		OutputDebug("default cmd = %d", request->GetName());
		break;
	}
}

void CLoginModule::HandleLogin(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	printf("CLoginModule::HandleLogin begin ! \n");
	CWeakPointer<CLoginPlayer> pPlayer(GetWorkerPlayer(request));
	ntwk::SocketID socketId(pPlayer->GetSocketID());

	if(SERVER_STATUS_STOP == g_serverStatus) {
		::node::DataPacket loginResponse;
		loginResponse.set_cmd(P_CMD_CTL_LOGIN);
		loginResponse.set_result(SERVER_ERROR_SERVER_STOP);

		CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
		pLoginServer->SendToClientBySocketID(socketId, loginResponse);
		OutputError("SERVER_STATUS_STOP == g_serverStatus");
		return;
	}

	CWeakPointer<::node::DataPacket> pDpResponse(GetWorkerResponsePacket(reply));

	CWeakPointer<::node::DataPacket> pDpRequest(GetWorkerRequestPacket(request));
	if(pDpRequest.IsInvalid()) {
		OutputError("pDpRequest.IsInvalid()");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	assert(HasDataPacketDataPtr(pDpRequest));

	login::LoginRequest loginRequest;
	if(!ParseWorkerData(loginRequest, pDpRequest)) {
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	// 判断是否已经登录过
	ntwk::SocketID oldSocketID;
	CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
	if(pLoginServer->FindSocketId(loginRequest.account(), oldSocketID)) {
		if(socketId != oldSocketID) {
			pLoginServer->CloseSocket(oldSocketID, LOGOUT_KICK);
		}
	}

	// no respond 
	if(!pDpResponse.IsInvalid()) {
		pDpResponse->set_result(SERVER_NO_RESPOND);
	}

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	char szBuffer[eBUF_SIZE_512] = {'\0'};

	int nOffset = snprintf(szBuffer, sizeof(szBuffer), I64FMTD "%s%s", loginRequest.account(),
		loginRequest.sessionkey().c_str(), AUTH_KEY);
	szBuffer[sizeof(szBuffer) - 1] = '\0';

	OutputBasic("Before MD5 Data = %s", szBuffer);

	std::string strAuthKey(util::md5(szBuffer, nOffset));

	snprintf(szBuffer, sizeof(szBuffer), "%s?accountId=" I64FMTD "&sessionKey=%s&authKey=%s",
		pConfig->GetString(APPCONFIG_LOGINCHECKWEB).c_str(),
		loginRequest.account(),
		loginRequest.sessionkey().c_str(),
		strAuthKey.c_str());
	szBuffer[sizeof(szBuffer) - 1] = '\0';

    OutputBasic("URL = %s", szBuffer);
	CLoginWebRequest* pLoginRequest = CLoginWebRequest::Create(
		szBuffer, loginRequest.account(), socketId);
	CHttpClientManager::Pointer()->AddRequest(pLoginRequest);

	printf("CLoginModule::HandleLogin AddRequest  u64Account = " I64FMTD " \n", loginRequest.account());
}

void CLoginModule::LoginCheckWeb(CWeakPointer<CBodyBitStream> request)
{
	if(request.IsInvalid()) {
		OutputError("request.IsInvalid()");
		return;
	}

	bool bSuccess = request->ReadBool();
	uint64_t u64Account = request->ReadUInt64();
	SocketID socketId;
	socketId.index = request->ReadInt32();
	socketId.binaryAddress = request->ReadUInt32();
	socketId.port = request->ReadUInt16();

	if(!bSuccess) {
		OutputError("!bSuccess");

		::node::DataPacket loginResponse;
		loginResponse.set_cmd(P_CMD_CTL_LOGIN);
		loginResponse.set_result(SERVER_ERROR_UNKNOW);

		CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
		pLoginServer->SendToClientBySocketID(socketId, loginResponse);
		return;
	}
	// 记录玩家登入
	CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
	pLoginServer->InsertSocketId(socketId, u64Account);
	// 获取角色
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strServantConnect(pConfig->GetString(APPCONFIG_SERVANTCONNECT));
	if(strServantConnect.empty()) {
		OutputError("strServantConnect.empty()");

		::node::DataPacket loginResponse;
		loginResponse.set_cmd(P_CMD_CTL_LOGIN);
		loginResponse.set_result(SERVER_ERROR_UNKNOW);

		CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
		pLoginServer->SendToClientBySocketID(socketId, loginResponse);
		return;
	}

	std::string strAgentName(pConfig->GetString(APPCONFIG_AGENTSERVERNAME));
	if (strAgentName.empty()) {
		OutputError("strAgentName.empty()");

		::node::DataPacket loginResponse;
		loginResponse.set_cmd(P_CMD_CTL_LOGIN);
		loginResponse.set_result(SERVER_ERROR_UNKNOW);

		CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
		pLoginServer->SendToClientBySocketID(socketId, loginResponse);
		return;
	}

	::node::GetUserResponse getResponse;
	CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
	eServerError serRet = pCtrlCenStubImpEx->GetUsers(strServantConnect,
		u64Account, getResponse);
	if (SERVER_SUCCESS != serRet) {

		OutputError("SERVER_SUCCESS != pCtrlCenStubImpEx->GetUsers() serRet = %d ", serRet);

		::node::DataPacket loginResponse;
		loginResponse.set_cmd(P_CMD_CTL_LOGIN);
		loginResponse.set_result(SERVER_ERROR_UNKNOW);

		CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
		pLoginServer->SendToClientBySocketID(socketId, loginResponse);
		return;
	}

	login::LoginResponse loginResponse;
	loginResponse.set_account(u64Account);

	int nSize = getResponse.rows_size();
	for (int i = 0; i < nSize; ++i) {
		const ::node::UserPacket& userData = getResponse.rows(i);

		// 获得角色信息
		::node::GetCharacterResponse getCharacterResponse;
		eServerError serResult = GetCharacterInfo(strServantConnect, strAgentName,
			userData.userid(), u64Account, userData.serverregion(), getCharacterResponse, true);
		if (SERVER_ERROR_NOTFOUND_CHARACTER == serResult) {
			// 已经做删除处理的不发给客户端
			continue;
		}

		::login::Character* pChara = loginResponse.add_charaset();
		pChara->set_userid(userData.userid());
		pChara->set_cfgid(getCharacterResponse.cfgid());
		pChara->set_name(getCharacterResponse.name());
        pChara->set_info(getCharacterResponse.info());
		pChara->set_mapid(userData.mapid());


		printf("CLoginModule::LoginCheckWeb Character  cfgid = %u u64Account = " I64FMTD "\n", pChara->cfgid(), u64Account);
	}

	::node::DataPacket packet;
	packet.set_cmd(P_CMD_CTL_LOGIN);
	packet.set_result(SERVER_SUCCESS);
	SerializeWorkerData(packet, loginResponse);

	printf("CLoginModule::LoginCheckWeb SendToClientBySocketID  u64Account = " I64FMTD " \n", u64Account);
	if (!pLoginServer->SendToClientBySocketID(socketId, packet)) {
		OutputError("!pLoginServer->SendToClientBySocketID ");
	}
}

void CLoginModule::HandleCreate(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pDpResponse(GetWorkerResponsePacket(reply));

	CWeakPointer<CLoginPlayer> pPlayer(GetWorkerPlayer(request));
	if(pPlayer.IsInvalid()) {
		OutputError("pPlayer.IsInvalid()");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	ntwk::SocketID socketId(pPlayer->GetSocketID());

	CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
	uint64_t account = pLoginServer->GetLoggedAccount(socketId);
	if(DEFAULT_ACCOUNT == account) {
		OutputError("!pLoginServer->IsLoggedAccount()");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	CWeakPointer<::node::DataPacket> pDpRequest(GetWorkerRequestPacket(request));
	if (pDpRequest.IsInvalid()) {
		OutputError("pDpRequest.IsInvalid()");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	login::CreateRequest createRequest;
	if (!ParseWorkerData(createRequest, pDpRequest)) {
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(PARSE_PACKAGE_FAIL);
		}
		return;
	}

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strServantConnect(pConfig->GetString(APPCONFIG_SERVANTCONNECT));
	if (strServantConnect.empty()) {
		OutputError("strServantConnect.empty()");
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	std::string strAgentName(pConfig->GetString(APPCONFIG_AGENTSERVERNAME));
	if (strAgentName.empty()) {
		OutputError("strAgentName.empty()");
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());

	uint16_t u16ServerRegion = 0;
	std::string agentEndPoint;
	pCtrlCenStubImpEx->GetLowLoadNode(strServantConnect,
		strAgentName, u16ServerRegion, NULL, &agentEndPoint);

	if (agentEndPoint.empty()) {
		OutputError("agentEndPoint.empty()");
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	::node::CheckCreateCharRequest checkCreateRequest;
	checkCreateRequest.set_account(account);
	checkCreateRequest.set_cfgid(createRequest.cfgid());
	checkCreateRequest.set_name(createRequest.name());
	uint64_t tempId = CombineUserId((uint32_t)account, 0);
	::node::CheckCreateCharResponse checkCreateResponse;
	eServerError serResult = InvokeWorkerNotification(
		agentEndPoint, N_CMD_CHECK_CREATE_CHARACTER, tempId,
		&checkCreateRequest, &checkCreateResponse);
	if (SERVER_SUCCESS != serResult) {
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(serResult);
		}
		return;
	}

	::node::CreateUserResponse idResponse;
	serResult = pCtrlCenStubImpEx->CreateUser(strServantConnect,
		account, idResponse, checkCreateResponse.mapid(), MAX_CHARARER_SIZE);
	if (SERVER_SUCCESS != serResult) {
		OutputError("SERVER_SUCCESS != pCtrlCenStubImpEx->CreateUser serResult = %d ", serResult);
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(serResult);
		}
		return;
	}

	const ::node::UserPacket& userData = idResponse.row();

	::node::CreateCharacterRequest createCharacterRequest;
	createCharacterRequest.set_account(account);
	createCharacterRequest.set_cfgid(createRequest.cfgid());
	createCharacterRequest.set_name(checkCreateResponse.name());
	createCharacterRequest.set_info(createRequest.info());
	serResult = InvokeWorkerNotification(agentEndPoint,
		N_CMD_CREATE_CHARACTER, userData.userid(),
		&createCharacterRequest, NULL);

	if (SERVER_SUCCESS != serResult) {
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(serResult);
		}
		OutputError("SERVER_SUCCESS != serResult serResult = %d ", serResult);
		if (SERVER_CALL_DEADLINE != serResult) {
			// 删除已经创建的数据
			int nDelResult = pCtrlCenStubImpEx->DeleteUser(strServantConnect, userData.userid());
			if (SERVER_SUCCESS != nDelResult) {
				OutputError("SERVER_SUCCESS != pCtrlCenStubImpEx->DeleteUser nDelResult = %d userId = "
					I64FMTD, nDelResult, userData.userid());
			}
		}
		return;
	}

	if (!pDpResponse.IsInvalid()) {

		login::CreateResponse createResponse;
		createResponse.set_account(account);
		
		::login::Character* pChara = createResponse.mutable_chara();
		pChara->set_userid(userData.userid());
		pChara->set_cfgid(createRequest.cfgid());
		pChara->set_name(checkCreateResponse.name());

		SerializeWorkerData(pDpResponse, createResponse);
		pDpResponse->set_result(SERVER_SUCCESS);
	}
}

void CLoginModule::HandleEnter(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pDpResponse(GetWorkerResponsePacket(reply));

	CWeakPointer<::node::DataPacket> pDpRequest(GetWorkerRequestPacket(request));
	if (pDpRequest.IsInvalid()) {
		OutputError("pDpRequest.IsInvalid()");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_FAILURE);
		}
		return;
	}

	OutputDebug("0 Begin Handle Enter. ");

	CWeakPointer<CLoginPlayer> pPlayer(GetWorkerPlayer(request));
	if (pPlayer.IsInvalid()) {
		OutputError("pPlayer.IsInvalid()");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	OutputDebug("1 Handle Enter.");

	ntwk::SocketID socketId(pPlayer->GetSocketID());

	CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
	if(!pLoginServer->IsLoggedAccount(socketId)) {
		OutputError("!pLoginServer->IsLoggedAccount()");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	OutputDebug("2 Handle Enter. ");

	login::EnterRequest enterRequest;
	if (!ParseWorkerData(enterRequest, pDpRequest)) {
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	OutputDebug("3 Handle Enter. userId = " I64FMTD, enterRequest.userid());

	AppConfig::PTR_T pConfig(AppConfig::Pointer());

	std::string strServantConnect(pConfig->GetString(APPCONFIG_SERVANTCONNECT));
	if (strServantConnect.empty()) {
		OutputError("strServantConnect.empty()");
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	OutputDebug("4 Handle Enter. userId = " I64FMTD, enterRequest.userid());

	::node::CheckUserResponse checkResponse;
	CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
	eServerError serRet = pCtrlCenStubImpEx->CheckUser(strServantConnect,
		enterRequest.userid(), checkResponse);
	if (SERVER_SUCCESS != serRet)
	{
		OutputError("SERVER_SUCCESS != pCtrlCenStubImpEx->CheckUserId() serRet = %d ", serRet);
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	OutputDebug("5 Handle Enter. userId = " I64FMTD, enterRequest.userid());

	std::string strAgentName(pConfig->GetString(APPCONFIG_AGENTSERVERNAME));
	if (strAgentName.empty()) {
		OutputError("strAgentName.empty()");
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	OutputDebug("6 Handle Enter. userId = " I64FMTD, enterRequest.userid());

	std::string strAgentIP(pCtrlCenStubImpEx->SeizeServerReturnIP(
		strServantConnect, strAgentName, enterRequest.userid(), true));

	if(strAgentIP.empty()) {
		OutputError("strAgentIP.empty()");
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	OutputDebug("9 Handle Enter. userId = " I64FMTD, enterRequest.userid());

	if (!pDpResponse.IsInvalid()) {
		//////////////////////////////////////////////////////////////////////////
		char szMd5Buf[eBUF_SIZE_128] = { '\0' };
		int nOffset = snprintf(szMd5Buf, sizeof(szMd5Buf), I64FMTD "" I64FMTD "%s%u", checkResponse.account(),
			enterRequest.userid(), checkResponse.createtime().c_str(), checkResponse.logincount());
		szMd5Buf[sizeof(szMd5Buf) - 1] = '\0';

		std::string strSessionKey(util::md5(szMd5Buf, nOffset));

		login::EnterResponse enterResponse;
		enterResponse.set_account(checkResponse.account());
		enterResponse.set_userid(enterRequest.userid());
		enterResponse.set_endpoint(strAgentIP);
		enterResponse.set_sessionkey(strSessionKey);

		SerializeWorkerData(pDpResponse, enterResponse);
		pDpResponse->set_result(SERVER_SUCCESS);
	}
	OutputDebug("End Handle Enter. userId = " I64FMTD, enterRequest.userid());
}

void CLoginModule::HandleRelogin(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pDpResponse(GetWorkerResponsePacket(reply));

	CWeakPointer<::node::DataPacket> pDpRequest(GetWorkerRequestPacket(request));
	if (pDpRequest.IsInvalid()) {
		OutputError("pDpRequest.IsInvalid()");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	CWeakPointer<CLoginPlayer> pPlayer(GetWorkerPlayer(request));
	if (pPlayer.IsInvalid()) {
		OutputError("pPlayer.IsInvalid()");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	ntwk::SocketID socketId(pPlayer->GetSocketID());

	::node::SwitchData switchRequest;
	if(!ParseWorkerData(switchRequest, pDpRequest)) {
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	// 判断是否已经登录过
	ntwk::SocketID oldSocketID;
	CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
	if (pLoginServer->FindSocketId(switchRequest.account(), oldSocketID)) {
		if (socketId != oldSocketID) {
			pLoginServer->CloseSocket(oldSocketID, LOGOUT_KICK);
		}
	}

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strServantConnect(pConfig->GetString(APPCONFIG_SERVANTCONNECT));
	if(strServantConnect.empty()) {
		OutputError("strServantConnect.empty()");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	std::string strAgentName(pConfig->GetString(APPCONFIG_AGENTSERVERNAME));
	if (strAgentName.empty()) {
		OutputError("strAgentName.empty()");
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	::node::CheckUserResponse checkResponse;
	CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
	eServerError serRet = pCtrlCenStubImpEx->CheckUser(strServantConnect,
		switchRequest.userid(), checkResponse);
	if (SERVER_SUCCESS != serRet) {
		OutputError("SERVER_SUCCESS != pCtrlCenStubImpEx->CheckUserId() serRet = %d ", serRet);
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	do {
		char szMd5Buf[eBUF_SIZE_128] = { '\0' };
		int nOffset = snprintf(szMd5Buf, sizeof(szMd5Buf), I64FMTD "" I64FMTD "%s%u" I64FMTD,
			switchRequest.account(), switchRequest.userid(),checkResponse.createtime().c_str(),
			checkResponse.logincount(), switchRequest.timestamp());
		szMd5Buf[sizeof(szMd5Buf) - 1] = '\0';

		if (switchRequest.sessionkey() != util::md5(szMd5Buf, nOffset)) {
			OutputError("loginRequest.sessionkey() != md5(szMd5Buf)");
			if(!pDpResponse.IsInvalid()) {
				pDpResponse->set_result(SERVER_ERROR_UNKNOW);
			}
			return;
		}
	} while (false);

	uint64_t curTimestamp = evt::CTimestampManager::Pointer()->GetTimestamp();
	uint64_t loginTimestamp = switchRequest.timestamp() + RELOGIN_TIMEOUT;
	if(loginTimestamp < curTimestamp) {
		OutputError("loginTimestamp " I64FMTD " < curTimestamp " I64FMTD, loginTimestamp, curTimestamp);
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	serRet = pCtrlCenStubImpEx->UpdateUser(
		strServantConnect, switchRequest.userid(),
		checkResponse.mapid(),checkResponse.serverregion(), true);
	if(SERVER_SUCCESS != serRet) {
		OutputError("SERVER_SUCCESS != pCtrlCenStubImpEx->UpdateLoginCount() serRet = %d ", serRet);
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	// 记录玩家登入
	pLoginServer->InsertSocketId(socketId, switchRequest.account());

	::node::GetUserResponse getResponse;
	serRet = pCtrlCenStubImpEx->GetUsers(strServantConnect,
		switchRequest.account(), getResponse);
	if(SERVER_SUCCESS != serRet)
	{
		OutputError("SERVER_SUCCESS != pCtrlCenStubImpEx->GetUserIds() serRet = %d ", serRet);
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_UNKNOW);
		}
		return;
	}

	if(!pDpResponse.IsInvalid()) {
		::login::ReloginResponse reloginResponse;
		reloginResponse.set_account(switchRequest.account());
		int nSize = getResponse.rows_size();
		for (int i = 0; i < nSize; ++i) {
			const ::node::UserPacket& userData = getResponse.rows(i);

			// 获得角色信息
			::node::GetCharacterResponse getCharacterResponse;
			eServerError serResult = GetCharacterInfo(strServantConnect, strAgentName,
				userData.userid(), switchRequest.account(), userData.serverregion(), getCharacterResponse);
			if (SERVER_ERROR_NOTFOUND_CHARACTER == serResult) {
				// 已经做删除处理的不发给客户端
				continue;
			}

			::login::Character* pChara = reloginResponse.add_charaset();
			pChara->set_userid(userData.userid());
			pChara->set_cfgid(getCharacterResponse.cfgid());
			pChara->set_name(getCharacterResponse.name());
			pChara->set_mapid(userData.mapid());
		}

		SerializeWorkerData(pDpResponse, reloginResponse);
		pDpResponse->set_result(SERVER_SUCCESS);
	}
}

eServerError CLoginModule::GetCharacterInfo(
	const std::string& strServantConnect,
	const std::string& strAgentName,
	uint64_t userId,
	uint64_t account,
	uint32_t serverRegion,
	::google::protobuf::Message& outMessage,
	bool bKick/* = false*/)
{
	std::string agentEndPoint;
	uint16_t u16ServerRegion = (uint16_t)serverRegion;
	CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
	if (IsFixedRegion(serverRegion)) {
		pCtrlCenStubImpEx->GetLowLoadByRegion(strServantConnect,
			strAgentName, u16ServerRegion, NULL, &agentEndPoint);
	} else {
		pCtrlCenStubImpEx->GetLowLoadNode(strServantConnect,
			strAgentName, u16ServerRegion, NULL, &agentEndPoint);
	}
	if (agentEndPoint.empty()) {
		OutputError("agentEndPoint.empty()");
		return SERVER_ERROR_UNKNOW;
	}
	
	::node::GetCharacterRequest inMessage;
	inMessage.set_kick(bKick);
	eServerError serResult = InvokeWorkerNotification(agentEndPoint,
		N_CMD_GET_CHARACTER, userId, &inMessage, &outMessage);

	if (SERVER_ERROR_NOTFOUND_CHARACTER == serResult) {
		// 删除已经创建的数据
		int nDelResult = pCtrlCenStubImpEx->DeleteUser(strServantConnect, userId);
		if (SERVER_SUCCESS != nDelResult) {
			OutputError("SERVER_SUCCESS != pCtrlCenStubImpEx->DeleteUser nDelResult = %d userId = "
				I64FMTD, nDelResult, userId);
		}
	}
	return serResult;
}
