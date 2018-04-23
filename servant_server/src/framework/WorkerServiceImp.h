/* 
 * File:   WorkerServiceImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 11:25
 */

#pragma once

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
		const std::string& serverBind,
		const std::string& servantAddress,
		const std::string& serverName,
		uint16_t serverId,
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

	virtual void SendToClient(const ::node::DataPacket& request,
		::rpcz::reply< ::node::DataPacket> response);

    virtual void CloseClient(const ::node::DataPacket& request,
        ::rpcz::reply< ::node::DataPacket> response);

    virtual void SendToWorker(const ::node::DataPacket& request,
        ::rpcz::reply< ::node::DataPacket> response);

	virtual void KickLogged(const ::node::DataPacket& request,
		::rpcz::reply< ::node::DataPacket> response);

protected:
	CreatePlayerMethod m_createPlayerMethod;
	ListInterestsMethod m_listProtoMethod;
	ListInterestsMethod m_listNotifMethod;
    std::string m_serverBind;
	std::string m_servantAddress;
	std::string m_serverName;
	uint16_t m_serverId;
};

