/*
 * File:   GameClient.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */
#include <stdio.h>
#include <iostream>
#include <iosfwd>
#include <ios>
#include "Log.h"
#include "Md5.h"
#include "TimerManager.h"
#include "NodeDefines.h"
#include "AppConfig.h"
#include "GameClient.h"
#include "ModuleManager.h"
#include "NetworkTypes.h"
#include "BodyBitStream.h"
#include "GuidFactory.h"
#include "LoginModule.h"
#include "data_packet.pb.h"
#include "CommandManager.h"
#include "HttpClientManager.h"

#include "LoginWebRequest.h"
#include "InnerOperateHelper.h"

#include "msg_login_login.pb.h"
#include "msg_agent_login.pb.h"

using namespace mdl;
using namespace evt;
using namespace ntwk;
using namespace thd;
using namespace util;

CGameClient::CGameClient()
	: m_isStarted(false)
{
}

CGameClient::~CGameClient() {
    Dispose();
}

bool CGameClient::Init() {

	if(atomic_cmpxchg8(&m_isStarted,
		true, false) == (char)true) {
			return false;
	}

    srand((unsigned)time(NULL));

    bool rc = true;
    // open log file
	LogInit(5, "./log/");
	PrintBasic("%s %s %s", CONFIG, PLATFORM_TEXT, ARCH);
	PrintError("%s %s %s", CONFIG, PLATFORM_TEXT, ARCH);
    // read config file
	AppConfig::PTR_T pConfig(AppConfig::Pointer());

	int GameThreads = pConfig->GetInt(APPCONFIG_GAMETHREADS);
	if(GameThreads < 10) {
		GameThreads = 10;
	}
	// init register command
	RegisterCommand();
    // register module
    RegistModule();

	// start thread pool.
	ThreadPool.Startup(GameThreads);

	// start thread
	ThreadPool.ExecuteTask(this);

    return rc;
}

void CGameClient::Dispose() {

	if(!m_isStarted) {
		return;
	}
	// unregister module
	UnregistModule();
	// set break the loop
	atomic_xchg8(&m_isStarted, false);
	// close game thread
	ThreadPool.Shutdown();
    // close timer event
	CTimerManager::Release();
	// close command 
	CommandManager::Release();
	// close facade
	CFacade::Release();
	// close tcp socket
	m_tcpClient.Stop();
	// log dispose
	LogRelease();
}

bool CGameClient::Run() {

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	CFacade::PTR_T pFacade(CFacade::Pointer());
	SocketID socketId;

	CHttpClientManager::PTR_T pHttpClientMgr(CHttpClientManager::Pointer());

    while (m_isStarted) {
		// timer
		pTMgr->Loop();
		// http
		pHttpClientMgr->Run();
		// tcp
		if(m_tcpClient.HasNewConnection(socketId)){
			PrintBasic("recv ip address = %s  port = %d........\n",
				socketId.ToString(false), socketId.port);
		}
		//
		if(m_tcpClient.HasLostConnection(socketId)){
			PrintBasic("lost ip address = %s  port = %d........\n",
				socketId.ToString(false), socketId.port);
		}
		//
		const int nPacketSize = m_tcpClient.ReceiveSize(m_socketId);
		for(int i = 0; i < nPacketSize; ++i) {

			Packet* pPacket = m_tcpClient.Receive(m_socketId);
			if(NULL == pPacket) {
				break;
			}
			::node::DataPacket request;
			if(!request.ParseFromArray((char*)pPacket->data, pPacket->length)) {
				OutputError("!request.ParseFromArray");
			}
			m_tcpClient.DeallocatePacket(pPacket, m_socketId);

			::node::DataPacket response;
			SendInnerProtocol(request, response, m_socketId.index);
		}

        Sleep(1);
    }
	return false;
}

void CGameClient::RegistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//register module
	CAutoPointer<CLoginModule> pLoginModule(
		new CLoginModule(LOGIN_MODULE_NAME));
	pFacade->RegisterModule(pLoginModule);
}

