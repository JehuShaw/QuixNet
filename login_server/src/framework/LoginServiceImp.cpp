#include "LoginServiceImp.h"
#include "LoginServer.h"

CLoginServiceImp::CLoginServiceImp(
	const std::string& endPoint,
	const std::string& servantAddress,
	const std::string& serverName,
	uint32_t serverId,
	CreatePlayerMethod createPlayerMethod /*= NULL*/, 
	ListInterestsMethod listProtoMethod /*= NULL*/,
	ListInterestsMethod listNotifMethod /*= NULL*/)

	: CWorkerServiceImp(
		endPoint, servantAddress,
		serverName, serverId, createPlayerMethod,
		listProtoMethod, listNotifMethod)
{
}

CLoginServiceImp::~CLoginServiceImp(void)
{
}

void CLoginServiceImp::SendToClient(const ::node::DataPacket& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	CLoginServer::PTR_T pLoginServer(CLoginServer::Pointer());
	pLoginServer->SendToClient(request.route(), request);
}
