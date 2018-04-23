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
#include "AgentMethod.h"
#include "IncludeModule.h"
#include "ProcessStatus.h"

using namespace mdl;
using namespace util;
using namespace evt;

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
}

void CLoginServerRegister::RegisterCommand()
{
}

void CLoginServerRegister::RegistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//register module
	CControlCentreModule::PTR_T pCtrlCenModule(CControlCentreModule::Pointer());
	pFacade->RegisterModule(pCtrlCenModule);
	CAutoPointer<CLoginModule> pLoginModule(new CLoginModule(LOGIN_MODULE_NAME));
	pFacade->RegisterModule(pLoginModule);
}

void CLoginServerRegister::UnregistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//unregister module
	pFacade->RemoveModule(LOGIN_MODULE_NAME);
	CControlCentreModule::PTR_T pCtrlCenModule(CControlCentreModule::Pointer());
	pFacade->RemoveModule(pCtrlCenModule->GetModuleName());
}

