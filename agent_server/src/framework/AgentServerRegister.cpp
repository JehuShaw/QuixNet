/*
 * File:   AgentServerRegister.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_10_7 PM 11:45
 */

#include "Log.h"
#include "NodeDefines.h"
#include "AppConfig.h"
#include "AgentServerRegister.h"
#include "ModuleManager.h"
#include "ControlCentreStubImpEx.h"
#include "ProcessStatus.h"
#include "IncludeModule.h"


using namespace mdl;
using namespace evt;
using namespace thd;
using namespace util;


CAgentServerRegister::CAgentServerRegister()
{
}

CAgentServerRegister::~CAgentServerRegister()
{
}

void CAgentServerRegister::InitStatusCallback()
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

void CAgentServerRegister::InitTemplate()
{
}

void CAgentServerRegister::RegisterCommand()
{
}

void CAgentServerRegister::RegistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//register module
	CControlCentreModule::PTR_T pCtrlCentreModule(
		CControlCentreModule::Pointer());
	pFacade->RegisterModule(pCtrlCentreModule);
	// register agent module
	CAutoPointer<CAgentModule> pAgentModule(
		new CAgentModule(AGENT_MODULE_NAME));
	pFacade->RegisterModule(pAgentModule);
}

void CAgentServerRegister::UnregistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//unregister module
	CControlCentreModule::PTR_T pCtrlCentreModule(
		CControlCentreModule::Pointer());
	pFacade->RemoveModule(pCtrlCentreModule->GetModuleName());
	// unregister agent module
	pFacade->RemoveModule(AGENT_MODULE_NAME);

}





