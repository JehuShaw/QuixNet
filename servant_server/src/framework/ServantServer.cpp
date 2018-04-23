/*
 * File:   ServantServer.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */
#include "Log.h"
#include "TimerManager.h"
#include "NodeDefines.h"
#include "AppConfig.h"
#include "ServantServer.h"
#include "ModuleManager.h"
#include "ServantStubImp.h"
#include "GuidFactory.h"
#include "ProtoRpczServiceImpl.h"
#include "CommandManager.h"
#include "TimestampManager.h"
#include "ServerRegisterHelper.h"

using namespace mdl;
using namespace evt;
using namespace util;
using namespace thd;

void LoadAppConfig() {
	AppConfig::REVISE_SET_T reviseSet;
	reviseSet[APPCONFIG_SERVERBIND] = ReviseAddress;
	reviseSet[APPCONFIG_MASTERCONNECT] = ReviseAddress;

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	if (!pConfig->LoadFile((char*)APP_CONFIG_NAME, reviseSet)) {
		fprintf(stderr, "Cannot load the %s file\n", (char*)APP_CONFIG_NAME);
		assert(false);
	}
}

CServantServer::CServantServer()
	: m_isStarted(false)
	, m_pServerRegister(CreateServerRegister())
{
}

CServantServer::~CServantServer() {
    Dispose();
}

bool CServantServer::Init(int argc, char** argv) {

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
		// register module
		m_pServerRegister->RegistModule();
	}

	rpcz::application::set_connection_manager_threads(RpczThreads);
	rpcz::application::set_zmq_io_threads(ZmqioThreads);

	m_pServer.SetRawPointer(new rpcz::server);
	// register ControlCentreService
	std::string strMaster(pConfig->GetString(APPCONFIG_MASTERCONNECT));
	assert(!strMaster.empty());

	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	m_pControlService.SetRawPointer(new CServantServiceImp(
		pChlMgr->GetRpczChannel(strMaster), strBind, u16ServerId));

	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pControlService),
		m_pControlService->GetDescriptor()->name());
	// register WorkerService
	m_pWorkerService.SetRawPointer(new CWorkerServiceImp(strBind, strMaster, strServerName, u16ServerId));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pWorkerService),
		m_pWorkerService->GetDescriptor()->name());
	m_pServer->bind(strBind);
	// start thread pool.
	ThreadPool.Startup(GameThreads);
	// connect server
	ConnectControlMaster(strMaster, strServerName, strBind, u16ServerId);
	// start thread
	ThreadPool.ExecuteTask(this);

	printf("The server started id = %d address = %s name = %s \n", u16ServerId, strBind.c_str(), strServerName.c_str());
    return rc;
}

void CServantServer::Dispose() {

	if(!m_isStarted) {
		return;
	}
	atomic_xchg(&g_serverStatus, SERVER_STATUS_IDLE);
	// remove keep timer.
	DisposeKeepRegTimer();
	// unregister the module
	if(!m_pServerRegister.IsInvalid()) {
		m_pServerRegister->UnregistModule();
	}
	// unregister this server
	DisconnectControlMaster();
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
	m_pControlService.SetRawPointer(NULL);
	// log dispose
	LogRelease();
}

bool CServantServer::Run() {
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
    while (m_isStarted) {
		// timer
		pTMgr->Loop();
        Sleep(6);
    }
	return false;
}

void CServantServer::DisposeKeepRegTimer()
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

void CServantServer::ConnectControlMaster(
	const std::string& strMaster,
	const std::string& strServerName,
	const std::string& strBind,
	uint16_t u16ServerId)
{
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());

	if(!strMaster.empty()) {
		std::string strProjectName(pConfig->GetString(APPCONFIG_PROJECTNAME));
		printf("Register to Control Master \"%s\" please wait... \n", strMaster.c_str());
		int nResult = pServantStubImp->RegisterServer(strMaster, strServerName, strBind,
			u16ServerId, ID_NULL, strProjectName, std::string(), m_strProcessPath,
			CALL_REGISTER_MS);

		if(nResult < CSR_SUCCESS && CSR_CHANNEL_ALREADY_EXIST != nResult) {
			OutputError("[connect = %s, name = %s, endPint = %s,"
				" errorCode = %d] Can't Register this server!", strMaster.c_str(),
				strServerName.c_str(), strBind.c_str(), nResult);
			printf("Register to Control Master \"%s\" fail. \n", strMaster.c_str());
			return;
		}
		printf("Register to Control Master \"%s\" success. \n", strMaster.c_str());
		if(CSR_SUCCESS_AND_START == nResult) {
			atomic_xchg(&g_serverStatus, SERVER_STATUS_START);
		}

		CAutoPointer<CallbackMFnP2<CServantServer, std::string, volatile bool> >
			callback(new CallbackMFnP2<CServantServer, std::string, volatile bool>
			(s_instance, &CServantServer::KeepControlMasterRegister, strMaster, false));

		uint64_t timerKey = CGuidFactory::Pointer()->CreateGuid();
		m_keepRegTimerKeys.push_back(timerKey);

		pTMgr->SetInterval(timerKey, KEEP_REGISTER_INTERVAL, callback);
	}
}

void CServantServer::DisconnectControlMaster()
{
	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strMaster(pConfig->GetString(APPCONFIG_MASTERCONNECT));
	if(!strMaster.empty()) {

		std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
		uint16_t u16ServerId = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERID);

		int nResult = pServantStubImp->UnregisterServer(strMaster, strServerName,
			u16ServerId, CALL_UNREGISTER_MS);
		if(nResult < CSR_SUCCESS && CSR_WITHOUT_THIS_MODULE != nResult) {
			OutputError("[connect = %s, name = %s, errorCode = %d]"
                " Can't Unregister this server!", strMaster.c_str(),
				strServerName.c_str(), nResult);
			return;
		}
	}
}

void CServantServer::KeepControlMasterRegister(std::string& connect, volatile bool& bRun)
{
	if(atomic_cmpxchg8(&bRun, true, false) == false) {
		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
		CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
		std::string strBind(pConfig->GetString(APPCONFIG_SERVERBIND));
		uint16_t u16ServerId = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERID);
		if(pServantStubImp->KeepRegister(connect, strServerName, strBind, u16ServerId, g_serverStatus)) {

			std::string strProjectName(pConfig->GetString(APPCONFIG_PROJECTNAME));
			pServantStubImp->RegisterServer(connect, strServerName, strBind,
				u16ServerId, ID_NULL, strProjectName, std::string(), m_strProcessPath);
		}
		atomic_xchg8(&bRun, false);
	}
}

