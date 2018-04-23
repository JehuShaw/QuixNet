/*
 * File:   GameServerRegister.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */
#include "NodeDefines.h"
#include "GameServerRegister.h"
#include "AppConfig.h"
#include "ModuleManager.h"
#include "CommandManager.h"
#include "ThreadPool.h"
#include "ControlCentreStubImpEx.h"
#include "WorkerOperateHelper.h"
#include "ProcessStatus.h"
#include "CsvParser.h"
#include "IncludeTemplate.h"
#include "IncludeModule.h"
#include "Player.h"

using namespace mdl;
using namespace evt;
using namespace util;
using namespace thd;

typedef std::map<std::string, std::string> TEMPLATE_NAMES_T;

static void LoadTemplateNames(TEMPLATE_NAMES_T& fileNames) {
	/* Declare the variables to be used */
	const char * filename = "templatefiles.csv";

	csv_parser file_parser;
	/* Define how many records we're gonna skip. This could be used to skip the column definitions. */
	file_parser.set_skip_lines(1);
	/* Specify the file to parse */
	WPError(file_parser.init(filename), filename);
	/* Here we tell the parser how to parse the file */
	file_parser.set_enclosed_char(csv_enclosure_char, ENCLOSURE_OPTIONAL);
	file_parser.set_field_term_char(csv_field_terminator);
	file_parser.set_line_term_char(csv_line_terminator);

	/* Check to see if there are more records, then grab each row one at a time */
	while(file_parser.has_more_rows())
	{
		csv_row row = file_parser.get_row();
		if(row.size() > 1U) {
			fileNames[row[0]] = TrimString(row[1]);
		}
	}
}

CGameServerRegister::CGameServerRegister()
{
}

CGameServerRegister::~CGameServerRegister()
{
}

void CGameServerRegister::InitStatusCallback()
{
	CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());

	CAutoPointer<GlobalMethodRS> pCPUUsageMethod(new GlobalMethodRS(GetCPUUsageStr));
	pCtrlCenStubImpEx->AddInfoMethod("CPU", pCPUUsageMethod);

	CAutoPointer<GlobalMethodRS> pMemoryUsageMethod(new GlobalMethodRS(GetMemoryUsageStr));
	pCtrlCenStubImpEx->AddInfoMethod("MemoryKB", pMemoryUsageMethod);

	CAutoPointer<GlobalMethodRS> pVirtualMemMethod(new GlobalMethodRS(GetVirtualMemoryUsageStr));
	pCtrlCenStubImpEx->AddInfoMethod("VirtualMemoryKB", pVirtualMemMethod);

	CAutoPointer<GlobalMethodRS> pIOReadMethod(new GlobalMethodRS(GetIOReadKBStr));
	pCtrlCenStubImpEx->AddInfoMethod("TotalIOReadKB", pIOReadMethod);

	CAutoPointer<GlobalMethodRS> pIOWriteMethod(new GlobalMethodRS(GetIOWriteKBStr));
	pCtrlCenStubImpEx->AddInfoMethod("TotalIOWriteKB", pIOWriteMethod);
}

void CGameServerRegister::InitTemplate()
{
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strFilePath(pConfig->GetString(APPCONFIG_TEMPLATEPATH));
	TEMPLATE_NAMES_T fileNames;
	LoadTemplateNames(fileNames);
	//////////////////////////////////////////////////////////////////////////
	TEMPLATE_NAMES_T::const_iterator it;
	// load config
	ASSERT(fileNames.end() != (it = fileNames.find("global_config")));
	ASSERT(CConfigTemplate::Pointer()->Init(strFilePath, it->second));
}

void CGameServerRegister::RegisterCommand()
{
	CommandManager::PTR_T pCmdMgr(CommandManager::Pointer());
	CAutoPointer<GlobalMethodRIP1> pCommandShowThreadPool(new GlobalMethodRIP1(CommandShowThreadPool));
	pCmdMgr->AddCommand("show-thread-pool", pCommandShowThreadPool, "Show the thread pool stats. >> show-thread-pool");

	CAutoPointer<GlobalMethodRIP1> pCommandAddAttrib(new GlobalMethodRIP1(CommandAddAttrib));
	pCmdMgr->AddCommand("add-attrib", pCommandAddAttrib, "add attrib. >> add-attrib <userId> "
		"<attrib type(0 PhyPower,1 Gem,2 Coin)> <value>");
	CAutoPointer<GlobalMethodRIP1> pCommandKickout(new GlobalMethodRIP1(CommandKickout));
	pCmdMgr->AddCommand("kick-out", pCommandKickout, "Kick the player out. >> kick-out <userId> ");
}

