/*
 * File:   LoginServer.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */
#include <zmq.hpp>
#include "Log.h"
#include "TimerManager.h"
#include "NodeDefines.h"
#include "AppConfig.h"
#include "LoginServer.h"
#include "ModuleManager.h"
#include "CacheOperateHelper.h"
#include "WorkerOperateHelper.h"
#include "NetworkTypes.h"
#include "BodyBitStream.h"
#include "ControlCentreStubImp.h"
#include "ControlCentreStubImpEx.h"
#include "ProtoRpczServiceImpl.h"
#include "GuidFactory.h"
#include "HttpClientManager.h"
#include "CommandManager.h"
#include "TimestampManager.h"
#include "ServerRegisterHelper.h"
#include "dbstl_common.h"

using namespace mdl;
using namespace evt;
using namespace ntwk;
using namespace util;
using namespace thd;

void LoadAppConfig() {
	AppConfig::REVISE_SET_T reviseSet;
	reviseSet[APPCONFIG_SERVERBIND] = ReviseAddress;
	reviseSet[APPCONFIG_SERVANTCONNECT] = ReviseAddress;
	reviseSet[APPCONFIG_SERVERCONNECT] = ReviseAddress;

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	if (!pConfig->LoadFile((char*)APP_CONFIG_NAME, reviseSet)) {
		fprintf(stderr, "Cannot load the %s file\n", (char*)APP_CONFIG_NAME);
		assert(false);
	}
}

CLoginServer::CLoginServer()
	: m_isStarted(false)
	, m_pServerRegister(CreateServerRegister())
{
}

CLoginServer::~CLoginServer() {
    Dispose();
}

bool CLoginServer::Init(int argc, char** argv) {

	if(atomic_cmpxchg8(&m_isStarted,
		true, false) == (char)true) {
			return false;
	}

	if(argc > 0) {
		m_strProcessPath = argv[0];
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
		std::replace(m_strProcessPath.begin(), m_strProcessPath.end(), '\\', '/');
#endif
	} else {
		printf("Can't get process path.\n");
	}

    srand((unsigned)time(NULL));

    bool rc = true;
    // open log file
	
	std::string strCurPath;
	std::string::size_type idx = m_strProcessPath.find_last_of('/');
	if(std::string::npos != idx) {
		strCurPath = m_strProcessPath.substr(0, idx);
	} else {
		strCurPath = ".";
	}
	std::string strLogPath(strCurPath);
	strLogPath += "/log/";
	LogInit(5, strLogPath.c_str());
    PrintBasic("%s %s %s", CONFIG, PLATFORM_TEXT, ARCH);
    PrintError("%s %s %s", CONFIG, PLATFORM_TEXT, ARCH);
    // read config file
	AppConfig::PTR_T pConfig(AppConfig::Pointer());

	int RpczThreads = pConfig->GetInt(APPCONFIG_RPCZTHREADS);
	if(RpczThreads < 1) {
		RpczThreads = 1;
	}
	int GameThreads = pConfig->GetInt(APPCONFIG_GAMETHREADS);
	if(GameThreads < 10) {
		GameThreads = 10;
	}
	int ZmqioThreads = pConfig->GetInt(APPCONFIG_ZMQIOTHREADS);
	if(ZmqioThreads < 1) {
		ZmqioThreads = 1;
	}
	uint16_t u16ServerId = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERID);
	std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
	std::string strBind(pConfig->GetString(APPCONFIG_SERVERBIND));
	std::string strServant(pConfig->GetString(APPCONFIG_SERVANTCONNECT));
	uint16_t u16ServerRegoin = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERREGION);
	// must have server.
	assert(!strBind.empty());
	// set the config

	CGuidFactory::Pointer()->SetCodeInt16(u16ServerId);

	if(!m_pServerRegister.IsInvalid()) {
		// init status callback
		m_pServerRegister->InitStatusCallback();
		// register module
		m_pServerRegister->RegistModule();
	}
	// startup Berkeley DB
	dbstl::dbstl_startup();

	rpcz::application::set_connection_manager_threads(RpczThreads);
	rpcz::application::set_zmq_io_threads(ZmqioThreads);

	m_pServer.SetRawPointer(new rpcz::server);
	// register ControlCentreService
	m_pControlService.SetRawPointer(new CControlCentreServiceImp(
		u16ServerRegoin, u16ServerId, strCurPath));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pControlService),
		m_pControlService->GetDescriptor()->name());
	// register WorkerService
	m_pWorkerService.SetRawPointer(new CLoginServiceImp(strBind, strServant, strServerName, u16ServerId));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pWorkerService),
		m_pWorkerService->GetDescriptor()->name());

	m_pServer->bind(strBind);
	// start thread pool.
	ThreadPool.Startup(GameThreads);
	// connect server
	ConnectServers(strServerName, strBind, u16ServerId, u16ServerRegoin);
	// connect control servant
	ConnectControlServant(strServant, strServerName, strBind, u16ServerId, u16ServerRegoin);
	// start thread
	ThreadPool.ExecuteTask(this);
	// tcp server
	std::string strAccept(pConfig->GetString(APPCONFIG_SERVERACCEPT));
	assert(!strAccept.empty());
	uint16_t nMaxLink = (uint16_t)pConfig->GetInt(APPCONFIG_ACCEPTMAXLINK, 1000);
	if(nMaxLink < 1) {
		nMaxLink = 1;
	}
    uint32_t u32PacketLimit = pConfig->GetInt(APPCONFIG_ACCEPTPACKETLIMIT, MAX_PACKET_SIZE);
	m_tcpServer.Start(strAccept.c_str(), nMaxLink, u32PacketLimit);
    printf("The server accept  address = %s maxlink = %hu packetlimit = %u \n", strAccept.c_str(),
        nMaxLink, u32PacketLimit);
	printf("The server bind id = %d address = %s name = %s \n", u16ServerId, strBind.c_str(), strServerName.c_str());
    return rc;
}

