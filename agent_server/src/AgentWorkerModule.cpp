#include "AgentWorkerModule.h"
#include "WorkerOperateHelper.h"
#include "ModuleOperateHelper.h"
#include "SeparatedStream.h"

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


std::vector<uint32_t> CAgentWorkerModule::s_allMaps;
thd::CSpinRWLock CAgentWorkerModule::s_allMapRwLock;

CAgentWorkerModule::CAgentWorkerModule(const std::string& moduleName)
	:  CWorkerModule(moduleName)
{
	CSeparatedStream enclosure(
		moduleName.data(), moduleName.length(),
		false, ',', ',');
	enclosure.Ignore();

	uint32_t temp;
	do {
		enclosure >> temp;
		CScopedWriteLock scopedWLock(m_mapRwLock);
		m_maps.insert(temp);
	} while (enclosure.MoreData());

	do {
		CScopedReadLock scopedRLock(m_mapRwLock);
		std::set<uint32_t>::const_iterator it(m_maps.begin());
		if(m_maps.end() != it) {
			CScopedWriteLock allWLock(s_allMapRwLock);
			s_allMaps.push_back(*it);
		}
	} while (false);
}

CAgentWorkerModule::CAgentWorkerModule(
	const std::string& moduleName,
	const std::string& endPoint,
	uint32_t serverId,
	bool routeServer,
	uint64_t routeAddressId,
	const ROUTE_USERIDS_T& routeUserIds)

	: CWorkerModule(moduleName, endPoint, serverId, routeServer, routeAddressId, routeUserIds)
{
	CSeparatedStream enclosure(
		moduleName.data(), moduleName.length(),
		false, ',', ',');
	enclosure.Ignore();

	uint32_t temp;
	do {
		enclosure >> temp;
		CScopedWriteLock scopedWLock(m_mapRwLock);
		m_maps.insert(temp);
	} while (enclosure.MoreData());

	do {
		CScopedReadLock scopedRLock(m_mapRwLock);
		std::set<uint32_t>::const_iterator it(m_maps.begin());
		if(m_maps.end() != it) {
			CScopedWriteLock allWLock(s_allMapRwLock);
			s_allMaps.push_back(*it);
		}
	} while (false);

}

uint32_t CAgentWorkerModule::GetBalMapID(uint64_t route) {
	CScopedReadLock allRLock(s_allMapRwLock);
	int32_t nSize = static_cast<int32_t>(s_allMaps.size());
	int32_t nIdx = JumpConsistentHash(route, nSize);
	if (nIdx < 0 || nIdx >= nSize) {
		return ID_NULL;
	}
	return s_allMaps[nIdx];
}

