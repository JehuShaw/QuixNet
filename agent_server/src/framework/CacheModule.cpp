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
static int32_t JumpConsistentHash(uint64_t key, int32_t numBuckets) {
	int64_t b = -1, j = 0;
	while (j < numBuckets) {
		b = j;
		key = key * 2862933555777941757ULL + 1;
		j = (int64_t)((b + 1) * (double(1LL << 31) / double((key >> 33) + 1)));
	}
	return (int32_t)b;
}

CCacheModule::CCacheModule(
	const std::string& moduleName,
	const std::string& endPoint,
	uint32_t serverId,
	bool routeServer,
	uint64_t routeAddressId,
	const ROUTE_USERIDS_T& routeUserIds)

	: IChannelControl(moduleName), m_dirServerId(0)
{
	CreatChannel(serverId, endPoint, (uint16_t)REGISTER_TYPE_CACHE,
		routeServer, routeAddressId, routeUserIds);
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

	CAutoPointer<IChannelValue> rpcChannel;
	if(ROUTE_BALANCE_USERID == routeType) {
		rpcChannel = GetChnlByBalUserId(pDpRequest->route());
	} else if (ROUTE_DIRECT_SERVERID == routeType) {
		rpcChannel = GetChnlByDirServId(static_cast<uint32_t>(pDpRequest->route()));
	} else if (ROUTE_BROADCAST_USER == routeType) {
		rpcChannel = GetFirstChnl();
	} else if (ROUTE_BALANCE_SERVERID == routeType) {
		rpcChannel = GetChnlByBalServId(static_cast<uint32_t>(pDpRequest->route()));
		if (!rpcChannel.IsInvalid()) {
			pDpRequest->set_route(rpcChannel->GetRouteAddressId());
		}
	} else if (ROUTE_HASH_32KEY == routeType) {
		rpcChannel = GetChnlByHash32Key(static_cast<uint32_t>(pDpRequest->route()));
	}

	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid() routeType = %d route = "
			I64FMTD, routeType, pDpRequest->route());
		return;
	}

	::node::CacheService_Stub cacheService_stub(&*rpcChannel->GetRpcChannel(), false);

	rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);

	cacheService_stub.HandleNotification(*pDpRequest, pDpResponse.operator->(), &controller, NULL);
	controller.wait();

	const rpcz::status_code curStatus = controller.get_status();

	if (curStatus == rpcz::status::DEADLINE_EXCEEDED) {
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_CALL_DEADLINE);
		}
	}

	if (curStatus != rpcz::status::OK) {
		if(rpcChannel->IncTimeoutCount()
			>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
		{
			RemoveChannel(rpcChannel->GetServerId());
		}
	} else {
		rpcChannel->SetTimeoutCount(0);
	}
}

bool CCacheModule::CreatChannel(uint32_t serverId, const std::string& endPoint, uint16_t serverType,
	bool routeServer, uint64_t routeAddressId, const ROUTE_USERIDS_T& routeUserIds)
{
	if (ID_NULL == serverId) {
		OutputError("ID_NULL == serverId");
		assert(false);
		return false;
	}

	if (routeServer) {
		atomic_xchg(&m_dirServerId, serverId);
	}

	UpdateRouteUserIds(serverId, routeUserIds);

	CScopedWriteLock writeLock(m_rwDirServer);
	CAutoPointer<IChannelValue> rpcChannel;
	std::pair<DIRSERVID_TO_CHNL_T::iterator, bool> pairIB(
		m_dirServIdChnls.insert(DIRSERVID_TO_CHNL_T::value_type(
			serverId, rpcChannel)));
	if (!pairIB.second) {
		return false;
	}

	pairIB.first->second.SetRawPointer(new CCacheChannel0);
	rpcChannel = pairIB.first->second;
	if(rpcChannel.IsInvalid()) {
		return false;
	}
	rpcChannel->SetTimeoutCount(0);
	rpcChannel->SetEndPoint(endPoint);
	rpcChannel->SetServerId(serverId);
	rpcChannel->SetServerType(serverType);
	rpcChannel->SetRouteAddressId(routeAddressId);
	rpcChannel->XchgServerLoad(routeUserIds.size());
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	rpcChannel->SetRpcChannel(pChlMgr->GetRpczChannel(endPoint));
	return true;
}

bool CCacheModule::RemoveChannel(uint32_t serverId)
{
	if (ID_NULL == serverId) {
		OutputError("ID_NULL == serverId");
		assert(false);
		return false;
	}
	int32_t serverLoad = 0;
	CScopedWriteLock wDirLock(m_rwDirServer);
	DIRSERVID_TO_CHNL_T::iterator itD(
		m_dirServIdChnls.find(serverId));
	if (m_dirServIdChnls.end() == itD) {
		return false;
	}
	serverLoad = itD->second->XchgServerLoad(0);
	m_dirServIdChnls.erase(itD);
	wDirLock.Unlock();

	if (serverLoad > 0) {
		ClearRouteUserIds(serverId);
	}
	return true;
}

