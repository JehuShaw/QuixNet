#include "CacheModule.h"
#include "SpinRWLock.h"
#include "worker.rpcz.h"
#include "ChannelManager.h"
#include "rpc_controller.hpp"
#include "Log.h"
#include "BodyMessage.h"
#include "NodeDefines.h"
#include "CacheChannel.h"
#include "CacheOperateHelper.h"

using namespace mdl;
using namespace rpcz;
using namespace util;
using namespace thd;

// jump consistent hash. John Lamping, Eric Veach Google
static int32_t JumpConsistentHash(uint64_t key, int32_t num_buckets) {
	int64_t b = -1, j = 0;
	while(j < num_buckets) {
		b = j;
		key = key * 2862933555777941757ULL + 1;
		j = (int64_t)((b + 1) * (double(1LL << 31) / double((key >> 33) + 1)));
	}
	return (int32_t)b;
}

CCacheModule::CCacheModule(void)
	: CModule()
{
}

CCacheModule::CCacheModule(
	const std::string& moduleName,
	const std::string& endPoint,
	uint16_t serverId)

	: CModule(moduleName)
{
	CreatChannel(serverId, endPoint, (uint16_t)REGISTER_TYPE_CACHE);
}

CCacheModule::CCacheModule(
	const std::string& moduleName,
	const std::string& endPoint,
	uint16_t serverId,
	const std::string& acceptAddress,
	const std::string& processPath,
	const std::string& projectName,
	uint16_t serverRegion)

	: CModule(moduleName)
{
	CreatChannel(serverId, endPoint, (uint16_t)REGISTER_TYPE_CACHE,
		acceptAddress, processPath, projectName, serverRegion);
}

CCacheModule::~CCacheModule(void)
{
}

void CCacheModule::OnRegister()
{
}

void CCacheModule::OnRemove()
{
}

