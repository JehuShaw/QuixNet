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
#include "PlayerBasic.h"

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
    while (file_parser.has_more_rows())
    {
        csv_row row = file_parser.get_row();
        if (row.size() > 1U) {
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
    ASSERT(CGlobalConfig::Pointer()->Init(strFilePath, it->second));

    // load character
    ASSERT(fileNames.end() != (it = fileNames.find("character")));
    ASSERT(CCharacterConfig::Pointer()->Init(strFilePath, it->second));


    // 地图
    ASSERT(fileNames.end() != (it = fileNames.find("map")));
    ASSERT(CMapConfig::Pointer()->Init(strFilePath, it->second));

    // 角色等级经验表
    ASSERT(fileNames.end() != (it = fileNames.find("level_exp")));
    ASSERT(CLevelExpConfig::Pointer()->Init(strFilePath, it->second));

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

void CGameServerRegister::RegisterModule()
{
    CFacade::PTR_T pFacade(CFacade::Pointer());
    //register module
    CAutoPointer<CControlCentreModule> pCtrlCentreModule(
        new CControlCentreModule(CONTROLCENTRE_MODULE_NAME));
    pFacade->RegisterModule(pCtrlCentreModule);

    CAutoPointer<CControlUserModule> pCtrlUserModule(
        new CControlUserModule(CONTROLUSER_MODULE_NAME));
    pFacade->RegisterModule(pCtrlUserModule);

    CLoggingModule::PTR_T pLoggingModule(
        CLoggingModule::Pointer());
    pFacade->RegisterModule(pLoggingModule);

    //AppConfig::PTR_T pConfig(AppConfig::Pointer());
    //uint16_t u16ServerId = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERID);
  
    // Register Player module.
    CAutoPointer<CPlayerModule> pPlayerModule(
        new CPlayerModule(PLAYER_MODEL_NAME));
    pFacade->RegisterModule(pPlayerModule);

}

void CGameServerRegister::UnregisterModule()
{
    CFacade::PTR_T pFacade(CFacade::Pointer());
    //unregister module


    pFacade->RemoveModule(PLAYER_BASIC_MODEL_NAME);


    CLoggingModule::PTR_T pLoggingModule(
        CLoggingModule::Pointer());
    pFacade->RemoveModule(pLoggingModule->GetModuleName());

    pFacade->RemoveModule(CONTROLUSER_MODULE_NAME);

    pFacade->RemoveModule(CONTROLCENTRE_MODULE_NAME);
}

int CGameServerRegister::CommandShowThreadPool(const util::CWeakPointer<ArgumentBase>& arg)
{
    ThreadPool.ShowStats();
    return COMMAND_RESULT_SUCCESS;
}

int CGameServerRegister::CommandAddAttrib(const util::CWeakPointer<ArgumentBase>& arg)
{
    util::CWeakPointer<CommandArgument> pCommArg(arg);
    if (pCommArg.IsInvalid()) {
        return COMMAND_RESULT_FAIL;
    }
    uint64_t userId = ID_NULL;
    int32_t nAttribType(-1);
    int32_t nValue(0);
    if (sscanf(pCommArg->GetParams().c_str(), I64FMTD " %d %d", &userId, &nAttribType, &nValue) < 3) {
        return COMMAND_RESULT_FAIL;
    }

    CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
    CAutoPointer<CPlayer> pPlayer(pChlMgr->GetPlayer(userId));
    if (!pPlayer.IsInvalid()) {
        CAutoPointer<CPlayerBasic> pPlayerBasic(pPlayer->GetUnit(PLAYER_UNIT_BASIC));
        if (!pPlayerBasic.IsInvalid()) {
            pPlayerBasic->AddAttribPoint((ePlayerAttribType)nAttribType, nValue);
        }
        else {
            printf("\n Can't found CPlayerBasic instance. userId = " I64FMTD " \n", userId);
            return COMMAND_RESULT_FAIL;
        }
    }
    else {
        printf("\n The userId = " I64FMTD " isn't online.\n", userId);
        return COMMAND_RESULT_FAIL;
    }

    return COMMAND_RESULT_SUCCESS;
}

int CGameServerRegister::CommandKickout(const util::CWeakPointer<ArgumentBase>& arg)
{
    util::CWeakPointer<CommandArgument> pCommArg(arg);
    if (pCommArg.IsInvalid()) {
        return COMMAND_RESULT_FAIL;
    }

    uint64_t nUserId(0);
    if (sscanf(pCommArg->GetParams().c_str(), I64FMTD, &nUserId) < 1) {
        return COMMAND_RESULT_FAIL;
    }

    if (CloseWorkerClient(nUserId)) {
        printf("\n The userId = " I64FMTD " have been ticked out.\n", nUserId);
    }
    return COMMAND_RESULT_SUCCESS;
}



