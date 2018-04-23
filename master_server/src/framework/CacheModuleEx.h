/*
 * File:   CacheModuleEx.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#pragma once

#include "CacheModule.h"


class CCacheModuleEx : public CCacheModule
{
public:
	CCacheModuleEx(void);

	CCacheModuleEx(const std::string& moduleName,
		const std::string& endPoint,
		uint16_t serverId);

	CCacheModuleEx(const std::string& moduleName,
		const std::string& endPoint,
		uint16_t serverId,
		const std::string& acceptAddress,
		const std::string& processPath,
		const std::string& projectName,
		uint16_t serverRegion);

protected:
	virtual bool FilterProtocolInterest(int nProtocal);
};