void CGameClient::UnregistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//unregister module
	pFacade->RemoveModule(LOGIN_MODULE_NAME);
}

void CGameClient::RegisterCommand()
{
	CommandManager::PTR_T pCmdMgr(CommandManager::Pointer());
	CAutoPointer<MemberMethodRIP1<CGameClient> > pCommandDirectConnect(new MemberMethodRIP1<CGameClient>(s_instance, &CGameClient::CommandDirectConnect));
	pCmdMgr->AddCommand("direct-connect", pCommandDirectConnect, "direct connect proxy server. >> direct-connect <address> <account> <session key>");

	CAutoPointer<MemberMethodRIP1<CGameClient> > pCommandConnect(new MemberMethodRIP1<CGameClient>(s_instance, &CGameClient::CommandConnect));
	pCmdMgr->AddCommand("connect", pCommandConnect, "connect server. >> connect <userName> <password>");
}

bool CGameClient::SendToServer(int cmd, const ::google::protobuf::Message& message)
{
	::node::DataPacket packet;
	packet.set_cmd(cmd);

	if(!SerializeInnerData(packet, message)) {
		return false;
	}

	SmallBuffer smallbuf(packet.ByteSize());
	if(!packet.SerializeToArray((char*)smallbuf, packet.ByteSize())) {
		OutputError("!message.SerializeToArray");
		return false;
	}

	return m_tcpClient.Send((unsigned char*)(char*)smallbuf,
		packet.ByteSize(), m_socketId);
}

void CGameClient::LoginTimeout(const unsigned int& socketIdx)
{
    SocketID socketId;
    socketId.index = socketIdx;
    m_tcpClient.CloseConnection(socketId);
}

int CGameClient::CommandDirectConnect(const util::CWeakPointer<ArgumentBase>& arg)
{
	util::CWeakPointer<CommandArgument> pCommArg(arg);
    if(pCommArg.IsInvalid()) {
        return COMMAND_RESULT_FAIL;
    }

	char szAddress[eBUF_SIZE_128] = {'\0'};
	uint64_t u64Account = 0;
	uint64_t u64UserId = 0;
	char szSessionKey[eBUF_SIZE_256] = {'\0'};
	if(sscanf(pCommArg->GetParams().c_str(), "%s "I64FMTD" "I64FMTD" %s",
		szAddress, &u64Account, &u64UserId, szSessionKey) < 1)
	{
		return COMMAND_RESULT_FAIL;
	}

	LoginProxy(szAddress, u64Account, u64UserId, szSessionKey);

	return COMMAND_RESULT_SUCCESS;
}

int CGameClient::CommandConnect(const util::CWeakPointer<ArgumentBase>& arg)
{
	util::CWeakPointer<CommandArgument> pCommArg(arg);
	if(pCommArg.IsInvalid()) {
		return COMMAND_RESULT_FAIL;
	}

	if(pCommArg->GetParams().empty()) {
		LoginWeb(NULL, NULL);
	} else {
		char szUserName[eBUF_SIZE_128] = {'\0'};
		char szPassword[eBUF_SIZE_128] = {'\0'};
		if(sscanf(pCommArg->GetParams().c_str(), "%s %s", szUserName, szPassword) < 2) {
			return COMMAND_RESULT_FAIL;
		}

		LoginWeb(szUserName, szPassword);
	}
	return COMMAND_RESULT_SUCCESS;
}

