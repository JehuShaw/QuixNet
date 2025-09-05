/*
 * File:   MasterServer.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */
#include <zmq.hpp>
#include "Log.h"
#include "TimerManager.h"
#include "NodeDefines.h"
#include "AppConfig.h"
#include "MasterServer.h"
#include "ModuleManager.h"
#include "ControlCentreStubImp.h"
#include "MasterStubImp.h"
#include "LocalIDFactory.h"
#include "ProtoRpczServiceImpl.h"
#include "CommandManager.h"
#include "NodeDefines.h"
#include "NodeModule.h"
#include "ServerRegisterHelper.h"
#include "MasterCmdManager.h"


using namespace mdl;
using namespace evt;
using namespace util;
using namespace thd;

void LoadAppConfig() {
	AppConfig::REVISE_SET_T reviseSet;
	reviseSet[APPCONFIG_SERVERBIND] = ReviseAddress;
	reviseSet[APPCONFIG_SERVERCONNECT] = ReviseAddress;

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	if (!pConfig->LoadFile((char*)APP_CONFIG_NAME, reviseSet)) {
		fprintf(stderr, "Cannot load the %s file\n", (char*)APP_CONFIG_NAME);
		assert(false);
	}
}

volatile uint64_t CMasterServer::s_prepareTimerKey = 0;
volatile uint64_t CMasterServer::s_autoPlayTimerKey = 0;

CMasterServer::CMasterServer()
	: m_isStarted(false)
	, m_bRegistCentre(false)
	, m_keepCentreTimerKey(0)
	, m_pServerRegister(CreateServerRegister())
{
}

CMasterServer::~CMasterServer() {
    Dispose();
}

bool CMasterServer::Init(int argc, char** argv) {

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
		strLogPath += "/log/";
	} else {
		strLogPath = "./log/";
	}
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
	std::string strAgentName(pConfig->GetString(APPCONFIG_AGENTSERVERNAME));
	std::string strBind(pConfig->GetString(APPCONFIG_SERVERBIND));
	// must have server.
	assert(!strBind.empty());
	std::string endPoint(pConfig->GetString(APPCONFIG_ENDPOINT));
	if(endPoint.empty()) {
		endPoint = strBind;
	}
	std::string strAccept(pConfig->GetString(APPCONFIG_SERVERACCEPT));
	assert(!strAccept.empty());

	if(!m_pServerRegister.IsInvalid()) {
		// init status callback
		m_pServerRegister->InitStatusCallback();
		// init register command
		m_pServerRegister->RegisterCommand();
		// register module
		m_pServerRegister->RegisterModule();
	}

	rpcz::application::set_connection_manager_threads(RpczThreads);
	rpcz::application::set_zmq_io_threads(ZmqioThreads);

	m_pServer.SetRawPointer(new rpcz::server);
	// register ControlCentreService
	m_pMasterService.SetRawPointer(new CMasterServiceImp(strLogPath));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pMasterService),
		m_pMasterService->GetDescriptor()->name());
	// register WorkerService
	m_pWorkerService.SetRawPointer(new CWorkerServiceImp(endPoint, endPoint, strServerName, uServerId));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pWorkerService),
		m_pWorkerService->GetDescriptor()->name());

	// set agent server name
	CMasterCmdManager::Pointer()->Init(strAgentName);

	m_pServer->bind(strBind);
	// start thread pool.
	ThreadPool.Startup(GameThreads);
	// connect server
	ConnectServers(strServerName, strBind, uServerId);
	// start http thread
	ThreadPool.ExecuteTask(&m_httpThreadHold);
	// start thread
	ThreadPool.ExecuteTask(this);
	// set auto play timer
	SetAutoPlayTimer();
	// tcp server
	unsigned short nMaxLink(pConfig->GetInt(APPCONFIG_ACCEPTMAXLINK, 1000));
	if (nMaxLink < 1) {
		nMaxLink = 1;
	}
	uint32_t u32PacketLimit = pConfig->GetInt(APPCONFIG_ACCEPTPACKETLIMIT, MAX_PACKET_SIZE);
	m_masterLogic.Init(strAccept, nMaxLink, u32PacketLimit);
	printf("The server accept  address = %s maxlink = %hu packetlimit = %u \n", strAccept.c_str(),
		nMaxLink, u32PacketLimit);
	printf("The server started id = %u bind = %s endpoint = %s name = %s \n",
		uServerId, strBind.c_str(), endPoint.c_str(), strServerName.c_str());
    return rc;
}

