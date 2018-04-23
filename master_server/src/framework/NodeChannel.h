/* 
 * File:   NodeChannel.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_4_28, 13:15
 */

#ifndef _NODECHANNEL_H
#define	_NODECHANNEL_H

#include "IChannelValue.h"


class CNodeChannel : public IChannelValue, public util::IPQElementBase {
public:
	CNodeChannel() :
	  m_timeoutCount(0),
	  m_acceptAddress(),
	  m_endPoint(),
	  m_serverId(0),
	  m_serverType(0),
	  m_processPath(),
	  m_projectName(),
	  m_serverRegion(0),
	  m_channel()
	{
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(int32_t) < sizeof(long));
#endif
		m_servPQItem.SetRawPointer(this, false);
	}
	virtual uint32_t GetServerLoad() const { return (uint32_t)m_serverLoad; }
	virtual const std::string& GetAcceptAddress() const { return m_acceptAddress; }
	virtual const std::string& GetEndPoint() const { return m_endPoint; }
	virtual uint16_t GetServerId() const { return m_serverId; }
	virtual uint16_t GetServerType() const { return m_serverType; }
	virtual const std::string& GetProcessPath() const { return m_processPath; }
	virtual const std::string& GetProjectName() const { return m_projectName; }
	virtual uint16_t GetServerRegion() const { return m_serverRegion; }

protected:
	virtual int GetIndex() const { return (int)m_index; }
	virtual util::CAutoPointer<rpcz::rpc_channel> GetRpcChannel() const { return m_channel; }
	virtual util::CAutoPointer<util::IPQElementBase> GetServerPQItem() const {
		return m_servPQItem; 
	}

protected:
	virtual void SetIndex(int nIdx) { atomic_xchg(&m_index, nIdx); }
	virtual void SetServerLoad(uint32_t nValue) { atomic_xchg(&m_serverLoad, nValue); }
	virtual void SetAcceptAddress(const std::string& strValue) { m_acceptAddress = strValue; }
	virtual void SetEndPoint(const std::string& strValue) { m_endPoint = strValue; }
	virtual void SetServerId(uint16_t nValue) { m_serverId = nValue; }
	virtual void SetServerType(uint16_t nValue) { m_serverType = nValue; }
	virtual void SetProcessPath(const std::string& strValue) { m_processPath = strValue; }
	virtual void SetProjectName(const std::string& strValue) { m_projectName = strValue; }
	virtual void SetServerRegion(uint16_t nValue) { m_serverRegion = nValue; }
	virtual void SetRpcChannel(util::CAutoPointer<rpcz::rpc_channel> channel) { m_channel = channel; }
	virtual void SetTimeoutCount(int32_t nValue) { atomic_xchg(&m_timeoutCount, nValue); }
	virtual int32_t IncTimeoutCount() { return atomic_inc(&m_timeoutCount); }

private:
	volatile int32_t m_index;
	volatile uint32_t m_serverLoad;
	volatile int32_t m_timeoutCount;
	std::string m_acceptAddress;
	std::string m_endPoint;
	uint16_t m_serverId;
	uint16_t m_serverType;
	std::string m_processPath;
	std::string m_projectName;
	uint16_t m_serverRegion;
	util::CAutoPointer<rpcz::rpc_channel> m_channel;
	util::CAutoPointer<CNodeChannel> m_servPQItem;
};

class CNodeComparer: public util::ITComparer {
public:
	virtual int Compare(const util::CAutoPointer<util::IPQElementBase>& aBase,
		const util::CAutoPointer<util::IPQElementBase>& bBase) const {

			util::CAutoPointer<CNodeChannel> a(aBase);
			util::CAutoPointer<CNodeChannel> b(bBase);
			if(a->GetServerLoad() > b->GetServerLoad()) {
				return 1;
			} else if(a->GetServerLoad() == b->GetServerLoad()) {
				return 0;
			} else {
				return -1;
			}
	}
};

class CServantChannel : public CNodeChannel {
public:
	CServantChannel()
		: CNodeChannel()
		, m_rwLock()
		, m_serverIds()
	{}
public:
	virtual void AddServerId(uint16_t serverId) {
		thd::CScopedWriteLock wrLock(m_rwLock);
		m_serverIds.insert(serverId);
	}

protected:
	virtual void IterateServerIds(std::vector<uint16_t>& outServerIds) const {
		thd::CScopedReadLock rdLock(m_rwLock);
		if(m_serverIds.empty()) {
			return;
		}
		std::set<uint16_t>::const_iterator it(m_serverIds.begin());
		for(; m_serverIds.end() != it; ++it) {
			outServerIds.push_back(*it);
		}
	}
private:
	thd::CSpinRWLock m_rwLock;
	std::set<uint16_t> m_serverIds;
};

#endif	/* _NODECHANNEL_H */

