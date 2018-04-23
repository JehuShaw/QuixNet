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
#include "GuidFactory.h"
#include "ProtoRpczServiceImpl.h"
#include "CommandManager.h"
#include "NodeDefines.h"
#include "HttpClientManager.h"
#include "NodeModule.h"
#include "ServerRegisterHelper.h"
#include "dbstl_common.h"

using namespace mdl;
using namespace evt;
using namespace util;
using namespace thd;
using namespace ntwk;

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
	// must have server.
	assert(!strBind.empty());
	// set the config

	CGuidFactory::Pointer()->SetCodeInt16(u16ServerId);

	if(!m_pServerRegister.IsInvalid()) {
		// init status callback
		m_pServerRegister->InitStatusCallback();
		// init register command
		m_pServerRegister->RegisterCommand();
		// register module
		m_pServerRegister->RegistModule();
	}
	// startup Berkeley DB
	dbstl::dbstl_startup();

	rpcz::application::set_connection_manager_threads(RpczThreads);
	rpcz::application::set_zmq_io_threads(ZmqioThreads);

	m_pServer.SetRawPointer(new rpcz::server);
	// register ControlCentreService
	m_pMasterService.SetRawPointer(new CMasterServiceImp(u16ServerId, strCurPath));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pMasterService),
		m_pMasterService->GetDescriptor()->name());
	// register WorkerService
	m_pWorkerService.SetRawPointer(new CWorkerServiceImp(strBind, strBind, strServerName, u16ServerId));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pWorkerService),
		m_pWorkerService->GetDescriptor()->name());

	m_pServer->bind(strBind);
	// start thread pool.
	ThreadPool.Startup(GameThreads);
	// connect server
	ConnectServers(strServerName, strBind, u16ServerId);
	// start thread
	ThreadPool.ExecuteTask(this);
	// set auto play timer
	SetAutoPlayTimer();

	printf("The server started id = %d address = %s name = %s \n", u16ServerId, strBind.c_str(), strServerName.c_str());
    return rc;
}

void CMasterServer::Dispose() {

	if(!m_isStarted) {
		return;
	}
	DisposeKeepRegTimer();
	// unregister module
	if(!m_pServerRegister.IsInvalid()) {
		m_pServerRegister->UnregistModule();
	}
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
	// release service
	m_pServer.SetRawPointer(NULL);
	m_pWorkerService.SetRawPointer(NULL);
	m_pMasterService.SetRawPointer(NULL);
	// log dispose
	LogRelease();
	// exit Berkeley DB
	dbstl::dbstl_exit();
}

bool CMasterServer::Run() {
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
    CHttpClientManager::PTR_T pHttpClientMgr(CHttpClientManager::Pointer());

    while (m_isStarted) {
		// timer
		pTMgr->Loop();
        // http
        pHttpClientMgr->Run();

        Sleep(6);
    }
	if(atomic_cmpxchg8(&m_bRegistCentre, false, true) == (char)true) {
		// unregister control centre, don't invole by Dispose()
		UnregistControlCentre();
	}
	return false;
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

void CMasterServer::ConnectServers(const std::string& strServerName,
	const std::string& strBind, unsigned short u16ServerId)
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
			strBind, u16ServerId, ID_NULL, CALL_REGISTER_MS);
		// This server can't connect Servant.
		if(CSR_SUCCESS != nResult) {
			OutputError("[connect = %s, name = %s, endPint = %s,"
				" errorCode = %d] Can't Register this server!", strConnect.c_str(),
				strServerName.c_str(), strBind.c_str(), nResult);
			printf("Register to \"%s\" fail. \n", strConnect.c_str());
			continue;
		}
		printf("Register to \"%s\" success. \n", strConnect.c_str());

		CAutoPointer<CallbackMFnP2<CMasterServer, std::string, volatile bool> >
			callback(new CallbackMFnP2<CMasterServer, std::string, volatile bool>
			(s_instance, &CMasterServer::KeepServersRegister, strConnect, false));

		uint64_t timerKey = CGuidFactory::Pointer()->CreateGuid();
		m_keepRegTimerKeys.push_back(timerKey);

		pTMgr->SetInterval(timerKey, KEEP_REGISTER_INTERVAL, callback);
	}
}

