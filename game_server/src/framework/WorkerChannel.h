/* 
 * File:   WorkerChannel.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_4_28, 13:15
 */

#ifndef _WORKERCHANNEL_H
#define	_WORKERCHANNEL_H

#include "IChannelValue.h"

class CWorkPQItem : public util::IPQElementBase {
public:
	CWorkPQItem(uint16_t serverId) : m_count(0), m_index(-1), m_serverId(serverId) {
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(uint32_t) < sizeof(unsigned long));
#endif
	}

	virtual int GetIndex() const { return (int)m_index; }
	virtual void SetIndex(int nIdx) { atomic_xchg(&m_index, nIdx); }

	uint16_t GetServerId() const { return m_serverId; }
	uint32_t GetCount() const { return (uint32_t)m_count; }
	void SetCount(uint32_t nCount) { atomic_xchg(&m_count, nCount); }
	inline bool AtomicIncCount() throw() {
		uint32_t count;
		do {
			count = (uint32_t)m_count;
			if(count < 0) {
				return false;
			}
		} while (atomic_cmpxchg(&m_count,
			count + 1, count) != count);
		return true;
	}
	inline void AtomicDecCount() throw() {
		uint32_t count;
		do {
			count = (uint32_t)m_count;
			if(count == 0) {
				return;
			}
		} while (atomic_cmpxchg(&m_count, (count < 0 ?
			(count + 1) : (count - 1)), count) != count);
	}
	
private:
	volatile uint32_t m_count; 
	volatile int32_t m_index;
	uint16_t m_serverId;
};

class CWorkerComparer: public util::ITComparer {
public:
	virtual int Compare(const util::CAutoPointer<util::IPQElementBase>& aBase,
		const util::CAutoPointer<util::IPQElementBase>& bBase) const {

			util::CAutoPointer<CWorkPQItem> a(aBase);
			util::CAutoPointer<CWorkPQItem> b(bBase);
			if(a->GetCount() > b->GetCount()) {
				return 1;
			} else if(a->GetCount() == b->GetCount()) {
				return 0;
			} else {
				return -1;
			}
	}
};

class CWorkerChannel0 : public IChannelValue {
public:
	CWorkerChannel0(uint16_t serverId)
		: m_timeoutCount(0)
		, m_endPoint()
		, m_serverId(serverId)
		, m_serverType(0)
		, m_channel()
		, m_userPQItem(new CWorkPQItem(serverId))
		, m_serverPQItem(new CWorkPQItem(serverId))
	{
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(int32_t) < sizeof(long));
#endif
	}
public:
	virtual const std::string& GetEndPoint() const { return m_endPoint; }
	virtual uint16_t GetServerId() const { return m_serverId; }
	virtual uint32_t GetServerLoad() const { if(!m_userPQItem.IsInvalid()) { return m_userPQItem->GetCount(); } return 0; }
	virtual uint16_t GetServerType() const { return m_serverType; }

	virtual const std::string& GetAcceptAddress() const { return GetStrNull(); }
	virtual const std::string& GetProcessPath() const { return GetStrNull(); }
	virtual const std::string& GetProjectName() const { return GetStrNull(); }

protected:
	
	virtual util::CAutoPointer<rpcz::rpc_channel> GetRpcChannel() const { return m_channel; }
	virtual util::CAutoPointer<util::IPQElementBase> GetUserPQItem() const { return m_userPQItem; }
	virtual util::CAutoPointer<util::IPQElementBase> GetServerPQItem() const { return m_serverPQItem; }

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
	util::CAutoPointer<CWorkPQItem> m_userPQItem;
	util::CAutoPointer<CWorkPQItem> m_serverPQItem;
	
};

class CWorkerChannel1: public CWorkerChannel0 {
public: 
	CWorkerChannel1(uint16_t serverId)
		: CWorkerChannel0(serverId)
		, m_acceptAddress()
		, m_processPath()
		, m_projectName()
		, m_serverRegion(0)
	{}

public:
	virtual const std::string& GetAcceptAddress() const { return m_acceptAddress; }
	virtual const std::string& GetProcessPath() const { return m_processPath; }
	virtual const std::string& GetProjectName() const { return m_projectName; }
	virtual uint16_t GetServerRegion()const { return m_serverRegion; }

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

#endif	/* _WORKERCHANNEL_H */

