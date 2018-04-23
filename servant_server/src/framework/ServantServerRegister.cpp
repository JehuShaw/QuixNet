/*
 * File:   ServantServerRegister.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */
#include "Log.h"
#include "NodeDefines.h"
#include "AppConfig.h"
#include "ServantServerRegister.h"
#include "ServantStubImp.h"
#include "ProcessStatus.h"
#include "ServantModule.h"

using namespace mdl;
using namespace evt;
using namespace util;
using namespace thd;


CServantServerRegister::CServantServerRegister()
{
}

CServantServerRegister::~CServantServerRegister()
{
}

void CServantServerRegister::InitStatusCallback()
{
	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());

	CAutoPointer<GlobalMethodRS> pCPUUsageMethod(new GlobalMethodRS(GetCPUUsageStr));
	pServantStubImp->AddInfoMethod("CPU", pCPUUsageMethod);

	CAutoPointer<GlobalMethodRS> pMemoryUsageMethod(new GlobalMethodRS(GetMemoryUsageStr));
	pServantStubImp->AddInfoMethod("MemoryKB", pMemoryUsageMethod);

	CAutoPointer<GlobalMethodRS> pVirtualMemMethod(new GlobalMethodRS(GetVirtualMemoryUsageStr));
	pServantStubImp->AddInfoMethod("VirtualMemoryKB", pVirtualMemMethod);

	CAutoPointer<GlobalMethodRS> pIOReadMethod(new GlobalMethodRS(GetIOReadKBStr));
	pServantStubImp->AddInfoMethod("TotalIOReadKB", pIOReadMethod);

	CAutoPointer<GlobalMethodRS> pIOWriteMethod(new GlobalMethodRS(GetIOWriteKBStr));
	pServantStubImp->AddInfoMethod("TotalIOWriteKB", pIOWriteMethod);
}


void CServantServerRegister::InitTemplate()
{
}

void CServantServerRegister::RegisterCommand()
{
}

void CServantServerRegister::RegistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//register module
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	uint16_t u16ServerId = (uint16_t)pConfig->GetInt(APPCONFIG_SERVERID);

    CAutoPointer<CServantModule> pServantModule(
		new CServantModule(SERVANT_MODEL_NAME, u16ServerId));
    pFacade->RegisterModule(pServantModule);
}

void CServantServerRegister::UnregistModule()
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	//unregister module
    pFacade->RemoveModule(SERVANT_MODEL_NAME);
}


