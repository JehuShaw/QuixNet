/* 
 * File:   ControlCentreModule.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */
#include "ControlCentreModule.h"
#include "NodeDefines.h"
#include "Log.h"
#include "WorkerOperateHelper.h"
#include "ControlArgument.h"
#include "ControlEventManager.h"

using namespace mdl;
using namespace util;
using namespace evt;


CControlCentreModule::CControlCentreModule(const char* name)
	: CModule(name)
{
}

void CControlCentreModule::OnRegister(){
    OutputBasic("OnRegister");
}

void CControlCentreModule::OnRemove(){
    OutputBasic("OnRemove");
}

std::vector<int> CControlCentreModule::ListNotificationInterests()
{
	return std::vector<int>({
		N_CMD_MASTER_TRANSPORT,
		N_CMD_MASTER_SHUTDOWN,
		N_CMD_SERVER_PLAY,
		N_CMD_SERVER_STOP
	});
}

IModule::InterestList CControlCentreModule::ListProtocolInterests()
{
	InterestList interests;
	return interests;
}

void CControlCentreModule::HandleNotification(
	const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
    int32_t nCmd = request->GetName();
    switch(nCmd) {
    case N_CMD_MASTER_TRANSPORT:
		CaseMasterTransport(request, reply);
        break;
	case N_CMD_MASTER_SHUTDOWN:
		CaseShutdown(request, reply);
		break;
	case N_CMD_SERVER_PLAY:
		CaseStart(request, reply);
		break;
	case N_CMD_SERVER_STOP:
		CaseStop(request, reply);
		break;
	default:
		break;
    }
}

void CControlCentreModule::CaseMasterTransport(
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

	::node::DataPacket masterRequest;
	if(!ParseWorkerData(masterRequest, pRequest)) {
		OutputError("!ParseWorkerData");
		pResponse->set_result(SERVER_FAILURE);
		return;
	}
	
	if (N_CMD_MASTER_TRANSPORT == masterRequest.cmd()) {
		int nId = masterRequest.sub_cmd() - NODE_CONTROLCMD_OFFSET;
		if (nId < 0) {
			OutputError("nId < 0  nId = %d offset = %d", nId, NODE_CONTROLCMD_OFFSET);
			pResponse->set_result(SERVER_FAILURE);
			return;
		}

		CWeakPointer<CWrapPlayer> pPlayer(GetWorkerPlayer(request));
		CControlArgument controlArgument(masterRequest, pPlayer, pResponse);
		CAutoPointer<CControlArgument> pCtrlArgument(&controlArgument, false);
		CWeakPointer<CControlArgument> pWeakCtrlArg(pCtrlArgument);
		CControlEventManager::PTR_T pCtrlEvtMgr(CControlEventManager::Pointer());

		pResponse->set_result(pCtrlEvtMgr->DispatchEvent(nId, pWeakCtrlArg));
	} else {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<CWrapPlayer> pPlayer(pChlMgr->GetPlayer(masterRequest.route()));
		SendWorkerNotification(masterRequest, *pResponse, pPlayer, pRequest->sub_cmd());
	}
}

void CControlCentreModule::CaseShutdown(
	const CWeakPointer<mdl::INotification>& request,
	CWeakPointer<mdl::IResponse>& reply)
{
	// response
	CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if(pResponse.IsInvalid()) {
		return;
	}

	atomic_xchg8(&g_bExit, true);

	pResponse->set_result(SERVER_SUCCESS);
}

void CControlCentreModule::CaseStart(
	const CWeakPointer<mdl::INotification>& request,
	CWeakPointer<mdl::IResponse>& reply)
{
	// response
	CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if(pResponse.IsInvalid()) {
		return;
	}

	if (atomic_xchg(&g_serverStatus, SERVER_STATUS_START) == SERVER_STATUS_START) {
		// Already start !
		pResponse->set_result(SERVER_SUCCESS);
		return;
	}

	// TODO

	pResponse->set_result(SERVER_SUCCESS);
}

void CControlCentreModule::CaseStop(
	const CWeakPointer<mdl::INotification>& request,
	CWeakPointer<mdl::IResponse>& reply)
{
	// response
	CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if(pResponse.IsInvalid()) {
		return;
	}

	atomic_xchg(&g_serverStatus, SERVER_STATUS_STOP);

	pResponse->set_result(SERVER_SUCCESS);
}

