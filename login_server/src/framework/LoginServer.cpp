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
#include "LocalIDFactory.h"
#include "CommandManager.h"
#include "TimestampManager.h"
#include "ServerRegisterHelper.h"
#include "LoginPlayer.h"

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
		false, true) == (char)true) {
			return false;
	}

	if(argc > 0) {
		m_strProcessPath = argv[0];
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
		std::replace(m_strProcessPath.begin(), m_strProcessPath.end(), '\\', '/');
#endif
	} else {
		printf("Can't get process path.\n");
	}

    srand((unsigned)time(NULL));

    bool rc = true;
    // open log file
	
	std::string strLogPath;
	std::string::size_type idx = m_strProcessPath.find_last_of('/');
	if(std::string::npos != idx) {
		strLogPath = m_strProcessPath.substr(0, idx);
	} else {
		strLogPath = ".";
	}
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
	uint32_t uServerId = pConfig->GetInt(APPCONFIG_SERVERID);
	std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
	std::string strBind(pConfig->GetString(APPCONFIG_SERVERBIND));
	assert(!strBind.empty());
	std::string endPoint(pConfig->GetString(APPCONFIG_ENDPOINT));
	if(endPoint.empty()) {
		endPoint = strBind;
	}
	std::string strServant(pConfig->GetString(APPCONFIG_SERVANTCONNECT));
	uint16_t u16ServerRegoin = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERREGION);

	if(!m_pServerRegister.IsInvalid()) {
		// init status callback
		m_pServerRegister->InitStatusCallback();
		// init csv configure
		m_pServerRegister->InitTemplate();
		// register module
		m_pServerRegister->RegisterModule();
	}

	rpcz::application::set_connection_manager_threads(RpczThreads);
	rpcz::application::set_zmq_io_threads(ZmqioThreads);

	m_pServer.SetRawPointer(new rpcz::server);
	// register ControlCentreService
	m_pControlService.SetRawPointer(new CControlCentreServiceImp(u16ServerRegoin));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pControlService),
		m_pControlService->GetDescriptor()->name());
	// register WorkerService
	m_pWorkerService.SetRawPointer(new CLoginServiceImp(endPoint, strServant, strServerName, uServerId));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pWorkerService),
		m_pWorkerService->GetDescriptor()->name());

	m_pServer->bind(strBind);
	// start thread pool.
	ThreadPool.Startup(GameThreads);
	// connect server
	ConnectServers(strServerName, endPoint, uServerId, u16ServerRegoin);
	// connect control servant
	ConnectControlServant(strServant, strServerName, endPoint, uServerId, u16ServerRegoin);
	// start http thread
	ThreadPool.ExecuteTask(&m_httpThreadHold);
	// start thread
	ThreadPool.ExecuteTask(this);
	// tcp server
	std::string strAccept(pConfig->GetString(APPCONFIG_SERVERACCEPT));
	assert(!strAccept.empty());
	uint16_t nMaxLink = (uint16_t)pConfig->GetInt(APPCONFIG_ACCEPTMAXLINK, 1000);
	if(nMaxLink < 1) {
		nMaxLink = 1;
	}
	std::string strAcceptBind("0.0.0.0");
	std::string::size_type offset = strAccept.rfind(':');
	if (std::string::npos != offset) {
		strAcceptBind.append(strAccept, offset, strAccept.size() - offset);
	} else {
		assert(std::string::npos != offset);
	}
    uint32_t u32PacketLimit = pConfig->GetInt(APPCONFIG_ACCEPTPACKETLIMIT, MAX_PACKET_SIZE);
	m_tcpServer.Start(strAcceptBind.c_str(), nMaxLink, u32PacketLimit);
    printf("The server accept  address = %s maxlink = %hu packetlimit = %u \n", strAccept.c_str(),
        nMaxLink, u32PacketLimit);
	printf("The server started id = %u bind = %s endpoint = %s name = %s \n", 
		uServerId, strBind.c_str(), endPoint.c_str(), strServerName.c_str());
    return rc;
}