int CCacheModule::ChannelCount() const
{
	CScopedReadLock readLock(m_rwDirServer);
	return static_cast<int>(m_dirServIdChnls.size());
}

void CCacheModule::IterateChannel(std::vector<util::CAutoPointer<IChannelValue> >& outChannels) const
{
	CScopedReadLock readLock(m_rwDirServer);
	DIRSERVID_TO_CHNL_T::const_iterator it(m_dirServIdChnls.begin());
	for (; m_dirServIdChnls.end() != it; ++it) {
		outChannels.push_back(it->second);
	}
}

CAutoPointer<IChannelValue> CCacheModule::GetChnlByDirServId(uint32_t serverId) const {
	if (ID_NULL == serverId) {
		OutputError("ID_NULL == serverId");
		assert(false);
		return CAutoPointer<IChannelValue>();
	}
	CScopedReadLock readLock(m_rwDirServer);
	DIRSERVID_TO_CHNL_T::const_iterator it(
		m_dirServIdChnls.find(serverId));

	if (m_dirServIdChnls.end() == it) {
		return CAutoPointer<IChannelValue>();
	}
	return it->second;
}

CAutoPointer<IChannelValue> CCacheModule::GetChnlByBalUserId(uint64_t userId)
{
	if (ID_NULL == userId) {
		OutputError("ID_NULL == userId");
		assert(false);
		return CAutoPointer<IChannelValue>();
	}
	CScopedReadLock rDirLock(m_rwDirServer);
	CScopedReadLock rUserLock(m_rwBalUser);
	BALUERID_TO_SERVID_T::const_iterator itU(m_balUserIds.find(userId));
	if (m_balUserIds.end() != itU) {
		DIRSERVID_TO_CHNL_T::const_iterator itD(
			m_dirServIdChnls.find(itU->second));
		if (m_dirServIdChnls.end() != itD) {
			return itD->second;
		}
	}

	if (m_dirServIdChnls.empty()) {
		return CAutoPointer<IChannelValue>();
	}
	int32_t nSize = static_cast<int32_t>(m_dirServIdChnls.size());
	int32_t nIndex = JumpConsistentHash(userId, nSize);
	if (nIndex < 0 || nIndex >= nSize) {
		assert(false);
		return CAutoPointer<IChannelValue>();
	}
	DIRSERVID_TO_CHNL_T::iterator itD(m_dirServIdChnls.begin());
	std::advance(itD, nIndex);
	CScopedWriteLock wUserLock(rUserLock);
	m_balUserIds.insert(BALUERID_TO_SERVID_T::value_type(userId, itD->first));
	wUserLock.Unlock();
	itD->second->IncServerLoad();
	return itD->second;
}

void CCacheModule::RemoveBalUserId(uint64_t userId, bool bRemoveRoute/* = true */)
{
	if (ID_NULL == userId) {
		OutputError("ID_NULL == userId");
		assert(false);
		return;
	}

	if (bRemoveRoute) {
		CScopedReadLock rDirLock(m_rwDirServer);
		CScopedWriteLock writeLock(m_rwBalUser);
		BALUERID_TO_SERVID_T::const_iterator itU(m_balUserIds.find(userId));
		if (m_balUserIds.end() == itU) {
			return;
		}
		DIRSERVID_TO_CHNL_T::iterator itD(m_dirServIdChnls.find(itU->second));
		if (m_dirServIdChnls.end() != itD) {
			itD->second->DecServerLoad();
		}
		m_balUserIds.erase(itU);
	}
}

CAutoPointer<IChannelValue> CCacheModule::GetChnlByBalServId(uint32_t serverId)
{
	if (ID_NULL == serverId) {
		OutputError("ID_NULL == serverId");
		assert(false);
		return CAutoPointer<IChannelValue>();
	}
	CScopedReadLock rDirLock(m_rwDirServer);
	DIRSERVID_TO_CHNL_T::const_iterator itD(m_dirServIdChnls.end());
	uint32_t dirServerId = m_dirServerId;
	if (dirServerId != ID_NULL) {
		itD = m_dirServIdChnls.find(dirServerId);
	}
	if (m_dirServIdChnls.end() == itD) {
		if (m_dirServIdChnls.empty()) {
			return CAutoPointer<IChannelValue>();
		}
		int32_t nSize = static_cast<int32_t>(m_dirServIdChnls.size());
		int32_t nIndex = JumpConsistentHash(serverId, nSize);
		if (nIndex < 0 || nIndex >= nSize) {
			assert(false);
			return CAutoPointer<IChannelValue>();
		}
		itD = m_dirServIdChnls.begin();
		std::advance(itD, nIndex);
		atomic_xchg(&m_dirServerId, itD->first);
		return itD->second;
	}
	return itD->second;
}

