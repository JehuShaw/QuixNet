/*
 * File:   CacheServerRegister.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#include "Log.h"
#include "NodeDefines.h"
#include "AppConfig.h"
#include "CacheServerRegister.h"
#include "ModuleManager.h"
#include "ControlCentreStubImpEx.h"
#include "CommandManager.h"
#include "ProcessStatus.h"
#include "IncludeModule.h"

using namespace mdl;
using namespace util;
using namespace evt;


CCacheServerRegister::CCacheServerRegister()
{
}

CCacheServerRegister::~CCacheServerRegister()
{
}

void CCacheServerRegister::InitStatusCallback()
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

void CCacheServerRegister::InitTemplate()
{
}

void CCacheServerRegister::RegisterCommand()
{
}

void CCacheServerRegister::RegistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//register module
	CControlCentreModule::PTR_T pCtrlCentreModule(
		CControlCentreModule::Pointer());
	pFacade->RegisterModule(pCtrlCentreModule);
}

void CCacheServerRegister::UnregistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//unregister module
	CControlCentreModule::PTR_T pCtrlCentreModule(
		CControlCentreModule::Pointer());
	pFacade->RemoveModule(pCtrlCentreModule->GetModuleName());
}



