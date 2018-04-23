/*
 * File:   MasterServerRegister.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */
#include "Log.h"
#include "NodeDefines.h"
#include "AppConfig.h"
#include "MasterServerRegister.h"
#include "MasterStubImp.h"
#include "CommandManager.h"
#include "IncludeModule.h"
#include "ProcessStatus.h"
#include "MasterServer.h"
#include "MasterCmdManager.h"
#include "msg_game_send_mail.pb.h"

using namespace mdl;
using namespace evt;
using namespace util;
using namespace thd;
using namespace ntwk;


CMasterServerRegister::CMasterServerRegister()
{
}

CMasterServerRegister::~CMasterServerRegister()
{
}

void CMasterServerRegister::InitStatusCallback()
{
	CMasterStubImp::PTR_T pMasterStubImp(CMasterStubImp::Pointer());

	CAutoPointer<GlobalMethodRS> pCPUUsageMethod(new GlobalMethodRS(GetCPUUsageStr));
	pMasterStubImp->AddInfoMethod("CPU", pCPUUsageMethod);

	CAutoPointer<GlobalMethodRS> pMemoryUsageMethod(new GlobalMethodRS(GetMemoryUsageStr));
	pMasterStubImp->AddInfoMethod("MemoryKB", pMemoryUsageMethod);

	CAutoPointer<GlobalMethodRS> pVirtualMemMethod(new GlobalMethodRS(GetVirtualMemoryUsageStr));
	pMasterStubImp->AddInfoMethod("VirtualMemoryKB", pVirtualMemMethod);

	CAutoPointer<GlobalMethodRS> pIOReadMethod(new GlobalMethodRS(GetIOReadKBStr));
	pMasterStubImp->AddInfoMethod("TotalIOReadKB", pIOReadMethod);

	CAutoPointer<GlobalMethodRS> pIOWriteMethod(new GlobalMethodRS(GetIOWriteKBStr));
	pMasterStubImp->AddInfoMethod("TotalIOWriteKB", pIOWriteMethod);
}

void CMasterServerRegister::InitTemplate()
{
}

void CMasterServerRegister::RegisterCommand()
{
	CommandManager::PTR_T pCmdMgr(CommandManager::Pointer());
	CAutoPointer<GlobalMethodRIP1> pCommandSendMail(new GlobalMethodRIP1(CommandSendMail));
	pCmdMgr->AddCommand("send-mail", pCommandSendMail, "send mail. >> send-mail <serverName> "
		"<receierId> <title> <content>");

	CAutoPointer<GlobalMethodRIP1> pCommandAutoRestart(new GlobalMethodRIP1(CommandAutoRestart));
	pCmdMgr->AddCommand("auto-restart", pCommandAutoRestart, "If any server crash, do you want auto restart it? "
		">> auto-restart <0 close, 1 open> ");

	CAutoPointer<GlobalMethodRIP1> pCommandRestart(new GlobalMethodRIP1(CommandRestart));
	pCmdMgr->AddCommand("restart", pCommandRestart, "Restart the server by serverId. "
		">> restart <serverId> ");

	CAutoPointer<GlobalMethodRIP1> pCommandShutdown(new GlobalMethodRIP1(CommandShutdown));
	pCmdMgr->AddCommand("shutdown", pCommandShutdown, "Shutdown the server by serverId. "
		">> shutdown <serverId> ");

	CAutoPointer<GlobalMethodRIP1> pCommandErase(new GlobalMethodRIP1(CommandErase));
	pCmdMgr->AddCommand("erase", pCommandErase, "Erase the server that isn't alive by serverId. "
		">> erase <serverId> ");

	CAutoPointer<GlobalMethodRIP1> pCommandPlay(new GlobalMethodRIP1(CommandPlay));
	pCmdMgr->AddCommand("play", pCommandPlay, "Play all the server. >> play");

	CAutoPointer<GlobalMethodRIP1> pCommandStop(new GlobalMethodRIP1(CommandStop));
	pCmdMgr->AddCommand("stop", pCommandPlay, "Stop all the server. >> stop");
}

void CMasterServerRegister::RegistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	CMasterServer::PTR_T pMaster(CMasterServer::Pointer());
	//register module
	CAutoPointer<CMasterModule> pMasterModule(new CMasterModule(
		pConfig->GetString(APPCONFIG_SERVERNAME).c_str(),
		(uint16_t)pConfig->GetInt(APPCONFIG_SERVERID),
		pConfig->GetString(APPCONFIG_SERVERBIND),
		pConfig->GetString(APPCONFIG_SERVERACCEPT),
		pMaster->GetProcessPath(),
		pConfig->GetString(APPCONFIG_PROJECTNAME)));

	pFacade->RegisterModule(pMasterModule);
}

void CMasterServerRegister::UnregistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	//unregister module
	pFacade->RemoveModule(pConfig->GetString(APPCONFIG_SERVERNAME).c_str());
}