void CLoginServer::Dispose() {

	if(!m_isStarted) {
		return;
	}
	atomic_xchg(&g_serverStatus, SERVER_STATUS_IDLE);
	DisposeKeepRegTimer();

	if(!m_pServerRegister.IsInvalid()) {
		// unregister module
		m_pServerRegister->UnregistModule();
	}
	// unregister control servant
	DisconnectControlServant();
	// unregister this server
	DisconnectServers();
	// set break the loop
	atomic_xchg8(&m_isStarted, false);
	// close game thread
	ThreadPool.Shutdown();
    // close timer event
	CTimerManager::Release();
    // clear channel manager
    CChannelManager::Pointer()->Dispose();
	// close command
	CommandManager::Release();
	// close timestamp
	CTimestampManager::Release();
	// close facade
	CFacade::Release();
	// close tcp socket
	m_tcpServer.Stop();
	// release service
	m_pServer.SetRawPointer(NULL);
	m_pWorkerService.SetRawPointer(NULL);
	m_pControlService.SetRawPointer(NULL);
	// log dispose
	LogRelease();
	// exit Berkeley DB
	dbstl::dbstl_exit();
}

bool CLoginServer::Run() {

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	CFacade::PTR_T pFacade(CFacade::Pointer());
	SocketID socketId;

	CHttpClientManager::PTR_T pHttpClientMgr(CHttpClientManager::Pointer());

    while(m_isStarted) {
		// timer
		pTMgr->Loop();
		// http
		pHttpClientMgr->Run();
		// tcp
		if(m_tcpServer.HasNewConnection(socketId)){
			PrintBasic("recv ip address = %s  port = %d........\n",
				socketId.ToString(false), socketId.port);
			InsertSocketId(socketId);
		}
		//
		if(m_tcpServer.HasLostConnection(socketId)){
			PrintBasic("lost ip address = %s  port = %d........\n",
				socketId.ToString(false), socketId.port);
			RemoveSocketId(socketId);
		}
		//
		const int nPacketSize = m_tcpServer.ReceiveSize();
		for(int i = 0; i < nPacketSize; ++i) {
			ReceivePacket* pRecievePacket = m_tcpServer.Receive();
			if(NULL == pRecievePacket) {
				break;
			}
			::node::DataPacket request;
			if(!request.ParseFromArray((char*)pRecievePacket->packet.data,
				pRecievePacket->packet.length)) {
				OutputError("!request.ParseFromArray");
				continue;
			}
			::node::DataPacket response;
			SendWorkerProtocol(request, response, pRecievePacket->socketId.index);
			if(response.has_result() || response.has_data()) {
				response.set_cmd(request.cmd());
				SendToClient(socketId, response);
			}

			m_tcpServer.DeallocatePacket(pRecievePacket);
		}

        Sleep(1);
    }
	return false;
}

