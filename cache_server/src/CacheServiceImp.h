/* 
 * File:   CacheServiceImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#pragma once

#include <map>
#include "cache.rpcz.h"
#include "AutoPointer.h"
#include "SpinRWLock.h"
#include "Common.h"

class CPlayerBase;

class CCacheServiceImp: public ::node::CacheService
{
public:
	typedef util::CAutoPointer<CPlayerBase> (*CreatePlayerMethod)(uint64_t userId);

public:
	CCacheServiceImp(
		const std::string& serverBind,
		const std::string& servantAddress,
		const std::string& serverName,
		uint16_t serverId,
		CreatePlayerMethod createPlayerMethod = NULL);

	~CCacheServiceImp(void);

	virtual void ListNotificationInterests(const ::node::VoidPacket& request,
		::rpcz::reply< ::node::InterestPacket> response);

	virtual void ListProtocolInterests(const ::node::VoidPacket& request,
		::rpcz::reply< ::node::InterestPacket> response);

	virtual void HandleNotification(const ::node::DataPacket& request,
		::rpcz::reply< ::node::DataPacket> response);

	virtual void HandleProtocol(const ::node::DataPacket& request,
		::rpcz::reply< ::node::DataPacket> response);

protected:
	void HandleAdd(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleLoad(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleStore(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleGet(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleSet(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleGets(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleCAS(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleDel(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleLoadAll(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);


	void HandleDBInsert(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleDBSelect(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleDBUpdate(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleDBDelete(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleDBSelectAll(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleDBEscapeString(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleDBStoredProcedures(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleDBAsyncStoredProcedures(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleSendToClient(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleBroadcastToClient(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleCloseClient(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleCloseAllClient(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleSendToWorker(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleKickLogged(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandlePlay(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

	void HandleStop(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);
	
	void HandleDefault(
		const ::node::DataPacket& inRequest,
		::node::DataPacket& outResponse);

protected:
	static uint16_t GetDBID(int32_t nRouteType, uint64_t n64Route);

protected:
	CreatePlayerMethod m_createPlayerMethod;
	std::string m_serverBind;
	std::string m_servantAddress;
	std::string m_serverName;
	uint16_t m_serverId;
};