std::vector<int> CCacheModule::ListNotificationInterests()
{
	CScopedLock scopedLock(m_notifMutex);
	if(m_notifications.IsInvalid()) {
		CAutoPointer<IChannelValue> rpcChannel(GetFirstChnl());
		assert(!rpcChannel.IsInvalid());
		::node::CacheService_Stub cacheService_stub(&*rpcChannel->GetRpcChannel(), false);
		::node::VoidPacket cacheVoid;
		::node::InterestPacket cacheInterest;

		rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		cacheService_stub.ListNotificationInterests(cacheVoid, &cacheInterest, &controller, NULL);
		controller.wait();

		if (!controller.ok()) {
			if(rpcChannel->IncTimeoutCount()
				>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
			{
				RemoveChannel(rpcChannel->GetServerId());
			}
		} else {
			rpcChannel->SetTimeoutCount(0);
		}

		int nSize = cacheInterest.interests_size();
		if(nSize > 0){
			m_notifications.SetRawPointer(new std::vector<int>(nSize, 0));
			std::vector<int>& vInterest = *m_notifications;
			for(int i = 0; i < nSize; ++i) {
				vInterest[i] = cacheInterest.interests(i);
			}
			return vInterest;
		}
	} else {
		return *m_notifications;
	}

	return std::vector<int>();
}

IModule::InterestList CCacheModule::ListProtocolInterests()
{
	CScopedLock scopedLock(m_protoMutex);
	InterestList interests;
	if(m_protocols.IsInvalid()) {
		CAutoPointer<IChannelValue> rpcChannel(GetFirstChnl());
		assert(!rpcChannel.IsInvalid());
		::node::CacheService_Stub cacheService_stub(&*rpcChannel->GetRpcChannel(), false);
		::node::VoidPacket cacheVoid;
		::node::InterestPacket cacheInterest;

		rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		cacheService_stub.ListProtocolInterests(cacheVoid, &cacheInterest, &controller, NULL);
		controller.wait();

		if (!controller.ok()) {
			if(rpcChannel->IncTimeoutCount()
				>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
			{
				RemoveChannel(rpcChannel->GetServerId());
			}
		} else {
			rpcChannel->SetTimeoutCount(0);
		}

		int nSize = cacheInterest.interests_size();
		if(nSize > 0){
			m_protocols.SetRawPointer(new std::vector<int>);
			std::vector<int>& vInterest = *m_protocols;
			for(int i = 0; i < nSize; ++i) {
				int nProtocal = cacheInterest.interests(i);		
				if(FilterProtocolInterest(nProtocal)) {
					continue;
				}
				vInterest.push_back(nProtocal);
				interests.push_back(BindMethod<CCacheModule>(
					nProtocal, &CCacheModule::HandleMessage));
			}
		}
	} else {
		const std::vector<int>& vInterest = *m_protocols;
		int nSize = (int)vInterest.size();
		for(int i = 0; i < nSize; ++i) {
			interests.push_back(BindMethod<CCacheModule>(
				vInterest[i], &CCacheModule::HandleMessage));
		}
	}
	return interests;
}

void CCacheModule::HandleNotification(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pDpRequest(GetCacheRequestPacket(request));
	if(pDpRequest.IsInvalid()) {
		OutputError("The body isn't CBodyMessage Type!");
		return;
	}

	CWeakPointer<::node::DataPacket> pDpResponse(GetCacheResponsePacket(reply));

	eRouteType routeType = (eRouteType)pDpRequest->route_type();

	if(ROUTE_BROADCAST_USER == routeType) {
		std::vector<util::CAutoPointer<IChannelValue> > channels;
		IterateChannel(channels);
		int nSize = (int)channels.size();
		CAutoPointer<IChannelValue> rpcChannel;
		for(int i = 0; i < nSize; ++i) {
			rpcChannel = channels[i];
			if(rpcChannel.IsInvalid()) {
				OutputError("rpcChannel.IsInvalid() routeType = %d", routeType);
				continue;
			}
			::node::CacheService_Stub cacheService_stub(&*rpcChannel->GetRpcChannel(), false);

			rpc_controller controller;
			controller.set_deadline_ms(CALL_DEADLINE_MS);
			::node::DataPacket dpResponse;
			cacheService_stub.HandleNotification(*pDpRequest, &dpResponse, &controller, NULL);
			controller.wait();

			if(!pDpResponse.IsInvalid()) {
				pDpResponse->MergeFrom(dpResponse);
			}

			if (!controller.ok()) {
				if(rpcChannel->IncTimeoutCount()
					>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
				{
					RemoveChannel(rpcChannel->GetServerId());
				}
			} else {
				rpcChannel->SetTimeoutCount(0);
			}
		}
	} else {
		CAutoPointer<IChannelValue> rpcChannel;
		if(ROUTE_BALANCE_USERID == routeType) {
			uint64_t userId = pDpRequest->route();
			rpcChannel = GetChnlByBalUserId(userId);
		} else if(ROUTE_BALANCE_SERVERID == routeType) {
			uint16_t serverId = (uint16_t)pDpRequest->route();
			rpcChannel = GetChnlByBalServId(serverId);
		} else if(ROUTE_DIRECT_SERVERID == routeType) {
			uint16_t serverId = (uint16_t)pDpRequest->route();
			rpcChannel = GetChnlByDirServId(serverId);
		}

		if(rpcChannel.IsInvalid()) {
			OutputDebug("rpcChannel.IsInvalid() routeType = %d", routeType);
			assert(!rpcChannel.IsInvalid());
			return;
		}
		::node::CacheService_Stub cacheService_stub(&*rpcChannel->GetRpcChannel(), false);

		rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		::node::DataPacket dpResponse;
		cacheService_stub.HandleNotification(*pDpRequest, &dpResponse, &controller, NULL);
		controller.wait();

		if(!pDpResponse.IsInvalid()) {
			pDpResponse->MergeFrom(dpResponse);
		}

		if (!controller.ok()) {
			if(rpcChannel->IncTimeoutCount()
				>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
			{
				RemoveChannel(rpcChannel->GetServerId());
			}
		} else {
			rpcChannel->SetTimeoutCount(0);
		}
	}
}

bool CCacheModule::CreatChannel(uint16_t serverId, const std::string& endPoint, uint16_t serverType)
{
	thd::CScopedWriteLock writeLock(m_rwServer);
	bool objectExists = false;
	unsigned int index = m_serverIdChnls.GetIndexFromKey(serverId, &objectExists);
	if(objectExists) {
		return false;
	}

	util::CAutoPointer<IChannelValue> rpcChannel(new CCacheChannel0);
	if(rpcChannel.IsInvalid()) {
		return false;
	}
	rpcChannel->SetTimeoutCount(0);
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	rpcChannel->SetRpcChannel(pChlMgr->GetRpczChannel(endPoint));
	rpcChannel->SetEndPoint(endPoint);
	rpcChannel->SetServerId(serverId);
	rpcChannel->SetServerType(serverType);

	if(index >= m_serverIdChnls.Size()) {
		m_serverIdChnls.InsertAtEnd(rpcChannel);
	} else {
		m_serverIdChnls.InsertAtIndex(rpcChannel, index);
	}

	return true;
}

bool CCacheModule::CreatChannel(uint16_t serverId, const std::string& endPoint, uint16_t serverType,
	const std::string& acceptAddress, const std::string& processPath, const std::string& projectName,
	uint16_t serverRegion)
{
	thd::CScopedWriteLock writeLock(m_rwServer);
	bool objectExists = false;
	unsigned int index = m_serverIdChnls.GetIndexFromKey(serverId, &objectExists);
	if(objectExists) {
		return false;
	}

	util::CAutoPointer<IChannelValue> rpcChannel(new CCacheChannel1);
	if(rpcChannel.IsInvalid()) {
		return false;
	}
	rpcChannel->SetTimeoutCount(0);
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	rpcChannel->SetRpcChannel(pChlMgr->GetRpczChannel(endPoint));
	rpcChannel->SetEndPoint(endPoint);
	rpcChannel->SetServerId(serverId);
	rpcChannel->SetServerType(serverType);
	rpcChannel->SetAcceptAddress(acceptAddress);
	rpcChannel->SetProcessPath(processPath);
	rpcChannel->SetProjectName(projectName);
	rpcChannel->SetServerRegion(serverRegion);

	if(index >= m_serverIdChnls.Size()) {
		m_serverIdChnls.InsertAtEnd(rpcChannel);
	} else {
		m_serverIdChnls.InsertAtIndex(rpcChannel, index);
	}

	return true;
}

CAutoPointer<IChannelValue> CCacheModule::GetChnlByBalUserId(uint64_t userId) const
{
	if(ID_NULL == userId) {
		return CAutoPointer<IChannelValue>();
	}

	CScopedReadLock readLock(m_rwServer);
	int32_t nSize = (int32_t)m_serverIdChnls.Size();
	int32_t nIndex = JumpConsistentHash(userId, nSize);
	if(nIndex < 0 || nIndex >= nSize) {
		assert(false);
		return CAutoPointer<IChannelValue>();
	}

	return m_serverIdChnls[nIndex];
}

bool CCacheModule::RemoveChannel(uint16_t serverId)
{
	thd::CScopedWriteLock writeLock(m_rwServer);
	if(m_serverIdChnls.Remove(serverId) == ARRAYLIST_MAX_INDEX) {
		return false;
	}
	return true;
}

int CCacheModule::ChannelCount() const
{
	CScopedReadLock readLock(m_rwServer);
	return (int)m_serverIdChnls.Size();
}

void CCacheModule::IterateChannel(std::vector<util::CAutoPointer<IChannelValue> >& outChannels) const
{
	CScopedReadLock readLock(m_rwServer);
	unsigned int nSize = m_serverIdChnls.Size();
	for(unsigned int i = 0; i < nSize; ++i) {
		outChannels.push_back(m_serverIdChnls[i]);
	}
}

CAutoPointer<IChannelValue> CCacheModule::GetChnlByDirServId(uint16_t serverId) const {
	CScopedReadLock readLock(m_rwServer);
	return m_serverIdChnls.GetElementFromKey(serverId);
}

void CCacheModule::HandleMessage(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pDpResponse(GetCacheResponsePacket(reply));

	CWeakPointer<::node::DataPacket> pDpRequest(GetCacheRequestPacket(request));
	if(pDpRequest.IsInvalid()) {
		OutputError("The Message isn't ::node::DispatchRequest Type!");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(FALSE);
		}
		return;
	}

	uint64_t userId = pDpRequest->route();

	CAutoPointer<IChannelValue> rpcChannel(GetChnlByBalUserId(userId));
	if(rpcChannel.IsInvalid()) {
		OutputError("No Channel!");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(FALSE);
		}
		return;
	}

	::node::CacheService_Stub cacheService_stub(&*rpcChannel->GetRpcChannel(), false);

	rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);
	::node::DataPacket dpResponse;
	cacheService_stub.HandleProtocol(*pDpRequest, &dpResponse, &controller, NULL);
	controller.wait();

	if(!pDpResponse.IsInvalid()) {
		pDpResponse->MergeFrom(dpResponse);
	}

	if (!controller.ok()) {
		if(rpcChannel->IncTimeoutCount()
			>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
		{
			RemoveChannel(rpcChannel->GetServerId());
		}
	} else {
		rpcChannel->SetTimeoutCount(0);
	}
}

CAutoPointer<IChannelValue> CCacheModule::GetChnlByBalServId(uint16_t serverId) const
{
	if(ID_NULL == serverId) {
		return CAutoPointer<IChannelValue>();
	}

	CScopedReadLock readLock(m_rwServer);
	int32_t nSize = (int32_t)m_serverIdChnls.Size();
	int32_t nIndex = JumpConsistentHash(serverId, nSize);
	if(nIndex < 0 || nIndex >= nSize) {
		assert(false);
		return CAutoPointer<IChannelValue>();
	}

	return m_serverIdChnls[nIndex];
}

int CCacheModule::ServidArrMapComparison(const uint16_t& a, const util::CAutoPointer<IChannelValue>& b)
{
	if(a < b->GetServerId()) return -1; if (a == b->GetServerId()) return 0; return 1;
}

