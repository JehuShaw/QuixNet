#include "AgentServiceImp.h"
#include "AgentServer.h"
#include "PlayerBase.h"
#include "WorkerOperateHelper.h"


CAgentServiceImp::CAgentServiceImp(
	const std::string& endPoint,
	const std::string& servantAddress,
	const std::string& serverName,
	uint32_t serverId,
	CreatePlayerMethod createPlayerMethod /*= NULL*/, 
	ListInterestsMethod listProtoMethod /*= NULL*/,
	ListInterestsMethod listNotifMethod /*= NULL*/)

	: CWorkerServiceImp(endPoint, servantAddress,
	serverName, serverId, createPlayerMethod,
	listProtoMethod, listNotifMethod)
{
}

CAgentServiceImp::~CAgentServiceImp(void)
{
}

void CAgentServiceImp::KickLogged(const ::node::DataPacket& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	::node::DataPacket dataPacket;
	dataPacket.set_cmd(request.cmd());

	CAgentServer::PTR_T pAgentServer(CAgentServer::Pointer());
	if (pAgentServer->KickLogged(request.route())) {
		dataPacket.set_result(SERVER_SUCCESS);
	} else {
		dataPacket.set_result(SERVER_FAILURE);
	}
	response.send(dataPacket);
}

void CAgentServiceImp::SendToClient(const ::node::DataPacket& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	std::string bytes;
	if (!request.SerializeToString(&bytes)) {
		OutputError("!request.SerializeToString(bytes)");
		return;
	}
	CAgentServer::PTR_T pAgentServer(CAgentServer::Pointer());
	bool rt = pAgentServer->SendToClient(request.route(), bytes);

    ::node::DataPacket dataPacket;
    dataPacket.set_cmd(request.cmd());
    dataPacket.set_result(rt);
    response.send(dataPacket);

	OutputDebug("pAgentServer->SendToClient() = %d  route = " I64FMTD " cmd = %d ",
		(int)rt, request.route(), request.cmd());
}

void CAgentServiceImp::BroadcastToClient(const ::node::BroadcastRequest& request,
	::rpcz::reply< ::node::VoidPacket> response)
{
	::node::VoidPacket voidResponse;
	response.send(voidResponse);

	CAgentServer::PTR_T pAgentServer(CAgentServer::Pointer());
	int nSizeIn = request.includeids_size();
	if (nSizeIn > 0) {
		for (int i = 0; i < nSizeIn; ++i) {
			pAgentServer->SendToClient(request.includeids(i), request.data());
		}
	} else {
		std::set<uint64_t> excludeIds;
		int nSizeEx = request.excludeids_size();
		for (int j = 0; j < nSizeEx; ++j) {
			excludeIds.insert(request.excludeids(j));
		}
		pAgentServer->BroadcastToClient(request.data(), excludeIds);
	}
}

void CAgentServiceImp::SendToPlayer(const ::node::ForwardRequest& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	const ::node::DataPacket& dpRequest = request.data();
	int nType = request.mapid();

	::node::DataPacket dpResponse;
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(dpRequest.route()));
	if(pPlayer.IsInvalid()) {
		SendWorkerNotification(dpRequest, dpResponse, pPlayer, nType);
	} else {
		CScopedPlayerMutex scopedPlayerMutex(pPlayer);
		SendWorkerNotification(dpRequest, dpResponse, pPlayer, nType);
	}

	dpResponse.set_cmd(dpRequest.cmd());
	response.send(dpResponse);
}

void CAgentServiceImp::PostToPlayer(const ::node::ForwardRequest& request,
	::rpcz::reply< ::node::VoidPacket> response)
{
	const ::node::DataPacket& dpRequest = request.data();
	int nType = request.mapid();

	::node::DataPacket dpResponse;
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(dpRequest.route()));
	if (pPlayer.IsInvalid()) {
		SendWorkerNotification(dpRequest, dpResponse, pPlayer, nType);
	} else {
		CScopedPlayerMutex scopedPlayerMutex(pPlayer);
		SendWorkerNotification(dpRequest, dpResponse, pPlayer, nType);
	}

	::node::VoidPacket voidPacket;
	response.send(voidPacket);
}

void CAgentServiceImp::CloseClient(const ::node::DataPacket& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	::node::DataPacket dataPacket;
	dataPacket.set_cmd(request.cmd());
	dataPacket.set_result(SERVER_SUCCESS);
	response.send(dataPacket);

	CAgentServer::PTR_T pAgentServer(CAgentServer::Pointer());
	pAgentServer->CloseClient(request.route(), request.sub_cmd());
}

void CAgentServiceImp::CloseAllClients(const ::node::BroadcastRequest& request,
	::rpcz::reply< ::node::VoidPacket> response)
{
	::node::VoidPacket voidResponse;
	response.send(voidResponse);

	CAgentServer::PTR_T pAgentServer(CAgentServer::Pointer());
	int nSizeIn = request.includeids_size();
	if (nSizeIn > 0) {
		for (int i = 0; i < nSizeIn; ++i) {
			pAgentServer->CloseClient(request.includeids(i), atoi(request.data().c_str()));
		}
	} else {
		std::set<uint64_t> excludeIds;
		int nSizeEx = request.excludeids_size();
		for (int j = 0; j < nSizeEx; ++j) {
			excludeIds.insert(request.excludeids(j));
		}
		pAgentServer->CloseAllClients(excludeIds, atoi(request.data().c_str()));
	}
}

void CAgentServiceImp::SendToWorker(const ::node::DataPacket& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	::node::DataPacket dspResponse;
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(request.route()));
	if (pPlayer.IsInvalid()) {
		SendWorkerNotification(request, dspResponse, pPlayer);
	} else {
		CScopedPlayerMutex scopedPlayerMutex(pPlayer);
		SendWorkerNotification(request, dspResponse, pPlayer);
	}
	dspResponse.set_cmd(request.cmd());
	response.send(dspResponse);
}