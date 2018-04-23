/*
 * File:   RankServerRegister.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */
#include "Log.h"
#include "NodeDefines.h"
#include "AppConfig.h"
#include "RankServerRegister.h"
#include "ModuleManager.h"
#include "ControlCentreStubImpEx.h"
#include "WorkerOperateHelper.h"
#include "ProcessStatus.h"
#include "IncludeTemplate.h"
#include "CsvParser.h"
#include "CommandManager.h"
#include "IncludeModule.h"


using namespace mdl;
using namespace evt;
using namespace thd;
using namespace util;

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

CRankServerRegister::CRankServerRegister()
{
}

CRankServerRegister::~CRankServerRegister()
{
}

void CRankServerRegister::InitStatusCallback()
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

void CRankServerRegister::InitTemplate()
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

void CRankServerRegister::RegisterCommand()
{
	CommandManager::PTR_T pCmdMgr(CommandManager::Pointer());

	CAutoPointer<GlobalMethodRIP1> pCommandKickout(new GlobalMethodRIP1(CommandKickout));
	pCmdMgr->AddCommand("kick-out", pCommandKickout, "Kick the player out. >> kick-out <userId> ");
}

void CRankServerRegister::RegistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//register module
	CControlCentreModule::PTR_T pCtrlCentreModule(
		CControlCentreModule::Pointer());
	pFacade->RegisterModule(pCtrlCentreModule);

    CAutoPointer<CRankModule> pRankModule(
	new CRankModule(RANK_MODEL_NAME));
    pFacade->RegisterModule(pRankModule);
}

void CRankServerRegister::UnregistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//unregister module
    pFacade->RemoveModule(RANK_MODEL_NAME);

	CControlCentreModule::PTR_T pCtrlCentreModule(
		CControlCentreModule::Pointer());
	pFacade->RemoveModule(pCtrlCentreModule->GetModuleName());
}

int CRankServerRegister::CommandKickout(const util::CWeakPointer<ArgumentBase>& arg)
{
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