int CMasterServerRegister::CommandSendMail(const util::CWeakPointer<ArgumentBase>& arg)
{
    util::CWeakPointer<CommandArgument> pCommArg(arg);
    if(pCommArg.IsInvalid()) {
        return COMMAND_RESULT_FAIL;
    }

    char szServerName[eBUF_SIZE_128] = {'\0'};
    uint64_t nReceierId(0);
    char szTitle[eBUF_SIZE_128] = {'\0'};
    char szContent[eBUF_SIZE_512] = {'\0'};
    if(sscanf(pCommArg->GetParams().c_str(),"%s "I64FMTD" %s %s",
        szServerName, &nReceierId, szTitle, szContent) < 4)
    {
        return COMMAND_RESULT_FAIL;
    }
    szTitle[sizeof(szTitle) - 1] = '\0';
    szContent[sizeof(szContent) - 1] = '\0';

    ::game::SendMailPacket sendMailPacket;
    sendMailPacket.set_sender_type(MAIL_SENDER_GM);
    if(ID_NULL == nReceierId) {
        sendMailPacket.set_receiver_type(MAIL_RECEIVER_GLOBAL_USER);
    } else {
        sendMailPacket.set_receiver_type(MAIL_RECEIVER_SPECIAL_USER);
        sendMailPacket.add_receivers(nReceierId);
    }
    sendMailPacket.set_title(szTitle, strlen(szTitle));
    sendMailPacket.set_content(szContent, strlen(szContent));

	AppConfig::PTR_T pConfig(AppConfig::Pointer());
    CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
    pMasterCmdMgr->SendByUserId(szServerName, nReceierId,
		C_CMD_CTM_SEND_MAIL, sendMailPacket);
    return COMMAND_RESULT_SUCCESS;
}

int CMasterServerRegister::CommandAutoRestart(const util::CWeakPointer<evt::ArgumentBase>& arg)
{
	util::CWeakPointer<CommandArgument> pCommArg(arg);
	if(pCommArg.IsInvalid()) {
		return COMMAND_RESULT_FAIL;
	}

	int nOpen = FALSE;
	if(sscanf(pCommArg->GetParams().c_str(),"%d", &nOpen) < 1)
	{
		return COMMAND_RESULT_FAIL;
	}

	if(FALSE == nOpen) {
		atomic_xchg8(&g_bAutoRestart, false);
	} else if(TRUE == nOpen) {
		atomic_xchg8(&g_bAutoRestart, true);
	} else {
		return COMMAND_RESULT_FAIL;
	}

	return COMMAND_RESULT_SUCCESS;
}

int CMasterServerRegister::CommandRestart(const util::CWeakPointer<evt::ArgumentBase>& arg)
{
	util::CWeakPointer<CommandArgument> pCommArg(arg);
	if(pCommArg.IsInvalid()) {
		return COMMAND_RESULT_FAIL;
	}

	uint16_t serverId = ID_NULL;
	if(sscanf(pCommArg->GetParams().c_str(),"%hu", &serverId) < 1)
	{
		return COMMAND_RESULT_FAIL;
	}
	CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
	int32_t nResult = pMasterCmdMgr->Restart(serverId);
	if(TRUE == nResult) {
		return COMMAND_RESULT_SUCCESS;
	}
	return COMMAND_RESULT_FAIL;
}

int CMasterServerRegister::CommandShutdown(const util::CWeakPointer<evt::ArgumentBase>& arg)
{
	util::CWeakPointer<CommandArgument> pCommArg(arg);
	if(pCommArg.IsInvalid()) {
		return COMMAND_RESULT_FAIL;
	}

	uint16_t serverId = ID_NULL;
	if(sscanf(pCommArg->GetParams().c_str(),"%hu", &serverId) < 1)
	{
		return COMMAND_RESULT_FAIL;
	}
	CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
	int32_t nResult = pMasterCmdMgr->Shutdown(serverId);
	if(TRUE == nResult) {
		return COMMAND_RESULT_SUCCESS;
	}
	return COMMAND_RESULT_FAIL;
}

int CMasterServerRegister::CommandErase(const util::CWeakPointer<evt::ArgumentBase>& arg)
{
	util::CWeakPointer<CommandArgument> pCommArg(arg);
	if(pCommArg.IsInvalid()) {
		return COMMAND_RESULT_FAIL;
	}

	uint16_t serverId = ID_NULL;
	if(sscanf(pCommArg->GetParams().c_str(),"%hu", &serverId) < 1)
	{
		return COMMAND_RESULT_FAIL;
	}

	CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
	int32_t nResult = pMasterCmdMgr->Erase(serverId);
	if(TRUE == nResult) {
		return COMMAND_RESULT_SUCCESS;
	}
	return COMMAND_RESULT_FAIL;
}

int CMasterServerRegister::CommandPlay(const util::CWeakPointer<evt::ArgumentBase>& arg)
{
	CMasterServer::PTR_T pMaster(CMasterServer::Pointer());
	pMaster->RemoveAutoPlayTimer();
	pMaster->OnServerPlay();

	return COMMAND_RESULT_SUCCESS;
}

int CMasterServerRegister::CommandStop(const util::CWeakPointer<evt::ArgumentBase>& arg)
{
	CMasterServer::PTR_T pMaster(CMasterServer::Pointer());
	pMaster->RemoveAutoPlayTimer();

	atomic_xchg(&g_serverStatus, SERVER_STATUS_STOP);

	CNodeModule::BroadcastAllNodes(N_CMD_SERVER_STOP);

	return COMMAND_RESULT_SUCCESS;
}