void CMasterServer::Dispose() {

	if(!m_isStarted) {
		return;
	}
	DisposeKeepRegTimer();
	CMasterServiceImp::ClearAllTimer();
	printf("DisposeKeepRegTimer done! \n");
	// unregister module
	if(!m_pServerRegister.IsInvalid()) {
		m_pServerRegister->UnregisterModule();
	}
	printf("UnregistModule done! \n");
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
	// close tcp socket
	m_masterLogic.Dispose();
	printf("m_masterLogic.Dispose done ! \n");
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
	// release service
	m_pServer.SetRawPointer(NULL);
	m_pWorkerService.SetRawPointer(NULL);
	m_pMasterService.SetRawPointer(NULL);
	// log dispose
	LogRelease();
	printf("LogRelease done! \n");
	AppConfig::Release();
	printf("AppConfig::Release done! \n");
}

bool CMasterServer::OnRun() {
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());

    while (m_isStarted) {
		// timer
		pTMgr->Loop();

        Sleep(6);
    }
	if(atomic_cmpxchg8(&m_bRegistCentre, true, false) == (char)true) {
		// unregister control center, don't invole by Dispose()
		UnregistControlCentre();
	}
	return false;
}

void CMasterServer::OnServerPrepare()
{
	CNodeModule::BroadcastAllNodes(N_CMD_SERVER_PREPARE);

	/////////////////////////////////////////////////////
	// Set play timer
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());

	if (0 != s_autoPlayTimerKey) {
		if (pTMgr->Remove(s_autoPlayTimerKey, true)) {
			atomic_xchg64(&s_autoPlayTimerKey, 0);
		} else {
			OutputError("!timerManager.Remove() s_autoPlayTimerKey = "
				I64FMTD, (uint64_t)s_autoPlayTimerKey);
		}
	}
	atomic_xchg64(&s_autoPlayTimerKey, CLocalIDFactory::Pointer()->GenerateID());

	int nPlayDelay = PREPARE_PLAY_INTERVAL;
	if (nPlayDelay < 0) {
		nPlayDelay = 0;
	}

	CAutoPointer<CallBackFuncP0>
		callback(new CallBackFuncP0
		(&CMasterServer::PlayCallback));

	pTMgr->SetTimeout(s_autoPlayTimerKey, nPlayDelay, callback);
}

void CMasterServer::OnServerPlay()
{
	m_pMasterService->OnServerPlay();

	atomic_xchg(&g_serverStatus, SERVER_STATUS_START);

	CNodeModule::BroadcastAllNodes(N_CMD_SERVER_PLAY);
}

void CMasterServer::DisposeKeepRegTimer()
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

void CMasterServer::ConnectServers(
	const std::string& strServerName,
	const std::string& endPoint,
	uint32_t uServerId)
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
			endPoint, uServerId, ID_NULL, CALL_REGISTER_MS);
		// This server can't connect Servant.
		if(nResult < CSR_TIMEOUT) {
			OutputError("[connect = %s, name = %s, endPint = %s,"
				" errorCode = %d] Can't Register this server!", strConnect.c_str(),
				strServerName.c_str(), endPoint.c_str(), nResult);
			printf("Register to \"%s\" fail. \n", strConnect.c_str());
			continue;
		} else if (nResult == CSR_TIMEOUT) {
			printf("Register to \"%s\" timeout. \n", strConnect.c_str());
		} else {
			printf("Register to \"%s\" success. \n", strConnect.c_str());
		}

		uint64_t timerKey = CLocalIDFactory::Pointer()->GenerateID();
		m_keepRegTimerKeys.push_back(timerKey);

		CAutoPointer<CallbackMFnP3<CMasterServer, std::string, volatile bool, volatile long> >
			callback(new CallbackMFnP3<CMasterServer, std::string, volatile bool, volatile long>
			(s_instance, &CMasterServer::KeepServersRegister, strConnect, false, 0));
		pTMgr->SetInterval(timerKey, KEEP_REGISTER_INTERVAL, callback);
	}
}