CAutoPointer<IChannelValue> CCacheModule::GetFirstChnl() const
{
	CScopedReadLock rDirLock(m_rwDirServer);
	if (m_dirServIdChnls.empty()) {
		return CAutoPointer<IChannelValue>();
	}
	return m_dirServIdChnls.begin()->second;
}

util::CAutoPointer<IChannelValue> CCacheModule::GetChnlByHash32Key(uint32_t u32Key)
{
	if (m_dirServIdChnls.empty()) {
		return CAutoPointer<IChannelValue>();
	}
	int32_t nSize = static_cast<int32_t>(m_dirServIdChnls.size());
	int32_t nIndex = JumpConsistentHash(u32Key, nSize);
	if (nIndex < 0 || nIndex >= nSize) {
		assert(false);
		return CAutoPointer<IChannelValue>();
	}
	DIRSERVID_TO_CHNL_T::const_iterator itD(m_dirServIdChnls.begin());
	std::advance(itD, nIndex);
	return itD->second;
}

void CCacheModule::UpdateRouteUserIds(uint32_t serverId, const ROUTE_USERIDS_T& routeUserIds)
{
	int32_t serverLoad = 0;
	CScopedReadLock rDirLock(m_rwDirServer);
	DIRSERVID_TO_CHNL_T::iterator itD(m_dirServIdChnls.find(serverId));
	if (m_dirServIdChnls.end() != itD) {
		serverLoad = itD->second->XchgServerLoad(0);
	}
	rDirLock.Unlock();
	if (serverLoad > 0) {
		ClearRouteUserIds(serverId);
	}

	int nSize = routeUserIds.size();
	for (int i = 0; i < nSize; ++i) {
		CScopedWriteLock wUserLock(m_rwBalUser);
		m_balUserIds.insert(BALUERID_TO_SERVID_T::value_type(routeUserIds.Get(i), serverId));
	}
}

void CCacheModule::ClearRouteUserIds(uint32_t serverId)
{
	CScopedReadLock rUserLock(m_rwBalUser);
	BALUERID_TO_SERVID_T::const_iterator itU(m_balUserIds.begin());
	while (m_balUserIds.end() != itU) {
		if (itU->second == serverId) {
			CScopedWriteLock wUserLock(rUserLock);
			itU = m_balUserIds.erase(itU);
			rUserLock.Degrade(wUserLock);
		} else {
			++itU;
		}
	}
}

void CCacheModule::HandleMessage(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pDpResponse(GetCacheResponsePacket(reply));

	CWeakPointer<::node::DataPacket> pDpRequest(GetCacheRequestPacket(request));
	if(pDpRequest.IsInvalid()) {
		OutputError("The Message isn't ::node::DispatchRequest Type!");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_FAILURE);
		}
		return;
	}

	switch (pDpRequest->cmd()) {
	case P_CMD_S_LOGOUT:
		HandleLogout(pDpRequest, pDpResponse);
		break;
	default:
		HandleDefault(pDpRequest, pDpResponse);
		break;
	}
}

void CCacheModule::HandleLogout(
	const util::CWeakPointer<::node::DataPacket>& pDpRequest,
	util::CWeakPointer<::node::DataPacket>& pDpResponse)
{
	HandleDefault(pDpRequest, pDpResponse);
	uint64_t userId = pDpRequest->route();
	RemoveBalUserId(userId);
}

void CCacheModule::HandleDefault(const util::CWeakPointer<::node::DataPacket>& pDpRequest,
	util::CWeakPointer<::node::DataPacket>& pDpResponse)
{
	uint64_t userId = pDpRequest->route();

	CAutoPointer<IChannelValue> rpcChannel(GetChnlByBalUserId(userId));
	if (rpcChannel.IsInvalid()) {
		OutputError("No Channel!");
		if (!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		}
		return;
	}

	::node::CacheService_Stub cacheService_stub(&*rpcChannel->GetRpcChannel(), false);

	rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);
	::node::DataPacket dpResponse;
	cacheService_stub.HandleProtocol(*pDpRequest, &dpResponse, &controller, NULL);
	controller.wait();

	const rpcz::status_code curStatus = controller.get_status();

	if (!pDpResponse.IsInvalid()) {
		pDpResponse->MergeFrom(dpResponse);

		if (curStatus == rpcz::status::DEADLINE_EXCEEDED) {
			pDpResponse->set_result(SERVER_CALL_DEADLINE);
		}
	}

	if (curStatus != rpcz::status::OK) {
		if (rpcChannel->IncTimeoutCount()
			>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
		{
			RemoveChannel(rpcChannel->GetServerId());
		}
	} else {
		rpcChannel->SetTimeoutCount(0);
	}
}

