/* 
 * File:   IChannelControl.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#pragma once

#include <string>
#include "ModuleManager.h"
#include "IChannelValue.h"

class IChannelControl : public virtual mdl::CModule
{
public:
	virtual ~IChannelControl() {}
	
	virtual bool CreatChannel(uint16_t serverId, const std::string& endPoint, uint16_t serverType) = 0;

	virtual bool CreatChannel(uint16_t serverId, const std::string& endPoint, uint16_t serverType,
		const std::string& acceptAddress, const std::string& processPath, const std::string& projectName,
		uint16_t serverRegion) = 0;

	virtual bool RemoveChannel(uint16_t serverId) = 0;

	virtual int ChannelCount() const = 0;

	virtual void IterateChannel(std::vector<util::CAutoPointer<IChannelValue> >& outChannels) const = 0;

	virtual util::CAutoPointer<IChannelValue> GetLowLoadUserChnl() const = 0;

	virtual util::CAutoPointer<IChannelValue> GetChnlByDirServId(uint16_t serverId) const = 0;

};

