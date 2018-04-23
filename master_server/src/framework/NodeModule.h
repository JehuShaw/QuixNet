/* 
 * File:   NodeModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_4_28, 13:15
 */

#ifndef _NODEMODULE_H
#define	_NODEMODULE_H

#include "NodeDefines.h"

#include "TPriorityQueue.h"
#include "worker.rpcz.h"
#include "INodeControl.h"
#include "IChannelValue.h"


class CNodeModule : public INodeControl
{
public:
	CNodeModule(void);

	CNodeModule(const std::string& moduleName,
		const std::string& endPoint,
		uint16_t serverId,
		uint16_t serverType,
		const std::string& acceptAddress,
		const std::string& processPath,
		const std::string& projectName,
		uint16_t serverRegion);

	~CNodeModule(void);

	virtual void OnRegister();

	virtual void OnRemove();

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual void HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	virtual bool CreatChannel(uint16_t serverId, const std::string& endPoint, uint16_t serverType);

	virtual bool CreatChannel(uint16_t serverId, const std::string& endPoint, uint16_t serverType,
		const std::string& acceptAddress, const std::string& processPath, const std::string& projectName,
		uint16_t serverRegion);

	virtual bool RemoveChannel(uint16_t serverId);

	virtual int ChannelCount() const ;

	virtual void IterateChannel(std::vector<util::CAutoPointer<IChannelValue> >& outChannels) const;

	virtual util::CAutoPointer<IChannelValue> GetLowLoadUserChnl() const;

	virtual util::CAutoPointer<IChannelValue> GetLowLoadByRegion(uint16_t serverRegion) const;

	util::CAutoPointer<IChannelValue> GetChnlByDirServId(uint16_t serverId) const;

	virtual void UpdateChannelLoad(uint16_t serverId, uint32_t serverLoad);

	int SendNotification(uint16_t serverId, int32_t cmd);

	int SendNotification(uint16_t serverId, int32_t cmd, const ::google::protobuf::Message& message);

	int SendNotification(uint16_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& message);

	int SendNotification(uint16_t serverId, int32_t cmd, const std::string& data);

	int SendNotification(uint16_t serverId, int32_t cmd, uint64_t userId, const std::string& data);

	int SendNotification(util::CAutoPointer<IChannelValue>& rpcChannel, const ::node::DataPacket& request);

	static util::CAutoPointer<IChannelValue> GetStaticChannel(uint16_t serverId) {
		thd::CScopedReadLock rdLock(s_rwTicket);
		SERVERID_TO_CHANNEL_T::const_iterator it(s_serIdChannels.find(serverId));
		if(s_serIdChannels.end() == it) {
			return util::CAutoPointer<IChannelValue>();
		}
		return it->second;
	}

	static void IterateStaticChannel(std::vector<util::CAutoPointer<IChannelValue> >& outChannels) {
		thd::CScopedReadLock rdLock(s_rwTicket);
		SERVERID_TO_CHANNEL_T::const_iterator it(s_serIdChannels.begin());
		for(; s_serIdChannels.end() != it; ++it) {
			const int nServerType = it->second->GetServerType();
			if(REGISTER_TYPE_NODE != nServerType 
				&& REGISTER_TYPE_SERVANT != nServerType)
			{
				continue;
			}
			outChannels.push_back(it->second);
		}
	}

	static int SendNodeMessage(uint16_t serverId, int32_t cmd);

	static int SendNodeMessage(uint16_t serverId, int32_t cmd, const ::google::protobuf::Message& message);

	static int SendNodeMessage(uint16_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& message);

	static int SendNodeMessage(uint16_t serverId, int32_t cmd, const std::string& data);

	static int SendNodeMessage(uint16_t serverId, int32_t cmd, uint64_t userId, const std::string& data);

	static int SendNodeMessage(util::CAutoPointer<IChannelValue>& rpcChannel, const ::node::DataPacket& request);

	static int SendNodeMessage(util::CAutoPointer<IChannelValue>& rpcChannel, const ::node::DataPacket& request, ::node::DataPacket& response);

	static void BroadcastAllNodes(int32_t cmd);

public:
	static int AddServerId(uint16_t servantId, uint16_t serverId);

	static void RemoveStaticChannel(uint16_t serverId) {
		thd::CScopedWriteLock wrLock(s_rwTicket);
		s_serIdChannels.erase(serverId);
	}

private:
	bool InsertSerIdChannel(
		uint16_t serverId,
		uint16_t serverType,
		const std::string& endPoint,
		util::CAutoPointer<IChannelValue>& outRpcChannel);

	bool InsertSerIdChannel(
		uint16_t serverId,
		uint16_t serverType,
		const std::string& endPoint,
		const std::string& acceptAddress,
		const std::string& processPath,
		const std::string& projectName,
		uint16_t serverRegion,
		util::CAutoPointer<IChannelValue>& outRpcChannel);

	bool EraseSerIdChannel(uint16_t serverId);

	static void AddStaticChannel(uint16_t serverId, const util::CAutoPointer<IChannelValue>& pChannel) {
		thd::CScopedWriteLock wrLock(s_rwTicket);
		s_serIdChannels[serverId] = pChannel;
	}

	static void ServantRemoveStaticChannel(uint16_t serverId);

private:
	struct NodeSetCompare {
		bool operator()(const util::CAutoPointer<IChannelValue>& pChannel1
			, const util::CAutoPointer<IChannelValue>& pChannel2)const
		{
			uint16_t n16Region1 = pChannel1->GetServerRegion();
			uint16_t n16Region2 = pChannel2->GetServerRegion();
			if(n16Region1 < n16Region2) {
				return true;
			} else if(n16Region1 == n16Region2) {
				uint32_t nLoad1 = pChannel1->GetServerLoad();
				uint32_t nLoad2 = pChannel2->GetServerLoad();
				if(nLoad1 < nLoad2) {
					return true;
				} else if(nLoad1 == nLoad2) {
					return pChannel1->GetServerId() < pChannel2->GetServerId();
				}
			}
			return false;
		}
	};

private:
	util::CTPriorityQueue m_lessQueue;
	thd::CSpinRWLock m_rwQueue;

	typedef std::set<util::CAutoPointer<IChannelValue>, struct NodeSetCompare> SORT_REGION_T;
	SORT_REGION_T m_sortRegion;
	thd::CSpinRWLock m_rwRegion;

	typedef std::map<uint16_t, util::CAutoPointer<IChannelValue> > SERVERID_TO_CHANNEL_T;
	SERVERID_TO_CHANNEL_T m_serIdChannels;
	thd::CSpinRWLock m_rwServer;

	static thd::CSpinRWLock s_rwTicket;
	static SERVERID_TO_CHANNEL_T s_serIdChannels;
};


#endif	/* _NODEMODULE_H */