void CLoginServer::DisposeKeepRegTimer()
{
	if(m_keepRegTimerKeys.empty()) {
		return;
	}
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	INTERVAL_KEYS_T::iterator it(m_keepRegTimerKeys.begin());
	for(;m_keepRegTimerKeys.end() != it; ++it) {
		pTMgr->Remove(*it, true);
	}
	m_keepRegTimerKeys.clear();
}

void CLoginServer::ConnectServers(
	const std::string& strServerName,
	const std::string& strBind,
	uint16_t u16ServerId,
	uint16_t u16ServerRegion)
{
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	CControlCentreStubImp::PTR_T pCtrlCenStubImp(CControlCentreStubImp::Pointer());
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	std::string strConnect;
	char szBuff[MAX_PATH];
	for(int i = 0; ; ++i) {
		sprintf(szBuff, APPCONFIG_SERVERCONNECT"%d", i);
		strConnect = pConfig->GetString(szBuff);
		if(strConnect.empty()) {
			break;
		}

		printf("Register to \"%s\" please wait... \n", strConnect.c_str());
		int nResult = pCtrlCenStubImp->RegisterServer(strConnect, strServerName,
			strBind, u16ServerId, u16ServerRegion, CALL_REGISTER_MS);

		if(nResult < CSR_SUCCESS) {
			OutputError("[connect = %s, name = %s, endPint = %s,"
				" errorCode = %d] Can't Register this server!", strConnect.c_str(),
				strServerName.c_str(), strBind.c_str(), nResult);
			printf("Register to \"%s\" fail. \n", strConnect.c_str());
			continue;
		}
		printf("Register to \"%s\" success. \n", strConnect.c_str());

		uint64_t timerKey = CGuidFactory::Pointer()->CreateGuid();
		m_keepRegTimerKeys.push_back(timerKey);

		CAutoPointer<CallbackMFnP2<CLoginServer, std::string, volatile bool> >
			callback(new CallbackMFnP2<CLoginServer, std::string, volatile bool>
			(s_instance, &CLoginServer::KeepServersRegister, strConnect, false));
		pTMgr->SetInterval(timerKey, KEEP_REGISTER_INTERVAL, callback);
	}
}

void CLoginServer::DisconnectServers()
{
	CControlCentreStubImp::PTR_T pCtrlCenStubImp(CControlCentreStubImp::Pointer());

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
	uint16_t u16ServerId = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERID);
	std::string strConnect;
	char szBuff[MAX_PATH];
	for(int i = 0; ; ++i) {
		sprintf(szBuff, APPCONFIG_SERVERCONNECT"%d", i);
		strConnect = pConfig->GetString(szBuff);
		if(strConnect.empty()) {
			break;
		}

		int nResult = pCtrlCenStubImp->UnregisterServer(strConnect, strServerName,
			u16ServerId, CALL_UNREGISTER_MS);

		if(nResult < CSR_SUCCESS && CSR_WITHOUT_THIS_MODULE != nResult && CSR_TIMEOUT != nResult) {
			OutputError("[connect = %s, name = %s, errorCode = %d]"
                " Can't Unregister this server!", strConnect.c_str(),
				strServerName.c_str(), nResult);
			continue;
		}
	}
}

void CLoginServer::KeepServersRegister(std::string& connect, volatile bool& bRun)
{
	if(atomic_cmpxchg8(&bRun, true, false) == false) {
		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		uint16_t u16ServerId = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERID);

		CControlCentreStubImp::PTR_T pCtrlCenStubImp(CControlCentreStubImp::Pointer());
		if(pCtrlCenStubImp->KeepRegister(connect, u16ServerId)) {
			std::string strBind(pConfig->GetString(APPCONFIG_SERVERBIND));
			std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
			uint16_t u16ServerRegion = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERREGION);

			pCtrlCenStubImp->RegisterServer(connect, strServerName, strBind,
				u16ServerId, u16ServerRegion);
		}
		atomic_xchg8(&bRun, false);
	}
}

