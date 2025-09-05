/*
 * File:   CacheServer.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */
#include <zmq.hpp>
#include "Log.h"
#include "TimerManager.h"
#include "NodeDefines.h"
#include "AppConfig.h"
#include "CacheServer.h"
#include "ModuleManager.h"
#include "AtomicLock.h"
#include "ControlCentreStubImp.h"
#include "ControlCentreStubImpEx.h"
#include "ProtoRpczServiceImpl.h"
#include "LocalIDFactory.h"
#include "CacheDBManager.h"
#include "CacheMemoryManager.h"
#include "CommandManager.h"
#include "ServerRegisterHelper.h"
#include "CacheServiceImpHelper.h"
#include "WorkerOperateHelper.h"

using namespace mdl;
using namespace util;
using namespace evt;
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

CCacheServer::CCacheServer()
	: m_isStarted(false)
	, m_pServerRegister(CreateServerRegister())
{
}

CCacheServer::~CCacheServer() {
    Dispose();
}

bool CCacheServer::Init(int argc, char** argv) {

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
	int32_t nRecordExpireMin = pConfig->GetInt(APPCONFIG_CACHERECORDEXPIRE, 5);
	if(nRecordExpireMin < 1) {
		nRecordExpireMin = 1;
	}
	g_recordExpireSec = nRecordExpireMin * 60;
	std::string strServant(pConfig->GetString(APPCONFIG_SERVANTCONNECT));
	std::string strAgentName(pConfig->GetString(APPCONFIG_AGENTSERVERNAME));
	uint16_t u16ServerRegoin = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERREGION);

    // load db config
    CCacheDBManager::Pointer()->Init();

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
	// register CacheService
	m_pCacheService.SetRawPointer(new CCacheServiceImp(endPoint,
		strServant, strServerName, strAgentName, uServerId, CreateCachePlayer));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pCacheService),
		m_pCacheService->GetDescriptor()->name());
	// register ControlCentreService
	m_pControlService.SetRawPointer(new CControlCentreServiceImp(u16ServerRegoin));
	m_pServer->register_rpc_service(new node::CProtoRpcServiceImpl(*m_pControlService),
		m_pControlService->GetDescriptor()->name());

	m_pServer->bind(strBind);
	// start thread pool.
	ThreadPool.Startup(GameThreads);
	// connect server
	ConnectServers(strServerName, endPoint, uServerId, u16ServerRegoin);
	// connect control servant
	ConnectControlServant(strServant, strServerName, endPoint, uServerId, u16ServerRegoin);
	// start thread
	ThreadPool.ExecuteTask(this);
    // auto update cache record
    CCacheMemoryManager::Pointer()->Init();
	printf("The server started id = %u bind = %s endpoint = %s name = %s \n", 
		uServerId, strBind.c_str(), endPoint.c_str(), strServerName.c_str());
    return rc;
}

void CCacheServer::Dispose() {

	if(!m_isStarted) {
		return;
	}
	atomic_xchg(&g_serverStatus, SERVER_STATUS_IDLE);
    // remove auto update cache record
    CCacheMemoryManager::Pointer()->Dispose();
	CCacheMemoryManager::Release();
	printf("CCacheMemoryManager::Pointer()->Dispose done! \n");
    // unregister keep reg timer
	DisposeKeepRegTimer();
	printf("DisposeKeepRegTimer done! \n");
	// unregister Module
	if(!m_pServerRegister.IsInvalid()) {
		m_pServerRegister->UnregisterModule();
	}
	printf("UnregistModule done! \n");
	// unregister control servant
	DisconnectControlServant();
	printf("DisconnectControlServant done! \n");
	// unregister this server
	DisconnectServers();
	printf("DisconnectServers done! \n");
	// set break the loop
	atomic_xchg8(&m_isStarted, false);
    // dispose db config
    CCacheDBManager::Pointer()->Dispose();
	CCacheDBManager::Release();
	printf("CCacheDBManager::Pointer()->Dispose done! \n");
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
	// close Timestamp
	CTimestampManager::Release();
	printf("CTimestampManager::Release done! \n");
	// close facade
	CFacade::Release();
	printf("CFacade::Release done! \n");
	// release service
	m_pServer.SetRawPointer(NULL);
	m_pControlService.SetRawPointer(NULL);
	m_pCacheService.SetRawPointer(NULL);
	// close log thread
	LogRelease();
	printf("LogRelease done! \n");
	AppConfig::Release();
	printf("AppConfig::Release done! \n");
}

bool CCacheServer::OnRun() {
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	while (m_isStarted) {
		// timer
		pTMgr->Loop();
		Sleep(6);
	}
	return false;
}

void CCacheServer::DisposeKeepRegTimer()
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

void CCacheServer::ConnectServers(
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

		CAutoPointer<CallbackMFnP3<CCacheServer, std::string, volatile bool, volatile long> >
			callback(new CallbackMFnP3<CCacheServer, std::string, volatile bool, volatile long>
			(s_instance, &CCacheServer::KeepServersRegister, strConnect, false, 0));
		pTMgr->SetInterval(timerKey, KEEP_REGISTER_INTERVAL, callback);
	}
}

void CCacheServer::DisconnectServers()
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

void CCacheServer::KeepServersRegister(
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

void CCacheServer::ConnectControlServant(
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

		CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
		int nResult = pCtrlCenStubImpEx->RegisterServer(strServant, strServerName,
			endPoint, uServerId, u16ServerRegion, strProjectName, std::string(),
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

		CAutoPointer<CallbackMFnP2<CCacheServer, std::string, volatile bool> >
			callback(new CallbackMFnP2<CCacheServer, std::string, volatile bool>
			(s_instance, &CCacheServer::KeepControlServantRegister, strServant, false));

		uint64_t timerKey = CLocalIDFactory::Pointer()->GenerateID();
		m_keepRegTimerKeys.push_back(timerKey);

		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		pTMgr->SetInterval(timerKey, KEEP_REGISTER_INTERVAL, callback);
	}
}

void CCacheServer::DisconnectControlServant()
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

void CCacheServer::KeepControlServantRegister(std::string& connect, volatile bool& bRun)
{
	if(atomic_cmpxchg8(&bRun, false, true) == (char)false) {
		AppConfig::PTR_T pConfig(AppConfig::Pointer());
		std::string strServerName(pConfig->GetString(APPCONFIG_SERVERNAME));
		uint32_t uServerId = pConfig->GetInt(APPCONFIG_SERVERID);

        CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
		int nResult = pCtrlCenStubImpEx->KeepRegister(connect, strServerName, uServerId,
			g_serverStatus, pChlMgr->GetClientSize());
		if(CSR_NOT_FOUND == nResult) {
			std::string endPoint(pConfig->GetString(APPCONFIG_ENDPOINT));
			if(endPoint.empty()) {
				endPoint = pConfig->GetString(APPCONFIG_SERVERBIND);
			}
			std::string strProjectName(pConfig->GetString(APPCONFIG_PROJECTNAME));
			uint16_t u16ServerRegion = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERREGION);

			nResult = pCtrlCenStubImpEx->RegisterServer(connect, strServerName, endPoint, uServerId,
				u16ServerRegion, strProjectName, std::string(), m_strProcessPath);
		}
		atomic_xchg8(&bRun, false);
	}
}