void CMasterServer::DisconnectServers()
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

void CMasterServer::KeepServersRegister(
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

			nResult = pCtrlCenStubImp->RegisterServer(connect, strServerName, endPoint,
				uServerId, ID_NULL);
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

void CMasterServer::RegistControlCentre()
{
	CAutoPointer<CallbackMFnP1<CMasterServer, volatile bool> >
		callback(new CallbackMFnP1<CMasterServer, volatile bool>
		(s_instance, &CMasterServer::KeepControlCentreRegister, false));

	m_keepCentreTimerKey = CLocalIDFactory::Pointer()->GenerateID();

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->SetInterval(m_keepCentreTimerKey, KEEP_REGISTER_INTERVAL, callback);
}

void CMasterServer::UnregistControlCentre() {
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->Remove(m_keepCentreTimerKey, true);
}

void CMasterServer::KeepControlCentreRegister(volatile bool& bRun)
{
	if(atomic_cmpxchg8(&bRun, false, true) == false) {

		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
		uint32_t uServerId = pConfig->GetInt(APPCONFIG_SERVERID);
		std::string endPoint(pConfig->GetString(APPCONFIG_ENDPOINT));
		if (endPoint.empty()) {
			endPoint = pConfig->GetString(APPCONFIG_SERVERBIND);
		}

		CMasterStubImp::PTR_T pMasterStubImp(CMasterStubImp::Pointer());
		pMasterStubImp->KeepRegister(strServerName, endPoint,
			uServerId, g_serverStatus, 0);

		atomic_xchg8(&bRun, false);
	}
}

void CMasterServer::SetAutoPlayTimer()
{
	if (0 != s_prepareTimerKey) {
		OutputError("The timer key is already exist."
			" s_prepareTimerKey = " I64FMTD, (uint64_t)s_prepareTimerKey);
		return;
	}
	atomic_xchg64(&s_prepareTimerKey, CLocalIDFactory::Pointer()->GenerateID());

	int nPrepareDelay = AUTO_PLAY_EXPIRY_TIME - PREPARE_PLAY_INTERVAL;
	if (nPrepareDelay < 0) {
		nPrepareDelay = 0;
	}

	CAutoPointer<CallBackFuncP0>
		callback(new CallBackFuncP0
		(&CMasterServer::PrepareCallback));

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->SetTimeout(s_prepareTimerKey, nPrepareDelay, callback);
}

void CMasterServer::RemoveAutoPlayTimer()
{
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());

	if (0 != s_prepareTimerKey) {
		if (pTMgr->Remove(s_prepareTimerKey, true)) {
			atomic_xchg64(&s_prepareTimerKey, 0);
		} else {
			OutputError("!timerManager.Remove() s_prepareTimerKey = "
				I64FMTD, (uint64_t)s_prepareTimerKey);
		}
	}

	if(0 != s_autoPlayTimerKey) {
		if (pTMgr->Remove(s_autoPlayTimerKey, true)) {
			atomic_xchg64(&s_autoPlayTimerKey, 0);
		} else {
			OutputError("!timerManager.Remove() s_autoPlayTimerKey = "
				I64FMTD, (uint64_t)s_autoPlayTimerKey);
		}
	}
}

void CMasterServer::PrepareCallback()
{
	atomic_xchg64(&s_prepareTimerKey, 0);

	CMasterServer::PTR_T pMaster(CMasterServer::Pointer());
	pMaster->OnServerPrepare();
}

void CMasterServer::PlayCallback()
{
	atomic_xchg64(&s_autoPlayTimerKey, 0);

	CMasterServer::PTR_T pMaster(CMasterServer::Pointer());
	pMaster->OnServerPlay();
}

