/*
 * File:   CacheModuleEx.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef CACHEMODULEEX_H
#define	CACHEMODULEEX_H

#include "CacheModule.h"


class CCacheModuleEx : public CCacheModule
{
public:
	CCacheModuleEx(const std::string& moduleName,
		const std::string& endPoint,
		uint32_t serverId,
		bool routeServer,
		uint64_t routeAddressId,
		const ROUTE_USERIDS_T& routeUserIds);

protected:
	virtual bool FilterProtocolInterest(int nProtocal);
};

#endif /* CACHEMODULEEX_H */
