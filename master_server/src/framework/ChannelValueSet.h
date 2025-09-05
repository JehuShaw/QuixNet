/* 
 * File:   ChannelValueSet.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef CHANNELVALUESET_H
#define CHANNELVALUESET_H

#include "IChannelValue.h"

class CFindChannelValue : public IChannelValue {
public:
	CFindChannelValue(uint32_t serverId) : m_serverId(serverId) {}
	virtual uint32_t GetServerId() const { return m_serverId; }
	virtual const std::string& GetEndPoint() const { throw std::runtime_error("Method const std::string& GetEndPoint() not implemented."); }
	virtual uint16_t GetServerType() const { throw std::runtime_error("Method uint16_t GetServerType() not implemented."); }

protected:
	virtual void SetEndPoint(const std::string& strValue) { throw std::runtime_error("Method SetEndPoint(const std::string& strValue) not implemented."); }
	virtual void SetServerId(uint32_t nValue) { throw std::runtime_error("Method SetServerId(uint32_t nValue) not implemented."); }
	virtual void SetServerType(uint16_t nValue) { throw std::runtime_error("Method SetServerType(uint16_t nValue) not implemented."); }

private:
	uint32_t m_serverId;
};

struct IChannelValueCompare
{
	bool operator()(const CAutoPointer<IChannelValue>& pObject1
		, const CAutoPointer<IChannelValue>& pObject2)const
	{
		bool bInValid1 = pObject1.IsInvalid();
		bool bInValid2 = pObject2.IsInvalid();
		if(!bInValid1 && !bInValid2) {
			return pObject1->GetServerId() < pObject2->GetServerId();
		} else if(bInValid1 && !bInValid2) {
			return true;
		}
		return false;
	}
};

typedef std::set<CAutoPointer<IChannelValue>, struct IChannelValueCompare > CHANNEL_VALUE_SET_T;

#endif /* CHANNELVALUESET_H */
