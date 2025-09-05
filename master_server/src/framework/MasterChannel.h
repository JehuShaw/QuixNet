/* 
 * File:   MasterChannel.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_4_28, 13:15
 */

#ifndef MASTERCHANNEL_H
#define	MASTERCHANNEL_H

#include "IChannelValue.h"

class CMasterChannel : public IChannelValue {
public:
	CMasterChannel()
		: m_acceptAddress()
		, m_endPoint()
		, m_serverId(0)
		, m_serverType(0)
		, m_serverRegion(0)
		, m_processPath()
		, m_projectName()
	{}
public:
	virtual int32_t GetServerLoad() const { return 0; }
	virtual const std::string& GetAcceptAddress() const { return m_acceptAddress; }
	virtual const std::string& GetEndPoint() const { return m_endPoint; }
	virtual uint32_t GetServerId() const { return m_serverId; }
	virtual uint16_t GetServerType() const { return m_serverType; }
	virtual const std::string& GetProcessPath() const { return m_processPath; }
	virtual const std::string& GetProjectName() const { return m_projectName; }
	virtual uint16_t GetServerRegion() const { return m_serverRegion; }

protected:
	virtual void SetAcceptAddress(const std::string& strValue) { m_acceptAddress = strValue; }
	virtual void SetEndPoint(const std::string& strValue) { m_endPoint = strValue; }
	virtual void SetServerId(uint32_t nValue) { m_serverId = nValue; }
	virtual void SetServerType(uint16_t nValue) { m_serverType = nValue; }
	virtual void SetProcessPath(const std::string& strValue) { m_processPath = strValue; }
	virtual void SetProjectName(const std::string& strValue) { m_projectName = strValue; }
	virtual void SetServerRegion(uint16_t nValue) { m_serverRegion = nValue; }

private:
	std::string m_acceptAddress;
	std::string m_endPoint;
	uint32_t m_serverId;
	uint16_t m_serverType;
	uint16_t m_serverRegion;
	std::string m_processPath;
	std::string m_projectName;
};

#endif	/* MASTERCHANNEL_H */

