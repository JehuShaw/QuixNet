/*
 * File:   AgentServer.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_10_7 PM 11:45
 */
#include <zmq.hpp>
#include "Log.h"
#include "TimerManager.h"
#include "NodeDefines.h"
#include "AppConfig.h"
#include "AgentServer.h"
#include "ModuleManager.h"
#include "LocalIDFactory.h"
#include "ControlCentreStubImp.h"
#include "ControlCentreStubImpEx.h"
#include "ProtoRpczServiceImpl.h"
#include "CommandManager.h"
#include "TimestampManager.h"
#include "ServerRegisterHelper.h"
#include "ControlCentreServiceImpHelper.h"
#include "ChannelManager.h"
#include "WorkerOperateHelper.h"

using namespace mdl;
using namespace evt;
using namespace ntwk;
using namespace thd;
using namespace util;

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

CAgentServer::CAgentServer()
	: m_isStarted(false)
	, m_bConnectServant(false)
	, m_pServerRegister(CreateServerRegister())
{
}

CAgentServer::~CAgentServer() {
    Dispose();
}

bool CAgentServer::Init(int argc, char** argv) {

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
    std::string strAccept(pConfig->GetString(APPCONFIG_SERVERACCEPT));
    assert(!strAccept.empty());
	std::string strServant(pConfig->GetString(APPCONFIG_SERVANTCONNECT));
	uint16_t u16ServerRegoin = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERREGION);

	CChannelManager::Pointer()->AddAgentChannel(uServerId);

	if(!m_pServerRegister.IsInvalid()) {
		// init status callback
		m_pServerRegister->InitStatusCallback();
		// register module
		m_pServerRegister->RegisterModule();
	}

	rpcz::application::set_connection_manager_threads(RpczThreads);
	rpcz::application::set_zmq_io_threads(ZmqioThreads);

	m_pServer.SetRawPointer(new rpcz::server);
	// register ControlCentreService
	m_pControlService.SetRawPointer(new CControlCentreServiceImp(
		u16ServerRegoin, CreateWorkerModle));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pControlService),
		m_pControlService->GetDescriptor()->name());
	// register WorkerService
	m_pWorkerService.SetRawPointer(new CAgentServiceImp(endPoint, strServant, strServerName, uServerId));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pWorkerService),
		m_pWorkerService->GetDescriptor()->name());

	m_pServer->bind(strBind);
	// start thread pool.
	ThreadPool.Startup(GameThreads);
	// connect server
	ConnectServers(strServerName, endPoint, uServerId, u16ServerRegoin);
	// connect control servant
	ConnectControlServant(strServant, strServerName, endPoint, uServerId, u16ServerRegoin, strAccept);
	// start thread
	ThreadPool.ExecuteTask(this);
	// tcp server
	unsigned short nMaxLink(pConfig->GetInt(APPCONFIG_ACCEPTMAXLINK, 1000));
	if(nMaxLink < 1) {
		nMaxLink = 1;
	}
    uint32_t u32PacketLimit = pConfig->GetInt(APPCONFIG_ACCEPTPACKETLIMIT, MAX_PACKET_SIZE);
	m_agentLogic.Init(strAccept, nMaxLink, strServant, u32PacketLimit, endPoint, u16ServerRegoin);
	printf("The server accept  address = %s maxlink = %hu packetlimit = %u \n", strAccept.c_str(),
        nMaxLink, u32PacketLimit);
	printf("The server started id = %u bind = %s endpoint = %s name = %s \n", 
		uServerId, strBind.c_str(), endPoint.c_str(), strServerName.c_str());
    return rc;
}