void CLoginServer::Dispose() {

	if(!m_isStarted) {
		return;
	}
	atomic_xchg(&g_serverStatus, SERVER_STATUS_IDLE);
	DisposeKeepRegTimer();
	printf("DisposeKeepRegTimer done! \n");
	if(!m_pServerRegister.IsInvalid()) {
		// unregister module
		m_pServerRegister->UnregisterModule();
	}
	printf("UnregistModule done! \n");
	// unregister control servant
	DisconnectControlServant();
	printf("DisconnectControlServant done! \n");
	// unregister this server
	DisconnectServers();
	printf("DisconnectServers done! \n");
	// dispose http thread hold
	m_httpThreadHold.Dispose();
	printf("m_httpThreadHold.Dispose done! \n");
	// set break the loop
	atomic_xchg8(&m_isStarted, false);
	// close game thread
	ThreadPool.Shutdown();
	printf("ThreadPool.Shutdown done! \n");
    // close timer event
	CTimerManager::Release();
	printf("CTimerManager::Release done! \n");
    // clear channel manager
    CChannelManager::Pointer()->Dispose();
	CChannelManager::Release();
	printf("CChannelManager::Pointer()->Dispose done! \n");
	// close command
	CommandManager::Release();
	printf("CommandManager::Release done! \n");
	// close timestamp
	CTimestampManager::Release();
	printf("CTimestampManager::Release done! \n");
	// close facade
	CFacade::Release();
	printf("CFacade::Release done! \n");
	// close tcp socket
	m_tcpServer.Stop();
	printf("m_tcpServer.Stop done! \n");
	// release service
	m_pServer.SetRawPointer(NULL);
	m_pWorkerService.SetRawPointer(NULL);
	m_pControlService.SetRawPointer(NULL);
	// log dispose
	LogRelease();
	printf("LogRelease done! \n");
	AppConfig::Release();
	printf("AppConfig::Release done! \n");
}