void CGameClient::LoginWeb(const char* szUserName, const char* szPassword)
{
	WebLoginType loginType = WEB_LOGIN_ANONYMOUS;
	if(NULL != szUserName && NULL != szPassword) {
		loginType = WEB_LOGIN_USERANDPWD;
	}

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strEquipId(pConfig->GetString(APPCONFIG_EQUIPID));

	char szBuffer[eBUF_SIZE_1024] = {'\0'};

	if(WEB_LOGIN_USERANDPWD == loginType) {
		std::stringstream strStream;
		strStream << strEquipId << loginType << szUserName << szPassword << AUTH_KEY;
		MD5_CTX md5;
		std::string strAuthKey(md5.MakePassMD5(strStream.str()));

		snprintf(szBuffer, sizeof(szBuffer), "%s?equipmentId=%s&type=%d&username=%s&password=%s&authKey=%s",
			pConfig->GetString(APPCONFIG_LOGINWEB).c_str(),
			strEquipId.c_str(),
			loginType,
			szUserName,
			szPassword,
			strAuthKey.c_str());
	} else {
		std::stringstream strStream;
		strStream << strEquipId << loginType << AUTH_KEY;
		MD5_CTX md5;
		std::string strAuthKey(md5.MakePassMD5(strStream.str()));

		snprintf(szBuffer, sizeof(szBuffer), "%s?equipmentId=%s&type=%d&authKey=%s",
			pConfig->GetString(APPCONFIG_LOGINWEB).c_str(),
			strEquipId.c_str(),
			loginType,
			strAuthKey.c_str());
	}

	OutputBasic("URL = %s", szBuffer);

	CLoginWebRequest* pLoginSession = CLoginWebRequest::Create(szBuffer);
	CHttpClientManager::Pointer()->AddRequest(pLoginSession);
}

bool CGameClient::Connect(const char* szAddress, uint32_t u32PacketLimit)
{
	if(NULL == szAddress || strcmp(szAddress, "") == 0) {
		return false;
	}

	if(SOCKETID_INDEX_NULL != m_socketId.index) {
		m_tcpClient.Stop();
	}

	if(m_tcpClient.Connect(m_socketId, szAddress, u32PacketLimit)) {
		return true;
	}
	return false;
}

void CGameClient::LoginWebSuccess(const char* loginIp, uint64_t account, const char* sessionKey)
{
	if(NULL == sessionKey || NULL == loginIp) {
		printf("%s : NULL == sessionKey || NULL == loginIp \n", __FUNCTION__);
		return;
	}

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	uint32_t u32PacketLimit = pConfig->GetInt(APPCONFIG_ACCEPTPACKETLIMIT, MAX_PACKET_SIZE);

	if(Connect(loginIp, u32PacketLimit)) {
		printf("The client connect to login address = %s packet limit = %u \n", loginIp, u32PacketLimit);

		login::LoginRequest loginRequest;
		loginRequest.set_account(account);
		loginRequest.set_sessionkey(sessionKey);
		SendToServer(P_CMD_CTL_LOGIN, loginRequest);
	} else {
		printf("The client connect to login address = %s fail \n", loginIp);
	}
}

void CGameClient::LoginWebFail()
{
	printf("login web return fail!\n");
}

void CGameClient::LoginProxy(const char* proxyIp, uint64_t account, uint64_t userId, const char* sessionKey)
{
	if(NULL == sessionKey || NULL == proxyIp) {
		printf("%s : NULL == sessionKey || NULL == proxyIp \n", __FUNCTION__);
		return;
	}

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	uint32_t u32PacketLimit = pConfig->GetInt(APPCONFIG_ACCEPTPACKETLIMIT, MAX_PACKET_SIZE);
	int nVersion = pConfig->GetInt(APPCONFIG_VERSION);

	if(Connect(proxyIp, u32PacketLimit)) {
		printf("The client connect to proxy address = %s packet limit = %u \n", proxyIp, u32PacketLimit);

		::agent::LoginRequest loginRequest;
		loginRequest.set_version(nVersion);
		loginRequest.set_account(account);
		loginRequest.set_userid(userId);
		loginRequest.set_sessionkey(sessionKey);
		SendToServer(P_CMD_C_LOGIN, loginRequest);
	} else {
		printf("The client connect to proxy address = %s fail \n", proxyIp);
	}
}

