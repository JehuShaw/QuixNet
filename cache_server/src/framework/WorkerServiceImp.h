/* 
 * File:   WorkerServiceImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 11:25
 */

#ifndef WORKERSERVICEIMP_H
#define WORKERSERVICEIMP_H

#include "worker.rpcz.h"
#include "AutoPointer.h"
#include "Common.h"

class CPlayerBase;

class CWorkerServiceImp : public node::WorkerService
{
public:
	typedef util::CAutoPointer<CPlayerBase> (*CreatePlayerMethod)(uint64_t userId);
	typedef void (*ListInterestsMethod)(::node::InterestPacket&);

public:
    CWorkerServiceImp(
		const std::string& endPoint,
		const std::string& servantAddress,
		const std::string& serverName,
		uint32_t serverId,
		CreatePlayerMethod createPlayerMethod = NULL, 
		ListInterestsMethod listProtoMethod = NULL,
		ListInterestsMethod listNotifMethod = NULL);

	~CWorkerServiceImp(void);

	virtual void HandleProtocol(const ::node::DataPacket& request,
		::rpcz::reply< ::node::DataPacket> response);

	virtual void HandleNotification(const ::node::DataPacket& request,
		::rpcz::reply< ::node::DataPacket> response);

	virtual void ListProtocolInterests(const ::node::VoidPacket& request,
		::rpcz::reply< ::node::InterestPacket> response);

	virtual void ListNotificationInterests(const ::node::VoidPacket& request,
		::rpcz::reply< ::node::InterestPacket> response);

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

protected:
	CreatePlayerMethod m_createPlayerMethod;
	ListInterestsMethod m_listProtoMethod;
	ListInterestsMethod m_listNotifMethod;
    const std::string m_endPoint;
	const std::string m_servantAddress;
	const std::string m_serverName;
	const uint32_t m_serverId;
};

#endif /* WORKERSERVICEIMP_H */