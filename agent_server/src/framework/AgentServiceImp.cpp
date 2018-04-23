#include "AgentServiceImp.h"
#include "AgentServer.h"
#include "PlayerBase.h"
#include "WorkerOperateHelper.h"


CAgentServiceImp::CAgentServiceImp(
	const std::string& strServerBind,
	const std::string& servantAddress,
	const std::string& serverName,
	uint16_t serverId,
	CreatePlayerMethod createPlayerMethod /*= NULL*/, 
	ListInterestsMethod listProtoMethod /*= NULL*/,
	ListInterestsMethod listNotifMethod /*= NULL*/)

	:CWorkerServiceImp(strServerBind, servantAddress,
	serverName, serverId, createPlayerMethod,
	listProtoMethod, listNotifMethod)
{
}

CAgentServiceImp::~CAgentServiceImp(void)
{
}

void CAgentServiceImp::SendToClient(const ::node::DataPacket& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	CAgentServer::PTR_T pAgentServer(CAgentServer::Pointer());
	bool rt = pAgentServer->SendToClient(request.route(), request);

    ::node::DataPacket dataPacket;
    dataPacket.set_cmd(request.cmd());
    dataPacket.set_result(rt);
    response.send(dataPacket);
}

void CAgentServiceImp::CloseClient(const ::node::DataPacket& request,
    ::rpcz::reply< ::node::DataPacket> response)
{
    CAgentServer::PTR_T pAgentServer(CAgentServer::Pointer());
    pAgentServer->CloseClient(request.route());

    ::node::DataPacket dataPacket;
    dataPacket.set_cmd(request.cmd());
    dataPacket.set_result(TRUE);
    response.send(dataPacket);
}

void CAgentServiceImp::SendToWorker(const ::node::DataPacket& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	::node::DataPacket dspResponse;
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(request.route()));
	if(pPlayer.IsInvalid()) {
		SendWorkerNotification(request, dspResponse, pPlayer);
	} else {
		CScopedPlayerMutex scopedPlayerMutex(pPlayer);
		SendWorkerNotification(request, dspResponse, pPlayer);
	}
	dspResponse.set_cmd(request.cmd());
	response.send(dspResponse);
}

void CAgentServiceImp::KickLogged(const ::node::DataPacket& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	::node::DataPacket dataPacket;
	dataPacket.set_cmd(request.cmd());

	CAgentServer::PTR_T pAgentServer(CAgentServer::Pointer());
	if(pAgentServer->KickLogged(request.route())) {
		dataPacket.set_result(TRUE);
	} else {
		dataPacket.set_result(FALSE);
	}

	response.send(dataPacket);
}

