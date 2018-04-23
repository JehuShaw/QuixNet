/* 
 * File:   CacheChannel.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_4_28, 13:15
 */

#ifndef _CACHECHANNEL_H
#define	_CACHECHANNEL_H

#include "IChannelValue.h"

class CCacheChannel0 : public IChannelValue {
public:
	CCacheChannel0()
		: m_timeoutCount(0)
		, m_endPoint()
		, m_serverId(0)
		, m_serverType(0)
		, m_channel()
	{
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(int32_t) < sizeof(long));
#endif
	}
public:
	virtual uint16_t GetServerId() const { return m_serverId; }
	virtual const std::string& GetEndPoint() const { return m_endPoint; }
	virtual uint32_t GetServerLoad() const { return 0; }
	virtual uint16_t GetServerType() const { return m_serverType; }

	virtual const std::string& GetAcceptAddress() const { return GetStrNull(); }
	virtual const std::string& GetProcessPath() const { return GetStrNull(); }
	virtual const std::string& GetProjectName() const { return GetStrNull(); }

protected:
	virtual util::CAutoPointer<rpcz::rpc_channel> GetRpcChannel() const { return m_channel; }

protected:
	virtual void SetEndPoint(const std::string& strValue) { m_endPoint = strValue; }
	virtual void SetServerId(uint16_t nValue) { m_serverId = nValue; }
	virtual void SetServerType(uint16_t nValue) { m_serverType = nValue; }
	virtual void SetRpcChannel(util::CAutoPointer<rpcz::rpc_channel> channel) { m_channel = channel; }
	virtual void SetTimeoutCount(int32_t nValue) { atomic_xchg(&m_timeoutCount, nValue); }
	virtual int32_t IncTimeoutCount() { return atomic_inc(&m_timeoutCount); }

protected:
	static const std::string& GetStrNull() {
		static const std::string strNull;
		return strNull;
	}

private:
	volatile int32_t m_timeoutCount;
	std::string m_endPoint;
	uint16_t m_serverId;
	uint16_t m_serverType;
	util::CAutoPointer<rpcz::rpc_channel> m_channel;
};

class CCacheChannel1 : public CCacheChannel0 {
public:
	CCacheChannel1()
		: CCacheChannel0()
		, m_acceptAddress()
		, m_processPath()
		, m_projectName()
		, m_serverRegion(0)
	{}
public:
	virtual const std::string& GetAcceptAddress() const { return m_acceptAddress; }
	virtual const std::string& GetProcessPath() const { return m_processPath; }
	virtual const std::string& GetProjectName() const { return m_projectName; }
	virtual uint16_t GetServerRegion() const { return m_serverRegion; }

protected:
	virtual void SetAcceptAddress(const std::string& strValue) { m_acceptAddress = strValue; }
	virtual void SetProcessPath(const std::string& strValue) { m_processPath = strValue; }
	virtual void SetProjectName(const std::string& strValue) { m_projectName = strValue; }
	virtual void SetServerRegion(uint16_t nValue) { m_serverRegion = nValue; }

private:
	std::string m_acceptAddress;
	std::string m_processPath;
	std::string m_projectName;
	uint16_t m_serverRegion;
};

#endif	/* _CACHECHANNEL_H */

