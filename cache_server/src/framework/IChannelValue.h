/* 
 * File:   IChannelValue.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef ICHANNELVALUE_H
#define	ICHANNELVALUE_H

#include <string>
#include "Common.h"
#include "ITPriorityQueue.h"
#include "rpc_channel.hpp"

class IChannelValue
{
public:
	virtual ~IChannelValue() {}
	virtual uint32_t GetServerId() const = 0;
	virtual const std::string& GetEndPoint() const = 0;
	virtual uint16_t GetServerType() const = 0;
	
	virtual uint64_t GetRouteAddressId() const { throw std::runtime_error("Method GetRouteAddressId() not implemented."); }

	virtual int32_t GetServerLoad() const { throw std::runtime_error("Method GetServerLoad() not implemented."); }
	virtual const std::string& GetAcceptAddress() const { throw std::runtime_error("Method GetAcceptAddress() not implemented."); }
	virtual const std::string& GetProcessPath() const { throw std::runtime_error("Method GetProcessPath() not implemented."); }
	virtual const std::string& GetProjectName() const { throw std::runtime_error("Method GetProjectName() not implemented."); }
	virtual uint16_t GetServerRegion() const { throw std::runtime_error("Method GetServerRegion() not implemented."); }

protected:
	virtual util::CAutoPointer<rpcz::rpc_channel> GetRpcChannel() const {
		throw std::runtime_error("Method util::CAutoPointer<rpcz::rpc_channel> GetRpcChannel() not implemented."); 
	}

	// servant use
	virtual void IterateServerIds(std::vector<uint32_t>& outServerIds) const {
		throw std::runtime_error("Method IterateServerIds(std::vector<uint32_t>& outServerIds) not implemented."); 
	}

protected:
	virtual void SetServerId(uint32_t nValue) = 0;
	virtual void SetEndPoint(const std::string& strValue) = 0;
	virtual void SetServerType(uint16_t nValue) = 0;

	friend class CNodeModule;
	friend class CNodeComparer;
	friend class CCacheModule;
	friend class CWorkerModule;
	friend class CAgentWorkerModule;
	friend class CWorkerComparer;
	friend class CMasterModule;
	friend class CServantModule;

	virtual void SetRouteAddressId(uint64_t nValue) { throw std::runtime_error("Method SetRouteAddressId(uint64_t nValue) not implemented."); }

	virtual void SetRpcChannel(util::CAutoPointer<rpcz::rpc_channel> channel) { 
		throw std::runtime_error("Method SetRpcChannel(util::CAutoPointer<rpcz::rpc_channel> channel) not implemented."); 
	}
	virtual void SetTimeoutCount(int32_t nValue) { throw std::runtime_error("Method SetTimeoutCount(long nValue) not implemented."); }
	virtual int32_t IncTimeoutCount() { throw std::runtime_error("Method IncTimeoutCount() not implemented."); }
	virtual int32_t XchgServerLoad(int32_t nValue) {
		throw std::runtime_error("Method XchgServerLoad(int32_t nValue) not implemented."); 
	}
	virtual void IncServerLoad() {
		throw std::runtime_error("Method IncServerLoad() not implemented.");
	}
	virtual void DecServerLoad() {
		throw std::runtime_error("Method DecServerLoad() not implemented.");
	}
	virtual void SetAcceptAddress(const std::string& strValue) { 
		throw std::runtime_error("Method SetAcceptAddress(const std::string& strValue) not implemented."); 
	}
	virtual void SetProcessPath(const std::string& strValue) { 
		throw std::runtime_error("Method SetProcessPath(const std::string& strValue) not implemented."); 
	}
	virtual void SetProjectName(const std::string& strValue) { 
		throw std::runtime_error("Method SetProjectName(const std::string& strValue) not implemented."); 
	}

	// servant use
	virtual void AddServerId(uint32_t serverId) { throw std::runtime_error("Method AddServerId(uint32_t serverId) not implemented."); }
	virtual void SetServerRegion(uint16_t nValue) { throw std::runtime_error("Method SetServerRegion(uint16_t nValue) not implemented."); }
};

#endif /* ICHANNELVALUE_H */