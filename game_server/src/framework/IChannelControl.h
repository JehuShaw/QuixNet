/* 
 * File:   IChannelControl.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef ICHANNELCONTROL_H
#define	ICHANNELCONTROL_H

#include <string>
#include <google/protobuf/repeated_field.h>
#include "ModuleManager.h"
#include "IChannelValue.h"

class IChannelControl : public mdl::CModule
{
public:
	typedef ::google::protobuf::RepeatedField< ::google::protobuf::uint64 > ROUTE_USERIDS_T;

	IChannelControl(const char* moduleName) : mdl::CModule(moduleName) {}

	IChannelControl(const std::string& moduleName) : mdl::CModule(moduleName) {}

	virtual ~IChannelControl() {}
	
	virtual bool CreatChannel(uint32_t serverId, const std::string& endPoint, uint16_t serverType,
		bool routeServer, uint64_t routeAddressId, const ROUTE_USERIDS_T& routeUserIds)
	{
		throw std::runtime_error("Method bool CreatChannel(uint32_t serverId, const std::string& endPoint, "
			"uint16_t serverType, bool routeServer, uint64_t routeAddressId, const ROUTE_USERIDS_T& routeUserIds) not implemented.");
	}

	virtual bool CreatChannel(uint32_t serverId, const std::string& endPoint, uint16_t serverType,
		const std::string& acceptAddress, const std::string& processPath,
		const std::string& projectName, uint16_t serverRegion)
	{
		throw std::runtime_error("Method bool CreatChannel(uint32_t serverId, const std::string& endPoint, "
			"uint16_t serverType, const std::string& acceptAddress, const std::string& processPath, "
			"const std::string& projectName, uint16_t serverRegion) not implemented.");
	}

	virtual bool RemoveChannel(uint32_t serverId) = 0;

	virtual int ChannelCount() const = 0;

	virtual void IterateChannel(std::vector<util::CAutoPointer<IChannelValue> >& outChannels) const = 0;

	virtual util::CAutoPointer<IChannelValue> GetLowLoadUserChnl() const
	{
		throw std::runtime_error("Method util::CAutoPointer<IChannelValue> GetLowLoadUserChnl() const  not implemented.");
	}

	virtual util::CAutoPointer<IChannelValue> GetChnlByDirServId(uint32_t serverId) const = 0;

};

#endif /* ICHANNELCONTROL_H */
