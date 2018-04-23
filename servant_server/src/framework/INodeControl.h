/* 
 * File:   INodeControl.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#pragma once

#include "IChannelControl.h"

class INodeControl: public virtual IChannelControl
{
public:
	virtual ~INodeControl() {}

	virtual void UpdateChannelLoad(uint16_t serverId, uint32_t serverLoad) = 0;

	virtual util::CAutoPointer<IChannelValue> GetLowLoadByRegion(uint16_t serverRegion) const = 0;
};

