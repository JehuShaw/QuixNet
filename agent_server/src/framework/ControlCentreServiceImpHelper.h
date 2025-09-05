/*
 * File:   ControlCentreServiceImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef CONTROLCENTRESERVICEIMPHELPER_H
#define	CONTROLCENTRESERVICEIMPHELPER_H

#include "AgentWorkerModule.h"
//#include "CacheModule.h"

static mdl::IModule* CreateWorkerModle(
	const std::string& moduleName,
	const std::string& endPoint,
	uint32_t serverId,
	bool routeServer,
	uint64_t routeAddressId,
	const IChannelControl::ROUTE_USERIDS_T& routeUserIds) {

	return new CAgentWorkerModule(moduleName, endPoint, serverId, routeServer, routeAddressId, routeUserIds);
}

//static mdl::IModule* CreateCacheModle(
//	const std::string& moduleName,
//	const std::string& endPoint,
//	uint32_t serverId,
//  bool routeServer,
//  uint64_t routeAddressId,
//  const IChannelControl::ROUTE_USERIDS_T& routeUserIds) {
//	return new CCacheModule(moduleName, endPoint, serverId, routeServer, routeAddressId, routeUserIds);
//}

#endif /* CONTROLCENTRESERVICEIMPHELPER_H */