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
#include "CacheOperateHelper.h"
#include "LocalIDFactory.h"
#include "TimerManager.h"
#include "CallBack.h"
#include "AgentServer.h"
#include "msg_node_get_character.pb.h"

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
	return std::vector<int>({ N_CMD_SERVER_STOP, N_CMD_GET_CHARACTER });
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
	case N_CMD_GET_CHARACTER:
		CaseGetCharacter(request, reply);
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
	BroadcastWorkerPacketToClient(dispatchResponse, ID_NULL, NULL);
	// Close all socket
	uint64_t timerId = CLocalIDFactory::Pointer()->GenerateID();
	CAutoPointer<CallBackFuncP0> callback(new CallBackFuncP0
		(CAgentModule::CloseAllClient));
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->SetTimeout(timerId, KEEP_FOR_CLOSE_CONNECTION, callback);

	pResponse->set_result(SERVER_SUCCESS);
}

void CAgentModule::CaseGetCharacter(
	const CWeakPointer<mdl::INotification>& request,
	CWeakPointer<mdl::IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if (pRequest.IsInvalid()) {
		return;
	}

	// response
	CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if (pResponse.IsInvalid()) {
		return;
	}

	uint64_t userId = pRequest->route();

	::node::GetCharacterRequest getRequest;
	if (!ParseWorkerData(getRequest, pRequest)) {
		OutputError("!ParseWorkerData userId = " I64FMTD, userId);
		pResponse->set_result(PARSE_PACKAGE_FAIL);
		return;
	}

	if(getRequest.kick()) {
		CAgentServer::PTR_T pAgent(CAgentServer::Pointer());
		pAgent->CloseClient(userId, LOGOUT_KICK);
	}

}

void CAgentModule::CloseAllClient()
{
	std::vector<uint64_t> clients;
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	pChlMgr->IteratorClient(clients);

	CAgentServer::PTR_T pAgent(CAgentServer::Pointer());
	std::vector<uint64_t>::const_iterator it(clients.begin());
	for(; clients.end() != it; ++it) {
		pAgent->CloseClient(*it, LOGOUT_CLOSEALL);
	}
}

