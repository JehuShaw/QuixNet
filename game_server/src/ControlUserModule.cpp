/* 
 * File:   ControlUserModule.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2020_12_1, 17:15
 */
#include "ControlUserModule.h"
#include "NodeDefines.h"
#include "Log.h"
#include "WorkerOperateHelper.h"
#include "ControlArgument.h"
#include "ControlEventManager.h"

using namespace mdl;
using namespace util;
using namespace evt;


CControlUserModule::CControlUserModule(const char* name)
	: CModule(name)
{
}

void CControlUserModule::OnRegister(){
    OutputBasic("OnRegister");
}

void CControlUserModule::OnRemove(){
    OutputBasic("OnRemove");
}

std::vector<int> CControlUserModule::ListNotificationInterests()
{
	return std::vector<int>({
		N_CMD_MASTER_TO_USER,
	});
}

IModule::InterestList CControlUserModule::ListProtocolInterests()
{
	return InterestList();
}

void CControlUserModule::HandleNotification(
	const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
    int32_t nCmd = request->GetName();
    switch(nCmd) {
    case N_CMD_MASTER_TO_USER:
		CaseMasterToUser(request, reply);
        break;
	default:
		break;
    }
}

void CControlUserModule::CaseMasterToUser(
	const CWeakPointer<mdl::INotification>& request, 
	CWeakPointer<mdl::IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if(pRequest.IsInvalid()) {
		return;;
	}

	// response
	CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if(pResponse.IsInvalid()) {
		return;
	}
	

	int nId = pRequest->sub_cmd() - NODE_CONTROLCMD_OFFSET;
	if (nId < 0) {
		OutputError("nId < 0  nId = %d offset = %d", nId, NODE_CONTROLCMD_OFFSET);
		pResponse->set_result(SERVER_FAILURE);
		return;
	}

	CWeakPointer<CWrapPlayer> pPlayer(GetWorkerPlayer(request));
	CControlArgument controlArgument(*pRequest, pPlayer, pResponse);
	CAutoPointer<CControlArgument> pCtrlArgument(&controlArgument, false);
	CWeakPointer<CControlArgument> pWeakCtrlArg(pCtrlArgument);
	CControlEventManager::PTR_T pCtrlEvtMgr(CControlEventManager::Pointer());

	pResponse->set_result(pCtrlEvtMgr->DispatchEvent(nId, pWeakCtrlArg));
}

