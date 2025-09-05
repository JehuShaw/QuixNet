/*
 * File:   LoginServerRegister.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#include "Log.h"
#include "NodeDefines.h"
#include "LoginServerRegister.h"
#include "ModuleManager.h"
#include "ControlCentreModule.h"
#include "ControlCentreStubImpEx.h"
#include "CsvParser.h"
#include "AgentMethod.h"
#include "IncludeModule.h"
#include "IncludeTemplate.h"
#include "ProcessStatus.h"

using namespace mdl;
using namespace util;
using namespace evt;

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

CLoginServerRegister::CLoginServerRegister()
{
}

CLoginServerRegister::~CLoginServerRegister()
{
}

void CLoginServerRegister::InitStatusCallback()
{
	CControlCentreStubImpEx::PTR_T pCtlCenStubImpEx(CControlCentreStubImpEx::Pointer());

	CAutoPointer<GlobalMethodRS> pCPUUsageMethod(new GlobalMethodRS(GetCPUUsageStr));
	pCtlCenStubImpEx->AddInfoMethod("CPU", pCPUUsageMethod);

	CAutoPointer<GlobalMethodRS> pMemoryUsageMethod(new GlobalMethodRS(GetMemoryUsageStr));
	pCtlCenStubImpEx->AddInfoMethod("MemoryKB", pMemoryUsageMethod);

	CAutoPointer<GlobalMethodRS> pVirtualMemMethod(new GlobalMethodRS(GetVirtualMemoryUsageStr));
	pCtlCenStubImpEx->AddInfoMethod("VirtualMemoryKB", pVirtualMemMethod);

	CAutoPointer<GlobalMethodRS> pIOReadMethod(new GlobalMethodRS(GetIOReadKBStr));
	pCtlCenStubImpEx->AddInfoMethod("TotalIOReadKB", pIOReadMethod);

	CAutoPointer<GlobalMethodRS> pIOWriteMethod(new GlobalMethodRS(GetIOWriteKBStr));
	pCtlCenStubImpEx->AddInfoMethod("TotalIOWriteKB", pIOWriteMethod);
}

void CLoginServerRegister::InitTemplate()
{
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strFilePath(pConfig->GetString(APPCONFIG_TEMPLATEPATH));
	TEMPLATE_NAMES_T fileNames;
	LoadTemplateNames(fileNames);
	//////////////////////////////////////////////////////////////////////////
	TEMPLATE_NAMES_T::const_iterator it;
	// load config
	//ASSERT(fileNames.end() != (it = fileNames.find("global_config")));
	//ASSERT(CGlobalConfig::Pointer()->Init(strFilePath, it->second));
}

void CLoginServerRegister::RegisterCommand()
{
}

void CLoginServerRegister::RegisterModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//register module
	CAutoPointer<CControlCentreModule> pCtrlCentreModule(
		new CControlCentreModule(CONTROLCENTRE_MODULE_NAME));
	pFacade->RegisterModule(pCtrlCentreModule);

	CAutoPointer<CLoginModule> pLoginModule(new CLoginModule(LOGIN_MODULE_NAME));
	pFacade->RegisterModule(pLoginModule);
}

void CLoginServerRegister::UnregisterModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//unregister module
	pFacade->RemoveModule(LOGIN_MODULE_NAME);

	pFacade->RemoveModule(CONTROLCENTRE_MODULE_NAME);

}