std::vector<int> CAgentWorkerModule::ListNotificationInterests()
{
	CScopedLock scopedLock(m_notifMutex);
	if (m_notifications.IsInvalid()) {
		CAutoPointer<IChannelValue> rpcChannel(GetLowLoadUserChnl());
		assert(!rpcChannel.IsInvalid());
		::node::WorkerService_Stub workerService_stub(&*rpcChannel->GetRpcChannel(), false);
		::node::VoidPacket workerVoid;
		::node::InterestPacket workerInterest;

		rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		workerService_stub.ListNotificationInterests(workerVoid, &workerInterest, &controller, NULL);
		controller.wait();

		if (!controller.ok()) {
			if (rpcChannel->IncTimeoutCount()
				>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
			{
				RemoveChannel(rpcChannel->GetServerId());
			}
		}
		else {
			rpcChannel->SetTimeoutCount(0);
		}

		int nSize = workerInterest.interests_size();
		if (nSize > 0) {
			m_notifications.SetRawPointer(new std::vector<int>(nSize, 0));
			std::vector<int>& vInterest = *m_notifications;
			for (int i = 0; i < nSize; ++i) {
				vInterest[i] = workerInterest.interests(i);
			}
			vInterest.push_back(N_CMD_CHECK_SWITCH_MAPID);
			return vInterest;
		}
	}
	else {
		return *m_notifications;
	}
	return std::vector<int>({ N_CMD_CHECK_SWITCH_MAPID });
}

IModule::InterestList CAgentWorkerModule::ListProtocolInterests()
{
	CScopedLock scopedLock(m_protoMutex);
	InterestList interests;
	if (m_protocols.IsInvalid()) {
		CAutoPointer<IChannelValue> rpcChannel(GetLowLoadUserChnl());
		assert(!rpcChannel.IsInvalid());
		::node::WorkerService_Stub workerService_stub(&*rpcChannel->GetRpcChannel(), false);
		::node::VoidPacket workerVoid;
		::node::InterestPacket workerInterest;

		rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		workerService_stub.ListProtocolInterests(workerVoid, &workerInterest, &controller, NULL);
		controller.wait();

		if (!controller.ok()) {
			if (rpcChannel->IncTimeoutCount()
				>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
			{
				RemoveChannel(rpcChannel->GetServerId());
			}
		}
		else {
			rpcChannel->SetTimeoutCount(0);
		}

		int nSize = workerInterest.interests_size();
		if (nSize > 0) {
			m_protocols.SetRawPointer(new std::vector<int>(nSize, 0));
			std::vector<int>& vInterest = *m_protocols;
			for (int i = 0; i < nSize; ++i) {
				vInterest[i] = workerInterest.interests(i);
				interests.push_back(BindMethod<CAgentWorkerModule>(
					vInterest[i], &CAgentWorkerModule::HandleAgentMessage));
			}
		}
	}
	else {
		const std::vector<int>& vInterest = *m_protocols;
		int nSize = (int)vInterest.size();
		for (int i = 0; i < nSize; ++i) {
			interests.push_back(BindMethod<CAgentWorkerModule>(
				vInterest[i], &CAgentWorkerModule::HandleAgentMessage));
		}
	}
	return interests;
}

CAutoPointer<IObserverRestricted> CAgentWorkerModule::FullProtocolInterests() {
	return BindMethod<CAgentWorkerModule>(&CAgentWorkerModule::HandleAgentMessage);
}

void CAgentWorkerModule::HandleAgentMessage(
	const util::CWeakPointer<mdl::INotification>& request,
	util::CWeakPointer<mdl::IResponse>& reply)
{
	do {
		CScopedReadLock scopedRLock(m_mapRwLock);
		if (!m_maps.empty()) {
			if (m_maps.find(request->GetType()) == m_maps.end()) {
				// Not the messages of this map.
				return;
			}
		}
	} while (false);

	util::CWeakPointer<::node::DataPacket> pDpResponse(GetWorkerResponsePacket(reply));

	util::CWeakPointer<::node::DataPacket> pDpRequest(GetWorkerRequestPacket(request));
	if(pDpRequest.IsInvalid()) {
		OutputError("The Message isn't ::node::DispatchRequest Type!");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(SERVER_FAILURE);
		}
		return;
	}
	if (!pDpResponse.IsInvalid()) {
		if (CANNT_FIND_MAP == pDpResponse->result()) {
			pDpResponse->clear_result();
		}
	}
	// set cmd
	switch(pDpRequest->cmd()) {
	case P_CMD_S_LOGOUT:
		HandleLogout(pDpRequest, pDpResponse);
		break;
	default:
		HandleDefault(pDpRequest, pDpResponse);
		break;
	}
}

void CAgentWorkerModule::HandleNotification(
	const util::CWeakPointer<mdl::INotification>& request,
	util::CWeakPointer<mdl::IResponse>& reply)
{
	int32_t nCmd = request->GetName();
	if (N_CMD_CHECK_SWITCH_MAPID == nCmd) {
		CaseCheckSwithMapID(request, reply);
	} else {
		util::CWeakPointer<::node::DataPacket> pDpRequest(GetWorkerRequestPacket(request));
		if (pDpRequest.IsInvalid()) {
			OutputError("The body isn't CBodyMessage Type!");
			return;
		}

		util::CWeakPointer<::node::DataPacket> pDpResponse(GetWorkerResponsePacket(reply));
		
		eRouteType routeType = (eRouteType)pDpRequest->route_type();

		if (ROUTE_BALANCE_USERID == routeType) {
			uint32_t nMapId = request->GetType();
			if (ID_NULL == nMapId) {
				uint64_t route = pDpRequest->route();
				nMapId = GetBalMapID(route);
				const_cast<util::CWeakPointer<mdl::INotification>&>(
					request)->SetType(nMapId);
			}
			do {
				CScopedReadLock scopedRLock(m_mapRwLock);
				if (!m_maps.empty()) {
					if (m_maps.find(nMapId) == m_maps.end()) {
						// Not the messages of this map.
						return;
					}
				}
			} while (false);

			if (!pDpResponse.IsInvalid()) {
				if (CANNT_FIND_MAP == pDpResponse->result()) {
					pDpResponse->clear_result();
				}
			}
		}

		CAutoPointer<IChannelValue> rpcChannel;
		if (ROUTE_BALANCE_USERID == routeType) {
			uint32_t nMapId = request->GetType();
			if (ID_NULL == nMapId) {
				rpcChannel = GetFirstChnl();
			} else {
				rpcChannel = GetChnlByBalUserId(pDpRequest->route());
			}
		} else if (ROUTE_DIRECT_SERVERID == routeType) {
			uint32_t serverId = static_cast<uint32_t>(pDpRequest->route());
			rpcChannel = GetChnlByDirServId(serverId);
		} else if (ROUTE_BROADCAST_USER == routeType) {
			rpcChannel = GetFirstChnl();
		} else if (ROUTE_BALANCE_SERVERID == routeType) {
			uint32_t serverId = static_cast<uint32_t>(pDpRequest->route());
			rpcChannel = GetChnlByBalServId(serverId);
		}

		if (rpcChannel.IsInvalid()) {
			OutputError("rpcChannel.IsInvalid() routeType = %d route = "
				I64FMTD, routeType, pDpRequest->route());
			return;
		}

		::node::WorkerService_Stub workerService_stub(&*rpcChannel->GetRpcChannel(), false);

		rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);

		workerService_stub.HandleNotification(*pDpRequest, pDpResponse.operator->(), &controller, NULL);
		controller.wait();

		const rpcz::status_code curStatus = controller.get_status();

		if (curStatus == rpcz::status::DEADLINE_EXCEEDED) {
			if (!pDpResponse.IsInvalid()) {
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
}

void CAgentWorkerModule::CaseCheckSwithMapID(
	const util::CWeakPointer<mdl::INotification>& request,
	util::CWeakPointer<mdl::IResponse>& reply)
{
	CWeakPointer<CBodyBitStream> pRequest(GetModuleRequestPacket(request));
	if (pRequest.IsInvalid()) {
		return;
	}

	// response
	CWeakPointer<CBodyBitStream> pResponse(GetModuleResponsePacket(reply));
	if (pResponse.IsInvalid()) {
		return;
	}
	
	bool bSwitchMap = pResponse->ReadBool();
	if(!bSwitchMap) {
		uint32_t oldMapId = pRequest->ReadUInt32();
		uint32_t newMapId = pRequest->ReadUInt32();

		CScopedReadLock scopedRLock(m_mapRwLock);
		if (!m_maps.empty()) {
			bool bExistOldMapId = (m_maps.find(oldMapId) != m_maps.end());
			bool bExistNewMapId = (m_maps.find(newMapId) != m_maps.end());
			if (bExistOldMapId || bExistNewMapId) {
				if (!bExistOldMapId || !bExistNewMapId) {
					pResponse->SetWriteOffset(0);
					pResponse->WriteBool(TRUE);
				}
			}
		}
	}
}