bool CLoginServer::OnRun() {

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	SocketID socketId;
	int nWhy;

    while(m_isStarted) {
		// timer
		pTMgr->Loop();
		// tcp
		do {
			if(m_tcpServer.HasLostConnection(socketId, nWhy)) {
				SocketID newSocketId;
				if(m_tcpServer.HasNewConnection(newSocketId)) {
					if(socketId == newSocketId) {
						break;
					}
					PrintBasic("lost ip address = %s  port = %d........\n",
						socketId.ToString(false), socketId.port);
					RemoveSocketId(socketId);

					socketId = newSocketId;
					PrintBasic("recv ip address = %s  port = %d........\n",
						socketId.ToString(false), socketId.port);
					InsertSocketId(socketId);
				} else {
					PrintBasic("lost ip address = %s  port = %d........\n",
						socketId.ToString(false), socketId.port);
					RemoveSocketId(socketId);
				}
			} else {
				if(m_tcpServer.HasNewConnection(socketId)){
					PrintBasic("recv ip address = %s  port = %d........\n",
						socketId.ToString(false), socketId.port);
					InsertSocketId(socketId);
				}
			}
		} while(false);
		//
		const int nPacketSize = m_tcpServer.ReceiveSize();
		for(int i = 0; i < nPacketSize; ++i) {
			ReceivePacket* pRecievePacket = m_tcpServer.Receive();
			if(NULL == pRecievePacket) {
				break;
			}
			CLoginPlayer loginPlayer(pRecievePacket->socketId);
			util::CAutoPointer<CPlayerBase> pPlayer(&loginPlayer, false);
			::node::DataPacket request;
			if(!request.ParseFromArray((char*)pRecievePacket->packet.data,
				pRecievePacket->packet.length)) {
				OutputError("!request.ParseFromArray socketId = %s ", pRecievePacket->socketId.ToString());
				m_tcpServer.DeallocatePacket(pRecievePacket);
				continue;
			}
			::node::DataPacket response;
			SendWorkerProtocol(request, response, pPlayer);
			if(response.result() != SERVER_NO_RESPOND) {
				response.set_cmd(request.cmd());
				SendToClient(pRecievePacket->socketId, response);
			}
			m_tcpServer.DeallocatePacket(pRecievePacket);
		}
        Sleep(6);
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
	const std::string& endPoint,
	uint32_t uServerId,
	uint16_t u16ServerRegion)
{
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	CControlCentreStubImp::PTR_T pCtrlCenStubImp(CControlCentreStubImp::Pointer());
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	std::string strConnect;
	char szBuff[MAX_PATH];
	for(int i = 0; ; ++i) {
		sprintf(szBuff, APPCONFIG_SERVERCONNECT "%d", i);
		strConnect = pConfig->GetString(szBuff);
		if(strConnect.empty()) {
			break;
		}

		printf("Register to \"%s\" please wait... \n", strConnect.c_str());
		int nResult = pCtrlCenStubImp->RegisterServer(strConnect, strServerName,
			endPoint, uServerId, u16ServerRegion, CALL_REGISTER_MS);

		if(nResult < CSR_TIMEOUT) {
			OutputError("[connect = %s, name = %s, endPint = %s,"
				" errorCode = %d] Can't Register this server!", strConnect.c_str(),
				strServerName.c_str(), endPoint.c_str(), nResult);
			printf("Register to \"%s\" fail. %d \n", strConnect.c_str(), nResult);
			continue;
		} else if (nResult == CSR_TIMEOUT) {
			printf("Register to \"%s\" timeout. \n", strConnect.c_str());
		} else {
			printf("Register to \"%s\" success. \n", strConnect.c_str());
		}

		uint64_t timerKey = CLocalIDFactory::Pointer()->GenerateID();
		m_keepRegTimerKeys.push_back(timerKey);

		CAutoPointer<CallbackMFnP3<CLoginServer, std::string, volatile bool, volatile long> >
			callback(new CallbackMFnP3<CLoginServer, std::string, volatile bool, volatile long>
			(s_instance, &CLoginServer::KeepServersRegister, strConnect, false, 0));
		pTMgr->SetInterval(timerKey, KEEP_REGISTER_INTERVAL, callback);
	}
}

void CLoginServer::DisconnectServers()
{
	CControlCentreStubImp::PTR_T pCtrlCenStubImp(CControlCentreStubImp::Pointer());

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
	uint32_t uServerId = pConfig->GetInt(APPCONFIG_SERVERID);
	std::string strConnect;
	char szBuff[MAX_PATH];
	for(int i = 0; ; ++i) {
		sprintf(szBuff, APPCONFIG_SERVERCONNECT "%d", i);
		strConnect = pConfig->GetString(szBuff);
		if(strConnect.empty()) {
			break;
		}

		int nResult = pCtrlCenStubImp->UnregisterServer(strConnect, strServerName,
			uServerId, CALL_UNREGISTER_MS);

		if(nResult < CSR_TIMEOUT && CSR_WITHOUT_THIS_MODULE != nResult) {
			OutputError("[connect = %s, name = %s, errorCode = %d]"
                " Can't Unregister this server!", strConnect.c_str(),
				strServerName.c_str(), nResult);
			continue;
		}
	}
}

void CLoginServer::KeepServersRegister(
	std::string& connect,
	volatile bool& bRun,
	volatile long& nTimeoutCount)
{
	if(atomic_cmpxchg8(&bRun, false, true) == (char)false) {
		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		uint32_t uServerId = pConfig->GetInt(APPCONFIG_SERVERID);

		CControlCentreStubImp::PTR_T pCtrlCenStubImp(CControlCentreStubImp::Pointer());
		int nResult = pCtrlCenStubImp->KeepRegister(connect, uServerId);
		if(CSR_NOT_FOUND == nResult) {
			std::string endPoint(pConfig->GetString(APPCONFIG_ENDPOINT));
			if(endPoint.empty()) {
				endPoint = pConfig->GetString(APPCONFIG_SERVERBIND);
			}
			std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
			uint16_t u16ServerRegion = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERREGION);

			nResult = pCtrlCenStubImp->RegisterServer(connect, strServerName, endPoint,
				uServerId, u16ServerRegion);
			if (CSR_SUCCESS_AND_START == nResult || CSR_SUCCESS == nResult) {
				atomic_xchg(&nTimeoutCount, 0);
			}
		} else if (CSR_TIMEOUT == nResult) {
			if (atomic_inc(&nTimeoutCount) >= TIMEOUT_MAX_TIMES_REMOVE_SERVER) {
				CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
				pChlMgr->RemoveRpczChannel(connect);
			}
		} else {
			atomic_xchg(&nTimeoutCount, 0);
		}
		atomic_xchg8(&bRun, false);
	}
}