void CGameServerRegister::RegistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//register module
	CControlCentreModule::PTR_T pCtrlCentreModule(
		CControlCentreModule::Pointer());
	pFacade->RegisterModule(pCtrlCentreModule);

	CLoggingModule::PTR_T pLoggingModule(
		CLoggingModule::Pointer());
	pFacade->RegisterModule(pLoggingModule);

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	uint16_t u16ServerRegion = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERREGION);
	CAutoPointer<CPlayerModule> pPlayerModule(
		new CPlayerModule(PLAYER_MODEL_NAME, u16ServerRegion));
	pFacade->RegisterModule(pPlayerModule);
}

void CGameServerRegister::UnregistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//unregister module
	pFacade->RemoveModule(PLAYER_MODEL_NAME);

	CLoggingModule::PTR_T pLoggingModule(
		CLoggingModule::Pointer());
	pFacade->RemoveModule(pLoggingModule->GetModuleName());

	CControlCentreModule::PTR_T pCtrlCentreModule(
		CControlCentreModule::Pointer());
	pFacade->RemoveModule(pCtrlCentreModule->GetModuleName());
}

int CGameServerRegister::CommandShowThreadPool(const util::CWeakPointer<ArgumentBase>& arg)
{
	ThreadPool.ShowStats();
	return COMMAND_RESULT_SUCCESS;
}

int CGameServerRegister::CommandAddAttrib(const util::CWeakPointer<ArgumentBase>& arg)
{
    util::CWeakPointer<CommandArgument> pCommArg(arg);
    if(pCommArg.IsInvalid()) {
        return COMMAND_RESULT_FAIL;
    }
    uint32_t nUserId(0);
    int32_t nAttribType(-1);
    int32_t nValue(0);
    if(sscanf(pCommArg->GetParams().c_str(),"%u %d %d",&nUserId, &nAttribType, &nValue) < 3) {
        return COMMAND_RESULT_FAIL;
    }

    CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
    CAutoPointer<CPlayer> pPlayer(pChlMgr->GetPlayer(nUserId));
    if(!pPlayer.IsInvalid()) {
        CScopedPlayerMutex scopedPlayerMutex(pPlayer);
        pPlayer->AddAttribPoint((ePlayerAttribType)nAttribType, nValue);
    } else {
        printf("\n The userId = %u isn't online.\n", nUserId);
        return COMMAND_RESULT_FAIL;
    }

    return COMMAND_RESULT_SUCCESS;
}


//void CGameServerRegister::KeepServersRegister2(const uint64_t& timerId) {
//	printf("\n call KeepServersRegister2\n");
//	CTimerManager& timerManager = CTimerManager::getInstance();
//
//	timer_data_t timerData(300);
//
//	timerManager.Modify(timerId, TIMER_OPERATER_RESET);
//}
//
//void CGameServer::KeepServersRegister3(const uint64_t& timerId) {
//	printf("\n call KeepServersRegister3\n");
//
//}

int CGameServerRegister::CommandKickout(const util::CWeakPointer<ArgumentBase>& arg)
{
	// test crash
	//int* p = NULL; *p = 0;

	//uint64_t timerKey1 = CGuidFactory::getInstance().CreateGuid();

	//	uint64_t timerKey2 = CGuidFactory::getInstance().CreateGuid();

	//CGameServer& gameServer = CGameServer::getInstance();
	//CAutoPointer<CallbackP1<CGameServer, uint64_t> >
	//	callback(new CallbackP1<CGameServer, uint64_t>
	//	(&gameServer, &CGameServer::KeepServersRegister2, timerKey2));



	//CAutoPointer<CallbackP1<CGameServer, uint64_t> >
	//	callback3(new CallbackP1<CGameServer, uint64_t>
	//	(&gameServer, &CGameServer::KeepServersRegister3, timerKey2));

	//

	//CTimerManager& timerManager = CTimerManager::getInstance();
	//bool ret1 = timerManager.SetTimeout(timerKey1, 100, callback, true, true);

	//bool ret2 =	timerManager.SetTimeout(timerKey2, 600, callback3, true, true);


	//////////////////////////////////////////////////////////////////////////
	// TEST
	//CAutoPointer<CTimer> pTimer(timerManager.GetTimer(timerKey, callback3));
	//if(pTimer.IsInvalid()) {

	//} else {
	//	int nTimeLeft = pTimer->GetDelay();
	//	int inff = 1;
	//}

    util::CWeakPointer<CommandArgument> pCommArg(arg);
    if(pCommArg.IsInvalid()) {
        return COMMAND_RESULT_FAIL;
    }

    uint64_t nUserId(0);
    if(sscanf(pCommArg->GetParams().c_str(),I64FMTD, &nUserId) < 1) {
        return COMMAND_RESULT_FAIL;
    }

    if(CloseWorkerClient(nUserId)) {
        printf("\n The userId = "I64FMTD" have been ticked out.\n", nUserId);
    }
    return COMMAND_RESULT_SUCCESS;
}



