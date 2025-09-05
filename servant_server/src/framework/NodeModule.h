/* 
 * File:   NodeModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_4_28, 13:15
 */

#ifndef NODEMODULE_H
#define	NODEMODULE_H

#include "NodeDefines.h"

#include "TPriorityQueue.h"
#include "worker.rpcz.h"
#include "INodeControl.h"
#include "IChannelValue.h"


class CNodeModule : public virtual INodeControl
{
public:
	CNodeModule(const std::string& moduleName,
		const std::string& endPoint,
		uint32_t serverId,
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

	virtual bool CreatChannel(uint32_t serverId, const std::string& endPoint, uint16_t serverType,
		const std::string& acceptAddress, const std::string& processPath, const std::string& projectName,
		uint16_t serverRegion);

	virtual bool RemoveChannel(uint32_t serverId);

	virtual int ChannelCount() const ;

	virtual void IterateChannel(std::vector<util::CAutoPointer<IChannelValue> >& outChannels) const;

	virtual util::CAutoPointer<IChannelValue> GetLowLoadUserChnl() const;

	virtual util::CAutoPointer<IChannelValue> GetLowLoadByRegion(uint16_t serverRegion) const;

	util::CAutoPointer<IChannelValue> GetChnlByDirServId(uint32_t serverId) const;

	virtual void UpdateChannelLoad(uint32_t serverId, int32_t serverLoad);

	int SendNotification(uint32_t serverId, int32_t cmd);

	int SendNotification(uint32_t serverId, int32_t cmd, const ::google::protobuf::Message& request);

	int SendNotification(uint32_t serverId, int32_t cmd, const ::google::protobuf::Message& request, ::google::protobuf::Message& response);

	int SendNotification(uint32_t serverId, int32_t cmd, const std::string& request, std::string& response);

	int SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId);

	int SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request);

	int SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request, ::google::protobuf::Message& response);

	int SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, const std::string& request, std::string& response);

	int SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, int32_t subcmd);

	int SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& message, int32_t subcmd);

	int SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request, ::google::protobuf::Message& response, int32_t subcmd);

	int SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, const std::string& request, std::string& response, int32_t subcmd);

	int SendNotification(util::CAutoPointer<IChannelValue>& rpcChannel, const ::node::DataPacket& request);

	int SendNotification(util::CAutoPointer<IChannelValue>& rpcChannel, const ::node::DataPacket& request, ::node::DataPacket& response);

	static util::CAutoPointer<IChannelValue> GetStaticChannel(uint32_t serverId) {
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

	static int SendNodeMessage(uint32_t serverId, int32_t cmd);

	static int SendNodeMessage(uint32_t serverId, int32_t cmd, const ::google::protobuf::Message& request);

	static int SendNodeMessage(uint32_t serverId, int32_t cmd, const ::google::protobuf::Message& request, ::google::protobuf::Message& response);

	static int SendNodeMessage(uint32_t serverId, int32_t cmd, const std::string& request, std::string& response);

	static int SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId);

	static int SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request);

	static int SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request, ::google::protobuf::Message& response);

	static int SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, const std::string& request, std::string& response);

	static int SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, int32_t subcmd);

	static int SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request, int32_t subcmd);

	static int SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request, ::google::protobuf::Message& response, int32_t subcmd);

	static int SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, const std::string& request, std::string& response, int32_t subcmd);

	static int SendNodeMessage(util::CAutoPointer<IChannelValue>& rpcChannel, const ::node::DataPacket& request);

	static int SendNodeMessage(util::CAutoPointer<IChannelValue>& rpcChannel, const ::node::DataPacket& request, ::node::DataPacket& response);

	static void BroadcastAllNodes(int32_t cmd);

public:
	static int AddServerId(uint32_t servantId, uint32_t serverId);

	static void RemoveStaticChannel(uint32_t serverId) {
		thd::CScopedWriteLock wrLock(s_rwTicket);
		s_serIdChannels.erase(serverId);
	}

private:
	bool InsertSerIdChannel(
		uint32_t serverId,
		uint16_t serverType,
		const std::string& endPoint,
		const std::string& acceptAddress,
		const std::string& processPath,
		const std::string& projectName,
		uint16_t serverRegion,
		util::CAutoPointer<IChannelValue>& outRpcChannel);

	bool EraseSerIdChannel(uint32_t serverId);

	static void AddStaticChannel(uint32_t serverId, const util::CAutoPointer<IChannelValue>& pChannel) {
		thd::CScopedWriteLock wrLock(s_rwTicket);
		s_serIdChannels[serverId] = pChannel;
	}

	static void ServantRemoveStaticChannel(uint32_t serverId);

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
				int32_t nLoad1 = pChannel1->GetServerLoad();
				int32_t nLoad2 = pChannel2->GetServerLoad();
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

	typedef std::map<uint32_t, util::CAutoPointer<IChannelValue> > SERVERID_TO_CHANNEL_T;
	SERVERID_TO_CHANNEL_T m_serIdChannels;
	thd::CSpinRWLock m_rwServer;

	static thd::CSpinRWLock s_rwTicket;
	static SERVERID_TO_CHANNEL_T s_serIdChannels;
};


#endif	/* NODEMODULE_H */

