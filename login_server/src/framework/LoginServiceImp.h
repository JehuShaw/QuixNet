/* 
 * File:   LoginServiceImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 11:25
 */

#pragma once

#include "WorkerServiceImp.h"

class CLoginServiceImp : public CWorkerServiceImp
{
public:
	CLoginServiceImp(
		const std::string& strServerBind,
		const std::string& servantAddress,
		const std::string& serverName,
		uint16_t serverId,
		CreatePlayerMethod createPlayerMethod = NULL, 
		ListInterestsMethod listProtoMethod = NULL,
		ListInterestsMethod listNotifMethod = NULL);

	~CLoginServiceImp(void);

	virtual void SendToClient(const ::node::DataPacket& request,
		::rpcz::reply< ::node::DataPacket> response);
};

