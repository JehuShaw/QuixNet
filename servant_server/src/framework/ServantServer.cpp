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
#include "LocalIDFactory.h"
#include "ProtoRpczServiceImpl.h"
#include "CommandManager.h"
#include "TimestampManager.h"
#include "ServerRegisterHelper.h"
#include "WorkerOperateHelper.h"

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
	std::string strBind(pConfig->GetString(APPCONFIG_SERVERBIND));
	// must have server.
	assert(!strBind.empty());
	std::string endPoint(pConfig->GetString(APPCONFIG_ENDPOINT));
	if(endPoint.empty()) {
		endPoint = strBind;
	}


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
	std::string strMaster(pConfig->GetString(APPCONFIG_MASTERCONNECT));
	assert(!strMaster.empty());

	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	m_pControlService.SetRawPointer(new CServantServiceImp(
		pChlMgr->GetRpczChannel(strMaster), endPoint, uServerId));

	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pControlService),
		m_pControlService->GetDescriptor()->name());
	// register WorkerService
	m_pWorkerService.SetRawPointer(new CWorkerServiceImp(endPoint, strMaster, strServerName, uServerId));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pWorkerService),
		m_pWorkerService->GetDescriptor()->name());
	m_pServer->bind(strBind);
	// start thread pool.
	ThreadPool.Startup(GameThreads);
	// connect server
	ConnectControlMaster(strMaster, strServerName, endPoint, uServerId);
	// start thread
	ThreadPool.ExecuteTask(this);

    printf("The server started id = %u bind = %s endpoint = %s name = %s \n", 
		uServerId, strBind.c_str(), endPoint.c_str(), strServerName.c_str());
    return rc;
}

void CServantServer::Dispose() {

	if(!m_isStarted) {
		return;
	}
	atomic_xchg(&g_serverStatus, SERVER_STATUS_IDLE);
	// remove keep timer.
	DisposeKeepRegTimer();
	CServantServiceImp::ClearAllTimer();
	printf("DisposeKeepRegTimer done! \n");
	// unregister the module
	if(!m_pServerRegister.IsInvalid()) {
		m_pServerRegister->UnregisterModule();
	}
	printf("UnregistModule done! \n");
	// unregister this server
	DisconnectControlMaster();
	printf("DisconnectControlMaster done! \n");
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

bool CServantServer::OnRun() {
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
	const std::string& endPoint,
	uint32_t uServerId)
{
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());

	if(!strMaster.empty()) {
		std::string strProjectName(pConfig->GetString(APPCONFIG_PROJECTNAME));
		printf("Register to Control Master \"%s\" please wait... \n", strMaster.c_str());
		int nResult = pServantStubImp->RegisterServer(strMaster, strServerName, endPoint,
			uServerId, ID_NULL, strProjectName, std::string(), m_strProcessPath,
			CALL_REGISTER_MS);

		if(nResult < CSR_TIMEOUT && CSR_CHANNEL_ALREADY_EXIST != nResult) {
			OutputError("[connect = %s, name = %s, endPint = %s,"
				" errorCode = %d] Can't Register this server!", strMaster.c_str(),
				strServerName.c_str(), endPoint.c_str(), nResult);
			printf("Register to Control Master \"%s\" fail. \n", strMaster.c_str());
			return;
		} else if (nResult == CSR_TIMEOUT) {
			printf("Register to Control Master \"%s\" timeout. %d \n", strMaster.c_str(), nResult);
		} else {
			printf("Register to Control Master \"%s\" success. %d \n", strMaster.c_str(), nResult);
		}

		if(CSR_SUCCESS_AND_START == nResult) {
			if (atomic_xchg(&g_serverStatus, SERVER_STATUS_START) != SERVER_STATUS_START) {
				SendWorkerNotification(N_CMD_SERVER_PREPARE, ID_NULL, NULL, NULL, ROUTE_BALANCE_SERVERID);
				SendWorkerNotification(N_CMD_SERVER_PLAY, ID_NULL, NULL, NULL, ROUTE_BALANCE_SERVERID);
			}
		}

		CAutoPointer<CallbackMFnP2<CServantServer, std::string, volatile bool> >
			callback(new CallbackMFnP2<CServantServer, std::string, volatile bool>
			(s_instance, &CServantServer::KeepControlMasterRegister, strMaster, false));

		uint64_t timerKey = CLocalIDFactory::Pointer()->GenerateID();
		m_keepRegTimerKeys.push_back(timerKey);

		pTMgr->SetInterval(timerKey, KEEP_REGISTER_INTERVAL, callback);
	}
}

void CServantServer::DisconnectControlMaster()
{
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strMaster(pConfig->GetString(APPCONFIG_MASTERCONNECT));
	if(!strMaster.empty()) {

		std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
		uint32_t uServerId = pConfig->GetInt(APPCONFIG_SERVERID);

		CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
		int nResult = pServantStubImp->UnregisterServer(strMaster, strServerName,
			uServerId, CALL_UNREGISTER_MS);

		if(nResult < CSR_TIMEOUT && CSR_WITHOUT_THIS_MODULE != nResult) {
			OutputError("[connect = %s, name = %s, errorCode = %d]"
                " Can't Unregister this server!", strMaster.c_str(),
				strServerName.c_str(), nResult);
			return;
		}
	}
}

void CServantServer::KeepControlMasterRegister(std::string& connect, volatile bool& bRun)
{
	if(atomic_cmpxchg8(&bRun, false, true) == (char)false) {
		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
		uint32_t uServerId = pConfig->GetInt(APPCONFIG_SERVERID);
		std::string endPoint(pConfig->GetString(APPCONFIG_ENDPOINT));
		if(endPoint.empty()) {
			endPoint = pConfig->GetString(APPCONFIG_SERVERBIND);
		}
		CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
		int nResult = pServantStubImp->KeepRegister(connect, strServerName,
			endPoint, uServerId, g_serverStatus);
		if(CSR_NOT_FOUND == nResult) {
			std::string strProjectName(pConfig->GetString(APPCONFIG_PROJECTNAME));

			nResult = pServantStubImp->RegisterServer(connect, strServerName, endPoint,
				uServerId, ID_NULL, strProjectName, std::string(), m_strProcessPath);
		}
		atomic_xchg8(&bRun, false);
	}
}

