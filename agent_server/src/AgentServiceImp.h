/* 
 * File:   AgentServiceImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 11:25
 */

#ifndef AGENTSERVICEIMP_H
#define	AGENTSERVICEIMP_H

#include "WorkerServiceImp.h"

class CAgentServiceImp : public CWorkerServiceImp
{
public:
	CAgentServiceImp(
		const std::string& strServerBind,
		const std::string& servantAddress,
		const std::string& serverName,
		uint32_t serverId,
		CreatePlayerMethod createPlayerMethod = NULL, 
		ListInterestsMethod listProtoMethod = NULL,
		ListInterestsMethod listNotifMethod = NULL);

	~CAgentServiceImp(void);

	virtual void KickLogged(const ::node::DataPacket& request,
		::rpcz::reply< ::node::DataPacket> response);

	virtual void SendToClient(const ::node::DataPacket& request,
		::rpcz::reply< ::node::DataPacket> response);

	virtual void BroadcastToClient(const ::node::BroadcastRequest& request,
		::rpcz::reply< ::node::VoidPacket> response);

	virtual void SendToPlayer(const ::node::ForwardRequest& request,
		::rpcz::reply< ::node::DataPacket> response);

	virtual void PostToPlayer(const ::node::ForwardRequest& request,
		::rpcz::reply< ::node::VoidPacket> response);

	virtual void CloseClient(const ::node::DataPacket& request,
		::rpcz::reply< ::node::DataPacket> response);

	virtual void CloseAllClients(const ::node::BroadcastRequest& request,
		::rpcz::reply< ::node::VoidPacket> response);

	virtual void SendToWorker(const ::node::DataPacket& request,
		::rpcz::reply< ::node::DataPacket> response);
};

#endif /* AGENTSERVICEIMP_H */