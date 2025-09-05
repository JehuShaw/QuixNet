/* 
 * File:   CacheServiceImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef CACHESERVICEIMP_H
#define	CACHESERVICEIMP_H

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
	
	static uint16_t GetDBIDFromDBByUserID(uint64_t userId);

public:
	CCacheServiceImp(
		const std::string& serverBind,
		const std::string& servantAddress,
		const std::string& serverName,
		const std::string& strAgentName,
		uint32_t serverId,
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
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleLoad(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleStore(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleGet(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleSet(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleGets(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleCAS(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleDel(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleLoadAll(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);


	void HandleDBInsert(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleDBSelect(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleDBUpdate(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleDBDelete(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleDBSelectAll(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleDBEscapeString(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleDBStoredProcedures(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleDBAsyncStoredProcedures(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleDBCheckGlobalExists(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleDBCheckEscapeString(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleSendToClient(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleBroadcastToClient(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleCloseClient(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleCloseAllClients(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleSendToWorker(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleKickLogged(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleSendToPlayer(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandlePostToPlayer(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandlePlay(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleStop(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

	void HandleAllDBStoredProcedures(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);
	
	void HandleDefault(
		const ::node::DataPacket& request,
		::rpcz::reply<::node::DataPacket>& response);

protected:
	static uint16_t GetDBID(int32_t nRouteType, uint64_t n64Route);

protected:
	CreatePlayerMethod m_createPlayerMethod;
	const std::string m_endPoint;
	const std::string m_servantAddress;
	const std::string m_serverName;
	const std::string m_agentName;
	const uint32_t m_serverId;
};

#endif /* CACHESERVICEIMP_H */