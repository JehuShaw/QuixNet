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

using namespace mdl;
using namespace util;
using namespace evt;


CControlCentreModule::CControlCentreModule() 
	: CModule(CONTROLCENTRE_MODULE_NAME), m_arrEvent(C_CMD_CTM_SIZE - NODE_CONTROLCMD_OFFSET)
{
}

bool CControlCentreModule::AddEventListener(eControlCMD cmd, const util::CAutoPointer<evt::MethodRIP1Base> method) {
	int nId = cmd - NODE_CONTROLCMD_OFFSET;
	if(nId < 0) {
		OutputError("nId < 0  nId = %d offset = %d", nId, NODE_CONTROLCMD_OFFSET);
		assert(false);
		return false;
	}

	return m_arrEvent.AddEventListener(nId, method);
}

bool CControlCentreModule::HasEventListener(eControlCMD cmd) {
	int nId = cmd - NODE_CONTROLCMD_OFFSET;
	if(nId < 0) {
		OutputError("nId < 0  nId = %d offset = %d", nId, NODE_CONTROLCMD_OFFSET);
		assert(false);
		return false;
	}

	return m_arrEvent.HasEventListener(nId);
}

void CControlCentreModule::RemoveEventListener(eControlCMD cmd) {
	int nId = cmd - NODE_CONTROLCMD_OFFSET;
	if(nId < 0) {
		OutputError("nId < 0  nId = %d offset = %d", nId, NODE_CONTROLCMD_OFFSET);
		assert(false);
		return;
	}

	m_arrEvent.RemoveEventListener(nId);
}

void CControlCentreModule::OnRegister(){
    OutputBasic("OnRegister");
}

void CControlCentreModule::OnRemove(){
    OutputBasic("OnRemove");
}

std::vector<int> CControlCentreModule::ListNotificationInterests()
{
	std::vector<int> interests;
    interests.push_back(N_CMD_MASTER_TRANSPORT);
	interests.push_back(N_CMD_MASTER_SHUTDOWN);
	interests.push_back(N_CMD_SERVER_PLAY);
	interests.push_back(N_CMD_SERVER_STOP);
	return interests;
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
		pResponse->set_result(FALSE);
		return;
	}
	
	int nId = masterRequest.cmd() - NODE_CONTROLCMD_OFFSET;
	if(nId < 0) {
		OutputError("nId < 0  nId = %d offset = %d", nId, NODE_CONTROLCMD_OFFSET);
		pResponse->set_result(FALSE);
		return;
	}

	CAutoPointer<CControlArgument> pCtrlArgument(new CControlArgument(masterRequest));
	CWeakPointer<CControlArgument> pWeakCtrlArg(pCtrlArgument);

	int nResult = m_arrEvent.DispatchEvent(nId, pWeakCtrlArg);

	pResponse->set_result(nResult);
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

	pResponse->set_result(TRUE);
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

	atomic_xchg(&g_serverStatus, SERVER_STATUS_START);

	pResponse->set_result(TRUE);
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

	pResponse->set_result(TRUE);
}