void CLoginServer::ConnectControlServant(
	const std::string& strServant,
	const std::string& strServerName,
	const std::string& strBind,
	uint16_t u16ServerId,
	uint16_t u16ServerRegion)
{
	if(!strServant.empty()) {

		printf("Register to Control Servant \"%s\" please wait... \n", strServant.c_str());
		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strProjectName(pConfig->GetString(APPCONFIG_PROJECTNAME));
		std::string strAcceptAddress(pConfig->GetString(APPCONFIG_SERVERACCEPT));

		CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
		int nResult = pCtrlCenStubImpEx->RegisterServer(strServant, strServerName,
            strBind, u16ServerId, u16ServerRegion, strProjectName, strAcceptAddress,
			m_strProcessPath, CALL_REGISTER_MS);

		if(nResult < CSR_SUCCESS && CSR_CHANNEL_ALREADY_EXIST != nResult) {
			OutputError("[connect = %s, name = %s, endPint = %s,"
				" errorCode = %d] Can't Register this server!", strServant.c_str(),
				strServerName.c_str(), strBind.c_str(), nResult);
			printf("Register to Control Servant \"%s\" fail. \n", strServant.c_str());
			return;
		}
		printf("Register to Control Servant \"%s\" success. \n", strServant.c_str());
		if(CSR_SUCCESS_AND_START == nResult) {
			atomic_xchg(&g_serverStatus, SERVER_STATUS_START);
		}

		CAutoPointer<CallbackMFnP2<CLoginServer, std::string, volatile bool> >
			callback(new CallbackMFnP2<CLoginServer, std::string, volatile bool>
			(s_instance, &CLoginServer::KeepControlServantRegister, strServant, false));

		uint64_t timerKey = CGuidFactory::Pointer()->CreateGuid();
		m_keepRegTimerKeys.push_back(timerKey);

		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		pTMgr->SetInterval(timerKey, KEEP_REGISTER_INTERVAL, callback);
	}
}

void CLoginServer::DisconnectControlServant()
{
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strServant(pConfig->GetString(APPCONFIG_SERVANTCONNECT));
	if(!strServant.empty()) {

		std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
		uint16_t u16ServerId = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERID);

		CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
		int nResult = pCtrlCenStubImpEx->UnregisterServer(strServant, strServerName,
			u16ServerId, CALL_UNREGISTER_MS);

		if(nResult < CSR_SUCCESS && CSR_WITHOUT_THIS_MODULE != nResult && CSR_TIMEOUT != nResult) {
			OutputError("[connect = %s, name = %s, errorCode = %d]"
                " Can't Unregister this server!", strServant.c_str(),
				strServerName.c_str(), nResult);
			return;
		}
	}
}

void CLoginServer::KeepControlServantRegister(std::string& connect, volatile bool& bRun)
{
	if(atomic_cmpxchg8(&bRun, true, false) == false) {
		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
		uint16_t u16ServerId = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERID);

		CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
		if(pCtrlCenStubImpEx->KeepRegister(connect, strServerName, u16ServerId,
			g_serverStatus, m_tcpServer.GetLinkCount())) {

				std::string strBind(pConfig->GetString(APPCONFIG_SERVERBIND));
				std::string strProjectName(pConfig->GetString(APPCONFIG_PROJECTNAME));
				uint16_t u16ServerRegion = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERREGION);

				pCtrlCenStubImpEx->RegisterServer(connect, strServerName, strBind, u16ServerId,
					u16ServerRegion, strProjectName, std::string(), m_strProcessPath);
		}
		atomic_xchg8(&bRun, false);
	}
}

bool CLoginServer::SendToClient(uint64_t userId, const ::node::DataPacket& message)
{
	SocketID socketId;
	if(!FindSocketId(userId, socketId)) {
		return false;
	}

	const_cast<::node::DataPacket&>(message).clear_route();
	const_cast<::node::DataPacket&>(message).clear_route_type();

	SmallBuffer smallbuf(message.ByteSize());
	if(!message.SerializeToArray((char*)smallbuf, message.ByteSize())) {
		OutputError("!pMessage->SerializeToArray");
		return false;
	}

    CScopedLock scopedLock(m_tcpSpinLock);
	return m_tcpServer.Send((unsigned char*)(char*)smallbuf,
		message.ByteSize(), socketId);
}

