/* 
 * File:   INodeControl.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef INODECONTROL_H
#define	INODECONTROL_H

#include "IChannelControl.h"

class INodeControl: public IChannelControl
{
public:
	INodeControl(const char* moduleName) : IChannelControl(moduleName) {}
	INodeControl(const std::string& moduleName) : IChannelControl(moduleName) {}
	virtual ~INodeControl() {}

	virtual void UpdateChannelLoad(uint32_t serverId, int32_t serverLoad) = 0;

	virtual util::CAutoPointer<IChannelValue> GetLowLoadByRegion(uint16_t serverRegion) const = 0;
};

#endif /* INODECONTROL_H */