void CLoginServer::ConnectControlServant(
	const std::string& strServant,
	const std::string& strServerName,
	const std::string& endPoint,
	uint32_t uServerId,
	uint16_t u16ServerRegion)
{
	if(!strServant.empty()) {

		printf("Register to Control Servant \"%s\" please wait... \n", strServant.c_str());
		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strProjectName(pConfig->GetString(APPCONFIG_PROJECTNAME));
		std::string strAcceptAddress(pConfig->GetString(APPCONFIG_SERVERACCEPT));

		CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
		int nResult = pCtrlCenStubImpEx->RegisterServer(strServant, strServerName,
            endPoint, uServerId, u16ServerRegion, strProjectName, strAcceptAddress,
			m_strProcessPath, CALL_REGISTER_MS);

		if(nResult < CSR_TIMEOUT && CSR_CHANNEL_ALREADY_EXIST != nResult) {
			OutputError("[connect = %s, name = %s, endPint = %s,"
				" errorCode = %d] Can't Register this server!", strServant.c_str(),
				strServerName.c_str(), endPoint.c_str(), nResult);
			printf("Register to Control Servant \"%s\" fail. \n", strServant.c_str());
			return;
		} else if (nResult == CSR_TIMEOUT) {
			printf("Register to Control Servant \"%s\" timeout. %d \n", strServant.c_str(), nResult);
		} else {
			printf("Register to Control Servant \"%s\" success. %d \n", strServant.c_str(), nResult);
		}

		if(CSR_SUCCESS_AND_START == nResult) {
			atomic_xchg(&g_serverStatus, SERVER_STATUS_START);
		}

		CAutoPointer<CallbackMFnP2<CLoginServer, std::string, volatile bool> >
			callback(new CallbackMFnP2<CLoginServer, std::string, volatile bool>
			(s_instance, &CLoginServer::KeepControlServantRegister, strServant, false));

		uint64_t timerKey = CLocalIDFactory::Pointer()->GenerateID();
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
		uint32_t uServerId = pConfig->GetInt(APPCONFIG_SERVERID);

		CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
		int nResult = pCtrlCenStubImpEx->UnregisterServer(strServant, strServerName,
			uServerId, CALL_UNREGISTER_MS);

		if(nResult < CSR_TIMEOUT && CSR_WITHOUT_THIS_MODULE != nResult) {
			OutputError("[connect = %s, name = %s, errorCode = %d]"
                " Can't Unregister this server!", strServant.c_str(),
				strServerName.c_str(), nResult);
			return;
		}
	}
}

void CLoginServer::KeepControlServantRegister(std::string& connect, volatile bool& bRun)
{
	if(atomic_cmpxchg8(&bRun, false, true) == (char)false) {
		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
		uint32_t uServerId = pConfig->GetInt(APPCONFIG_SERVERID);

		CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
		int nResult = pCtrlCenStubImpEx->KeepRegister(connect, strServerName, uServerId,
			g_serverStatus, m_tcpServer.GetLinkCount());

		if(CSR_NOT_FOUND == nResult) {
			std::string endPoint(pConfig->GetString(APPCONFIG_ENDPOINT));
			if(endPoint.empty()) {
				endPoint = pConfig->GetString(APPCONFIG_SERVERBIND);
			}
			std::string strProjectName(pConfig->GetString(APPCONFIG_PROJECTNAME));
			uint16_t u16ServerRegion = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERREGION);
			std::string strAcceptAddress(pConfig->GetString(APPCONFIG_SERVERACCEPT));

			nResult = pCtrlCenStubImpEx->RegisterServer(connect, strServerName, endPoint, uServerId,
			    u16ServerRegion, strProjectName, strAcceptAddress, m_strProcessPath);
		}
		atomic_xchg8(&bRun, false);
	}
}

bool CLoginServer::SendToClient(uint64_t account, const ::node::DataPacket& message)
{
	SocketID socketId;
	if(!FindSocketId(account, socketId)) {
		OutputError("!FindSocketId()");
		return false;
	}

	const_cast<::node::DataPacket&>(message).clear_route();
	const_cast<::node::DataPacket&>(message).clear_route_type();

	SmallBuffer smallbuf(message.ByteSizeLong());
	if(!message.SerializeToArray((char*)smallbuf, message.ByteSizeLong())) {
		OutputError("!pMessage->SerializeToArray");
		return false;
	}

    CScopedLock scopedLock(m_tcpSpinLock);
	return m_tcpServer.Send((unsigned char*)(char*)smallbuf,
		message.ByteSizeLong(), socketId);
}

bool CLoginServer::SendToClientBySocketID(const ntwk::SocketID& socketId, const ::node::DataPacket& message)
{
	const_cast<::node::DataPacket&>(message).clear_route();
	const_cast<::node::DataPacket&>(message).clear_route_type();

	SmallBuffer smallbuf(message.ByteSizeLong());
	if(!message.SerializeToArray((char*)smallbuf, message.ByteSizeLong())) {
		OutputError("!message.SerializeToArray");
		return false;
	}

    CScopedLock scopedLock(m_tcpSpinLock);
	return m_tcpServer.Send((unsigned char*)(char*)smallbuf,
		message.ByteSizeLong(), socketId);
}


