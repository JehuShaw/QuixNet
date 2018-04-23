/* 
 * File:   AgentModule.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */
#include "AgentModule.h"
#include "NodeDefines.h"
#include "Log.h"
#include "WorkerOperateHelper.h"
#include "GuidFactory.h"
#include "TimerManager.h"
#include "CallBack.h"
#include "AgentServer.h"

using namespace mdl;
using namespace util;
using namespace evt;


CAgentModule::CAgentModule(const char* name) 
	: CModule(name)
{
}

void CAgentModule::OnRegister(){
    OutputBasic("OnRegister");
}

void CAgentModule::OnRemove(){
    OutputBasic("OnRemove");
}

std::vector<int> CAgentModule::ListNotificationInterests()
{
	std::vector<int> interests;
	interests.push_back(N_CMD_SERVER_STOP);
	return interests;
}

IModule::InterestList CAgentModule::ListProtocolInterests()
{
	InterestList interests;
	return interests;
}

void CAgentModule::HandleNotification(
	const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
    int32_t nCmd = request->GetName();
    switch(nCmd) {
	case N_CMD_SERVER_STOP:
		CaseStop(request, reply);
		break;
	default:
		break;
    }
}

void CAgentModule::CaseStop(
	const CWeakPointer<mdl::INotification>& request,
	CWeakPointer<mdl::IResponse>& reply)
{
	// response
	CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if(pResponse.IsInvalid()) {
		return;
	}
	// Set server stop state
	atomic_xchg(&g_serverStatus, SERVER_STATUS_STOP);
	// Tell all client
	::node::DataPacket dispatchResponse;
	dispatchResponse.set_cmd(P_CMD_S_LOGOUT);
	dispatchResponse.set_result(SERVER_ERROR_SERVER_STOP);
	BroadcastWorkerPacketToClient(dispatchResponse, ID_NULL);
	// Close all socket
	uint64_t timerId = CGuidFactory::Pointer()->CreateGuid();
	CAutoPointer<CallBackFuncP0> callback(new CallBackFuncP0
		(CAgentModule::CloseAllClient));
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->SetTimeout(timerId, KEEP_FOR_CLOSE_CONNECTION, callback);

	pResponse->set_result(TRUE);
}

void CAgentModule::CloseAllClient()
{
	std::vector<uint64_t> clients;
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	pChlMgr->IteratorClient(clients);

	CAgentServer::PTR_T pAgent(CAgentServer::Pointer());
	std::vector<uint64_t>::const_iterator it(clients.begin());
	for(; clients.end() != it; ++it) {
		pAgent->CloseClient(*it);
	}
}