void CAgentServer::Dispose() {

	if(!m_isStarted) {
		return;
	}
	atomic_xchg(&g_serverStatus, SERVER_STATUS_IDLE);
	// remove keep timer.
	DisposeKeepRegTimer();
	printf("DisposeKeepRegTimer done ! \n");
	// unregister module
	if(!m_pServerRegister.IsInvalid()) {
		m_pServerRegister->UnregisterModule();
	}
	printf("UnregistModule done ! \n");
	// unregister control servant
	DisconnectControlServant();
	printf("DisconnectControlServant done ! \n");
	// unregister this server
	DisconnectServers();
	printf("DisconnectServers done ! \n");
	// set break the loop
	atomic_xchg8(&m_isStarted, false);
	// close game thread
	ThreadPool.Shutdown();
	printf("ThreadPool.Shutdown done ! \n");
    // close timer event
	CTimerManager::Release();
	printf("CTimerManager::Release done ! \n");
	// close tcp socket
	m_agentLogic.Dispose();
	printf("m_agentLogic.Dispose done ! \n");
	// clear channel manager
	CChannelManager::Pointer()->Dispose();
	CChannelManager::Release();
	printf("CChannelManager::Pointer()->Dispose done ! \n");
	// close command
	CommandManager::Release();
	printf("CommandManager::Release done ! \n");
	// close timestamp
	CTimestampManager::Release();
	printf("CTimestampManager::Release done ! \n");
	// close facade
	CFacade::Release();
	printf("CFacade::Release done ! \n");
	// release service
	m_pServer.SetRawPointer(NULL);
	m_pWorkerService.SetRawPointer(NULL);
	m_pControlService.SetRawPointer(NULL);
	// log dispose
	LogRelease();
	printf("LogRelease done ! \n");
	AppConfig::Release();
	printf("AppConfig::Release done! \n");
}

bool CAgentServer::OnRun() {

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());

    while (m_isStarted) {
		// timer
		pTMgr->Loop();

        Sleep(6);
    }
	return false;
}

void CAgentServer::DisposeKeepRegTimer()
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

void CAgentServer::ConnectServers(
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

		CAutoPointer<CallbackMFnP3<CAgentServer, std::string, volatile bool, volatile long> >
			callback(new CallbackMFnP3<CAgentServer, std::string, volatile bool, volatile long>
			(s_instance, &CAgentServer::KeepServersRegister, strConnect, false, 0));
		pTMgr->SetInterval(timerKey, KEEP_REGISTER_INTERVAL, callback);
	}
}

void CAgentServer::DisconnectServers()
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

void CAgentServer::KeepServersRegister(
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

void CAgentServer::ConnectControlServant(
	const std::string& strServant,
	const std::string& strServerName,
	const std::string& endPoint,
	uint32_t uServerId,
	uint16_t u16ServerRegion,
	const std::string& acceptAddress)
{
	if(!strServant.empty()) {

		printf("Register to Control Servant \"%s\" please wait... \n", strServant.c_str());
		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strProjectName(pConfig->GetString(APPCONFIG_PROJECTNAME));

		CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
		int nResult = pCtrlCenStubImpEx->RegisterServer(strServant, strServerName,
            endPoint, uServerId, u16ServerRegion, strProjectName, acceptAddress,
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

		CAutoPointer<CallbackMFnP2<CAgentServer, std::string, volatile bool> >
			callback(new CallbackMFnP2<CAgentServer, std::string, volatile bool>
			(s_instance, &CAgentServer::KeepControlServantRegister, strServant, false));

		uint64_t timerKey = CLocalIDFactory::Pointer()->GenerateID();
		m_keepRegTimerKeys.push_back(timerKey);

		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		pTMgr->SetInterval(timerKey, KEEP_REGISTER_INTERVAL, callback);
	}
}

void CAgentServer::DisconnectControlServant()
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

void CAgentServer::KeepControlServantRegister(std::string& connect, volatile bool& bRun)
{
	if(atomic_cmpxchg8(&bRun, false, true) == (char)false) {
		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
		uint32_t uServerId = pConfig->GetInt(APPCONFIG_SERVERID);

		CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
		int nResult = pCtrlCenStubImpEx->KeepRegister(connect, strServerName, uServerId,
			g_serverStatus, m_agentLogic.GetClientCount());
		if(CSR_NOT_FOUND == nResult) {
			std::string endPoint(pConfig->GetString(APPCONFIG_ENDPOINT));
			if(endPoint.empty()) {
				endPoint = pConfig->GetString(APPCONFIG_SERVERBIND);
			}
			std::string strProjectName(pConfig->GetString(APPCONFIG_PROJECTNAME));
			std::string strAccept(pConfig->GetString(APPCONFIG_SERVERACCEPT));
			uint16_t u16ServerRegion = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERREGION);

			nResult = pCtrlCenStubImpEx->RegisterServer(connect, strServerName, endPoint, uServerId,
				u16ServerRegion, strProjectName, strAccept, m_strProcessPath);
		}
		atomic_xchg8(&bRun, false);
	}
}