bool CLoginServer::InsertSocketId(const ntwk::SocketID& socketId, uint64_t account) {
	CScopedWriteLock wrlock(m_socketIdsLock);

    SOCKETID_TO_CLIENT_T::iterator itSU = m_clients.find(socketId);
    if(m_clients.end() == itSU) {
        return false;
    }
    m_socketIds[account] = itSU->first;
	itSU->second.account = account;

    uint64_t timerId = itSU->second.timerId;
    if(ID_NULL != timerId) {
        itSU->second.timerId = ID_NULL;
        CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
        pTMgr->Remove(timerId);

		printf("CLoginServer::InsertSocketId remove  socketId = %s timerId = " I64FMTD " account = " I64FMTD " \n", socketId.ToString(), timerId, account);
    }
    return true;
}

void CLoginServer::InsertSocketId(const SocketID& socketId) {
	CScopedWriteLock wrlock(m_socketIdsLock);
    struct ClientData client;
    client.account = DEFAULT_ACCOUNT;
    client.timerId = CLocalIDFactory::Pointer()->GenerateID();
	m_clients[socketId] = client;
    CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
    CAutoPointer<CallbackMFnP2<CLoginServer, ntwk::SocketID, int> >
        callback(new CallbackMFnP2<CLoginServer, ntwk::SocketID, int>
        (s_instance, &CLoginServer::LoginTimeout, socketId, LOGOUT_LOGIN_TIMEOUT));
    pTMgr->SetTimeout(client.timerId, LOGIN_EXPIRY_TIME, callback);
	printf("CLoginServer::InsertSocketId add  socketId = %s timerId = " I64FMTD " \n", socketId.ToString(), client.timerId);
}

uint64_t CLoginServer::RemoveSocketId(const SocketID& socketId) {
	CScopedWriteLock wrlock(m_socketIdsLock);
    uint64_t outAccount(DEFAULT_ACCOUNT);
	SOCKETID_TO_CLIENT_T::iterator itSU = m_clients.find(socketId);
	if(m_clients.end() != itSU) {
		outAccount = itSU->second.account;
        uint64_t timerId = itSU->second.timerId;
		m_socketIds.erase(itSU->second.account);
        m_clients.erase(itSU);
        if(ID_NULL != timerId) {
            CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
            pTMgr->Remove(timerId);
        }
	}
	return outAccount;
}

bool CLoginServer::FindSocketId(uint64_t account, SocketID& outSocketId) {
	CScopedReadLock rdlock(m_socketIdsLock);
	ACCOUNT_TO_SOCKETID_T::iterator itUS = m_socketIds.find(account);
	if(m_socketIds.end() == itUS) {
		return false;
	}
	outSocketId = itUS->second;
	return true;
}

bool CLoginServer::FindAccount(const ntwk::SocketID& socketId, uint64_t& outAccount)
{
	CScopedReadLock rdlock(m_socketIdsLock);

	SOCKETID_TO_CLIENT_T::iterator itSU = m_clients.find(socketId);
	if(m_clients.end() == itSU) {
		return false;
	}
	outAccount = itSU->second.account;
	return true;
}

bool CLoginServer::SendToClient(const SocketID& socketId,
	const ::node::DataPacket& message)
{
	const_cast<::node::DataPacket&>(message).clear_route();
	const_cast<::node::DataPacket&>(message).clear_route_type();

	SmallBuffer smallbuf(message.ByteSizeLong());
	if(!message.SerializeToArray((char*)smallbuf, message.ByteSizeLong())) {
		OutputError("!message.SerializeToArray");
		return false;
	}

    CScopedLock scopedLock(m_tcpSpinLock);
	return m_tcpServer.Send((unsigned char*)(char*)smallbuf,
		message.ByteSizeLong(), socketId);
}

bool CLoginServer::IsLoggedAccount(const ntwk::SocketID& socketId) {
	CScopedReadLock rdlock(m_socketIdsLock);

	SOCKETID_TO_CLIENT_T::iterator itSU = m_clients.find(socketId);
	if(m_clients.end() == itSU) {
		return false;
	}
	return itSU->second.account != DEFAULT_ACCOUNT;
}

uint64_t CLoginServer::GetLoggedAccount(const ntwk::SocketID& socketId) {
	CScopedReadLock rdlock(m_socketIdsLock);

	SOCKETID_TO_CLIENT_T::iterator itSU = m_clients.find(socketId);
	if (m_clients.end() == itSU) {
		return false;
	}
	return itSU->second.account;
}

void CLoginServer::LoginTimeout(ntwk::SocketID& socketId, int& nWhy)
{
    m_tcpServer.CloseConnection(socketId, nWhy);

	printf("CLoginServer::LoginTimeout  socketId = %s \n", socketId.ToString());
}

void CLoginServer::CloseSocket(ntwk::SocketID& socketId, int nWhy)
{
	m_tcpServer.CloseConnection(socketId, nWhy);
}

