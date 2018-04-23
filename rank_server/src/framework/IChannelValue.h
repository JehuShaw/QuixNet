/* 
 * File:   IChannelValue.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#pragma once

#include <string>
#include "Common.h"
#include "ITPriorityQueue.h"
#include "rpc_channel.hpp"

class IChannelValue
{
public:
	virtual ~IChannelValue() {}
	virtual uint16_t GetServerId() const = 0;
	virtual const std::string& GetEndPoint() const = 0;
	virtual uint16_t GetServerType() const = 0;

	virtual uint32_t GetServerLoad() const { throw std::runtime_error("Method GetServerLoad() not implemented."); }
	virtual const std::string& GetAcceptAddress() const { throw std::runtime_error("Method GetAcceptAddress() not implemented."); }
	virtual const std::string& GetProcessPath() const { throw std::runtime_error("Method GetProcessPath() not implemented."); }
	virtual const std::string& GetProjectName() const { throw std::runtime_error("Method GetProjectName() not implemented."); }
	virtual uint16_t GetServerRegion() const { throw std::runtime_error("Method GetServerRegion() not implemented."); }

protected:
	virtual util::CAutoPointer<rpcz::rpc_channel> GetRpcChannel() const {
		throw std::runtime_error("Method util::CAutoPointer<rpcz::rpc_channel> GetRpcChannel() not implemented."); 
	}
	// worker use
	virtual util::CAutoPointer<util::IPQElementBase> GetUserPQItem() const { throw std::runtime_error("Method GetUserPQItem() not implemented."); }
	virtual util::CAutoPointer<util::IPQElementBase> GetServerPQItem() const { throw std::runtime_error("Method GetUserPQItem() not implemented."); }

	// servant use
	virtual void IterateServerIds(std::vector<uint16_t>& outServerIds) const {
		throw std::runtime_error("Method IterateServerIds(std::vector<uint16_t>& outServerIds) not implemented."); 
	}

protected:
	virtual void SetServerId(uint16_t nValue) = 0;
	virtual void SetEndPoint(const std::string& strValue) = 0;
	virtual void SetServerType(uint16_t nValue) = 0;

	friend class CNodeModule;
	friend class CNodeComparer;
	friend class CCacheModule;
	friend class CWorkerModule;
	friend class CWorkerComparer;
	friend class CMasterModule;
	friend class CServantModule;

	virtual void SetRpcChannel(util::CAutoPointer<rpcz::rpc_channel> channel) { 
		throw std::runtime_error("Method SetRpcChannel(util::CAutoPointer<rpcz::rpc_channel> channel) not implemented."); 
	}
	virtual void SetTimeoutCount(int32_t nValue) { throw std::runtime_error("Method SetTimeoutCount(long nValue) not implemented."); }
	virtual int32_t IncTimeoutCount() { throw std::runtime_error("Method IncTimeoutCount() not implemented."); }
	virtual void SetServerLoad(uint32_t nValue) { 
		throw std::runtime_error("Method SetServerLoad(unsigned long nValue) not implemented."); 
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
	virtual void AddServerId(uint16_t serverId) { throw std::runtime_error("Method AddServerId(uint16_t serverId) not implemented."); }
	virtual void SetServerRegion(uint16_t nValue) { throw std::runtime_error("Method SetServerRegion(uint16_t nValue) not implemented."); }
};