void CMasterServer::DisconnectServers()
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

		if(CSR_SUCCESS != nResult && CSR_WITHOUT_THIS_MODULE != nResult) {
			OutputError("[connect = %s, name = %s, errorCode = %d]"
                " Can't Unregister this server!", strConnect.c_str(),
				strServerName.c_str(), nResult);
			continue;
		}
	}
}

void CMasterServer::KeepServersRegister(std::string& connect, volatile bool& bRun)
{
	if(atomic_cmpxchg8(&bRun, true, false) == false) {
		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		uint16_t u16ServerId = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERID);
		CControlCentreStubImp::PTR_T pCtrlCenStubImp(CControlCentreStubImp::Pointer());
		if(pCtrlCenStubImp->KeepRegister(connect, u16ServerId)) {
			std::string strBind(pConfig->GetString(APPCONFIG_SERVERBIND));
			std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));

			pCtrlCenStubImp->RegisterServer(connect, strServerName, strBind,
				u16ServerId, ID_NULL);
		}
		atomic_xchg8(&bRun, false);
	}
}

void CMasterServer::RegistControlCentre()
{
	CAutoPointer<CallbackMFnP1<CMasterServer, volatile bool> >
		callback(new CallbackMFnP1<CMasterServer, volatile bool>
		(s_instance, &CMasterServer::KeepControlCentreRegister, false));

	m_keepCentreTimerKey = CGuidFactory::Pointer()->CreateGuid();

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->SetInterval(m_keepCentreTimerKey, KEEP_REGISTER_INTERVAL, callback);
}

void CMasterServer::UnregistControlCentre() {
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->Remove(m_keepCentreTimerKey, true);
}

void CMasterServer::KeepControlCentreRegister(volatile bool& bRun)
{
	if(atomic_cmpxchg8(&bRun, true, false) == false) {

		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
		std::string strBind(pConfig->GetString(APPCONFIG_SERVERBIND));
		uint16_t u16ServerId = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERID);

		CMasterStubImp::PTR_T pMasterStubImp(CMasterStubImp::Pointer());
		pMasterStubImp->KeepRegister(strServerName, strBind,
			u16ServerId, g_serverStatus, 0);
		atomic_xchg8(&bRun, false);
	}
}

void CMasterServer::SetAutoPlayTimer()
{
	if(0 != s_autoPlayTimerKey) {
		OutputError("The auto play timer key is already exist."
			" s_autoPlayTimerKey = "I64FMTD, (uint64_t)s_autoPlayTimerKey);
		return;
	}
	atomic_xchg64(&s_autoPlayTimerKey, CGuidFactory::Pointer()->CreateGuid());

	CAutoPointer<CallBackFuncP0>
		callback(new CallBackFuncP0
		(&CMasterServer::AutoPlayCallback));

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->SetTimeout(s_autoPlayTimerKey, AUTO_PLAY_EXPIRY_TIME, callback);
}

void CMasterServer::RemoveAutoPlayTimer()
{
	if(0 == s_autoPlayTimerKey) {
		return;
	}

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	if(pTMgr->Remove(s_autoPlayTimerKey, true)) {
		atomic_xchg64(&s_autoPlayTimerKey, 0);
	} else {
		OutputError("!timerManager.Remove() s_autoPlayTimerKey = "
			I64FMTD, (uint64_t)s_autoPlayTimerKey);
	}
}

void CMasterServer::AutoPlayCallback()
{
	atomic_xchg64(&s_autoPlayTimerKey, 0);

	CMasterServer::PTR_T pMaster(CMasterServer::Pointer());
	pMaster->OnServerPlay();
}