bool CLoginServer::SendToClientByIdx(unsigned int socketIdx, const ::node::DataPacket& message)
{
	const_cast<::node::DataPacket&>(message).clear_route();
	const_cast<::node::DataPacket&>(message).clear_route_type();

	SmallBuffer smallbuf(message.ByteSize());
	if(!message.SerializeToArray((char*)smallbuf, message.ByteSize())) {
		OutputError("!message.SerializeToArray");
		return false;
	}

    CScopedLock scopedLock(m_tcpSpinLock);
	return m_tcpServer.Send((unsigned char*)(char*)smallbuf,
		message.ByteSize(), socketIdx);
}


bool CLoginServer::InsertSocketId(uint64_t userId, int socketIdx) {
	CScopedWriteLock wrlock(m_socketIdsLock);
    SocketID socketId;
    socketId.index = socketIdx;
    SOCKETID_TO_CLIENT_T::iterator itSU = m_clients.find(socketId);
    if(m_clients.end() == itSU) {
        return false;
    }
    m_socketIds[userId] = itSU->first;
    uint64_t timerId = itSU->second.timerId;
    if(ID_NULL != timerId) {
        itSU->second.timerId = ID_NULL;
        CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
        pTMgr->Remove(timerId);
    }
    return true;
}

void CLoginServer::InsertSocketId(const SocketID& socketId) {
	CScopedWriteLock wrlock(m_socketIdsLock);
    struct ClientData client;
    client.userId = -1;
    client.timerId = CGuidFactory::Pointer()->CreateGuid();
	m_clients[socketId] = client;
    CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
    CAutoPointer<CallbackMFnP1<CLoginServer, unsigned int> >
        callback(new CallbackMFnP1<CLoginServer, unsigned int>
        (s_instance, &CLoginServer::LoginTimeout, socketId.index));
    pTMgr->SetTimeout(client.timerId, LOGIN_EXPIRY_TIME, callback);
}

uint64_t CLoginServer::RemoveSocketId(const SocketID& socketId) {
	CScopedWriteLock wrlock(m_socketIdsLock);
    uint64_t outUserId(DEFAULT_USERID);
	SOCKETID_TO_CLIENT_T::iterator itSU = m_clients.find(socketId);
	if(m_clients.end() != itSU) {
        outUserId = itSU->second.userId;
        uint64_t timerId = itSU->second.timerId;
		m_socketIds.erase(itSU->second.userId);
        m_clients.erase(itSU);
        if(ID_NULL != timerId) {
            CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
            pTMgr->Remove(timerId);
        }
	}
	return outUserId;
}

bool CLoginServer::FindSocketId(uint64_t userId, SocketID& outSocketId) {
	CScopedReadLock rdlock(m_socketIdsLock);
	USERID_TO_SOCKETID_T::iterator itUS = m_socketIds.find(userId);
	if(m_socketIds.end() == itUS) {
		return false;
	}
	outSocketId = itUS->second;
	return true;
}

bool CLoginServer::FindUserId(unsigned int socketIdx, uint64_t& outUserId)
{
	CScopedReadLock rdlock(m_socketIdsLock);
	SocketID socketId;
	socketId.index = socketIdx;
	SOCKETID_TO_CLIENT_T::iterator itSU = m_clients.find(socketId);
	if(m_clients.end() == itSU) {
		return false;
	}
	outUserId = itSU->second.userId;
	return true;
}

bool CLoginServer::SendToClient(const SocketID& socketId,
	const ::node::DataPacket& message)
{
	const_cast<::node::DataPacket&>(message).clear_route();
	const_cast<::node::DataPacket&>(message).clear_route_type();

	SmallBuffer smallbuf(message.ByteSize());
	if(!message.SerializeToArray((char*)smallbuf, message.ByteSize())) {
		OutputError("!message.SerializeToArray");
		return false;
	}

    CScopedLock scopedLock(m_tcpSpinLock);
	return m_tcpServer.Send((unsigned char*)(char*)smallbuf,
		message.ByteSize(), socketId);
}

void CLoginServer::LoginTimeout(unsigned int& socketIdx)
{
    SocketID socketId;
    socketId.index = socketIdx;
    m_tcpServer.CloseConnection(socketId);
}

