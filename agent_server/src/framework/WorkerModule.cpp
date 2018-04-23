#include "WorkerModule.h"
#include "NodeDefines.h"
#include "SpinRWLock.h"
#include "ChannelManager.h"
#include "rpc_controller.hpp"
#include "BodyMessage.h"
#include "WorkerOperateHelper.h"
#include "worker.rpcz.h"
#include "WorkerChannel.h"
#include "dbstl_common.h"
#include "TimerManagerHelper.h"

using namespace mdl;
using namespace rpcz;
using namespace util;
using namespace thd;

CWorkerModule::CWorkerModule(
	DbEnv* pDbEnv,
	uint16_t thisServId,
	const std::string& moduleName)

	: CModule(moduleName)
	, m_lessUserQueue(CAutoPointer<CWorkerComparer>(new CWorkerComparer))
	, m_lessServQueue(CAutoPointer<CWorkerComparer>(new CWorkerComparer))
{
	m_pThis.SetRawPointer(this, false);
	if(pDbEnv) {
		char szBuf[64] = { '\0' };
		ultostr(szBuf, thisServId, 10, 0);
		std::string strDbName(szBuf);
		strDbName += '_';
		strDbName += moduleName;

		if(true) {
			CScopedLock lock(m_lkDB);
			m_pDb = dbstl::open_db(pDbEnv, strDbName.c_str(),
				DB_BTREE, DB_CREATE, 0);
		}
		bool bDBChnlEmpty = true;
		if(true) {
			CScopedWriteLock wDbLock(m_rwDbChnls);
			m_dbServIdChnls.set_db_handle(m_pDb);
			bDBChnlEmpty = m_dbServIdChnls.empty();
		}
		if(true) {
			CScopedWriteLock wLock(m_rwBalUser);
			m_balUserIdChnls.set_db_handle(m_pDb);
		}
		if(true) {
			CScopedWriteLock wLock(m_rwBalServer);
			m_balServIdChnls.set_db_handle(m_pDb);
		}

		if(!bDBChnlEmpty) {
			evt::SetTimeout(WORKERMODULE_DELAY_TIME, CAutoPointer<CallbackMFnP0<CWorkerModule> >(
				new CallbackMFnP0<CWorkerModule>(m_pThis, &CWorkerModule::RemoveInvalidServId)));
		}
		
	} else {
		assert(pDbEnv);
	}
}

CWorkerModule::CWorkerModule(
	DbEnv* pDbEnv,
	uint16_t thisServId,
	const std::string& moduleName,
	const std::string& endPoint,
	uint16_t serverId)

	: CModule(moduleName)
	, m_lessUserQueue(CAutoPointer<CWorkerComparer>(new CWorkerComparer))
	, m_lessServQueue(CAutoPointer<CWorkerComparer>(new CWorkerComparer))
{
	m_pThis.SetRawPointer(this, false);
	if(pDbEnv) {
		char szBuf[64] = { '\0' };
		ultostr(szBuf, thisServId, 10, 0);
		std::string strDbName(szBuf);
		strDbName += '_';
		strDbName += moduleName;

		if(true) {
			CScopedLock lock(m_lkDB);
			m_pDb = dbstl::open_db(pDbEnv, strDbName.c_str(),
				DB_BTREE, DB_CREATE, 0);
		}
		bool bDBChnlEmpty = true;
		if(true) {
			CScopedWriteLock wDbLock(m_rwDbChnls);
			m_dbServIdChnls.set_db_handle(m_pDb);
			bDBChnlEmpty = m_dbServIdChnls.empty();
		}
		if(true) {
			CScopedWriteLock wLock(m_rwBalUser);
			m_balUserIdChnls.set_db_handle(m_pDb);
		}
		if(true) {
			CScopedWriteLock wLock(m_rwBalServer);
			m_balServIdChnls.set_db_handle(m_pDb);
		}

		if(!bDBChnlEmpty) {
			evt::SetTimeout(WORKERMODULE_DELAY_TIME, CAutoPointer<CallbackMFnP0<CWorkerModule> >(
				new CallbackMFnP0<CWorkerModule>(m_pThis, &CWorkerModule::RemoveInvalidServId)));
		}
	} else {
		assert(pDbEnv);
	}
	CreatChannel(serverId, endPoint, (uint16_t)REGISTER_TYPE_WORKER);
}

CWorkerModule::CWorkerModule(
	DbEnv* pDbEnv,
	uint16_t thisServId,
	const std::string& moduleName,
	const std::string& endPoint,
	uint16_t serverId,
	const std::string& acceptAddress,
	const std::string& processPath,
	const std::string& projectName,
	uint16_t serverRegion)

	: CModule(moduleName)
	, m_lessUserQueue(CAutoPointer<CWorkerComparer>(new CWorkerComparer))
	, m_lessServQueue(CAutoPointer<CWorkerComparer>(new CWorkerComparer))
{
	m_pThis.SetRawPointer(this, false);
	if(pDbEnv) {
		char szBuf[64] = { '\0' };
		ultostr(szBuf, thisServId, 10, 0);
		std::string strDbName(szBuf);
		strDbName += '_';
		strDbName += moduleName;

		if(true) {
			CScopedLock lock(m_lkDB);
			m_pDb = dbstl::open_db(pDbEnv, strDbName.c_str(),
				DB_BTREE, DB_CREATE, 0);
		}
		bool bDBChnlEmpty = true;
		if(true) {
			CScopedWriteLock wDbLock(m_rwDbChnls);
			m_dbServIdChnls.set_db_handle(m_pDb);
			bDBChnlEmpty = m_dbServIdChnls.empty();
		}
		if(true) {
			CScopedWriteLock wLock(m_rwBalUser);
			m_balUserIdChnls.set_db_handle(m_pDb);
		}
		if(true) {
			CScopedWriteLock wLock(m_rwBalServer);
			m_balServIdChnls.set_db_handle(m_pDb);
		}

		if(!bDBChnlEmpty) {
			evt::SetTimeout(WORKERMODULE_DELAY_TIME, CAutoPointer<CallbackMFnP0<CWorkerModule> >(
				new CallbackMFnP0<CWorkerModule>(m_pThis, &CWorkerModule::RemoveInvalidServId)));
		}
	} else {
		assert(pDbEnv);
	}
	CreatChannel(serverId, endPoint, (uint16_t)REGISTER_TYPE_WORKER,
		acceptAddress, processPath, projectName, serverRegion);
}

CWorkerModule::~CWorkerModule(void)
{
	if(true) {
		CScopedLock lock(m_lkDB);
		dbstl::close_db(m_pDb);
	}
	
}

void CWorkerModule::OnRegister()
{
}

void CWorkerModule::OnRemove()
{
}

std::vector<int> CWorkerModule::ListNotificationInterests()
{
	CScopedLock scopedLock(m_notifMutex);
	if(m_notifications.IsInvalid()) {
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
			if(rpcChannel->IncTimeoutCount()
				>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
			{
				RemoveChannel(rpcChannel->GetServerId());
			}
		} else {
			rpcChannel->SetTimeoutCount(0);
		}

		int nSize = workerInterest.interests_size();
		if(nSize > 0){
			m_notifications.SetRawPointer(new std::vector<int>(nSize, 0));
			std::vector<int>& vInterest = *m_notifications;
			for(int i = 0; i < nSize; ++i) {
				vInterest[i] = workerInterest.interests(i);
			}
			vInterest.push_back(N_CMD_REMOVE_BALSERVID);
			return vInterest;
		}
	} else {
		return *m_notifications;
	}
	std::vector<int> vecTemp;
	vecTemp.push_back(N_CMD_REMOVE_BALSERVID);
	return vecTemp;
}

IModule::InterestList CWorkerModule::ListProtocolInterests()
{
	CScopedLock scopedLock(m_protoMutex);
	InterestList interests;
	if(m_protocols.IsInvalid()) {
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
			if(rpcChannel->IncTimeoutCount()
				>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
			{
				RemoveChannel(rpcChannel->GetServerId());
			}
		} else {
			rpcChannel->SetTimeoutCount(0);
		}

		int nSize = workerInterest.interests_size();
		if(nSize > 0){
			m_protocols.SetRawPointer(new std::vector<int>(nSize, 0));
			std::vector<int>& vInterest = *m_protocols;
			for(int i = 0; i < nSize; ++i) {
				vInterest[i] = workerInterest.interests(i);
				interests.push_back(BindMethod<CWorkerModule>(
					vInterest[i], &CWorkerModule::HandleMessage));
			}
		}
	} else {
		const std::vector<int>& vInterest = *m_protocols;
		int nSize = (int)vInterest.size();
		for(int i = 0; i < nSize; ++i) {
			interests.push_back(BindMethod<CWorkerModule>(
				vInterest[i], &CWorkerModule::HandleMessage));
		}
	}
	return interests;
}

CAutoPointer<IObserverRestricted> CWorkerModule::FullProtocolInterests() {
	return BindMethod<CWorkerModule>(&CWorkerModule::HandleMessage);
}

void CWorkerModule::HandleMessage(const util::CWeakPointer<mdl::INotification>& request,
	util::CWeakPointer<mdl::IResponse>& reply)
{
	util::CWeakPointer<::node::DataPacket> pDpResponse(GetWorkerResponsePacket(reply));

	util::CWeakPointer<::node::DataPacket> pDpRequest(GetWorkerRequestPacket(request));
	if(pDpRequest.IsInvalid()) {
		OutputError("The Message isn't ::node::DispatchRequest Type!");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(FALSE);
		}
		return;
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

void CWorkerModule::HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
	util::CWeakPointer<mdl::IResponse>& reply)
{
	util::CWeakPointer<::node::DataPacket> pDpRequest(GetWorkerRequestPacket(request));
	if(pDpRequest.IsInvalid()) {
		OutputError("The body isn't CBodyMessage Type!");
		return;
	}

	util::CWeakPointer<::node::DataPacket> pDpResponse(GetWorkerResponsePacket(reply));

	eRouteType routeType = (eRouteType)pDpRequest->route_type();

	CAutoPointer<IChannelValue> rpcChannel;
	if(ROUTE_BALANCE_USERID == routeType) {
		uint64_t userId = pDpRequest->route();
		if(GetChnlByBalUserId(userId, rpcChannel)) {
			UpdateChnlByBalUserId(userId, rpcChannel);
		}
	} else if(ROUTE_DIRECT_SERVERID == routeType) {
		uint16_t serverId = (uint16_t)pDpRequest->route();
		rpcChannel = GetChnlByDirServId(serverId);
	} else if(ROUTE_BROADCAST_USER == routeType) {
		rpcChannel = GetLowLoadUserChnl();
	} else if(ROUTE_BALANCE_SERVERID == routeType) {
		uint16_t serverId = (uint16_t)pDpRequest->route();
		if(N_CMD_REMOVE_BALSERVID == request->GetName())  {
			bool bChange = GetChnlByBalServId(serverId, rpcChannel);
			RemoveBalServId(serverId);
			if(!pDpResponse.IsInvalid()) {
				pDpResponse->set_result(TRUE);
			}
			if(bChange || rpcChannel.IsInvalid()) {
				return;
			}
		} else {
			if(GetChnlByBalServId(serverId, rpcChannel)) {
				UpdateChnlByBalServId(serverId, rpcChannel);
			}
		}
	}

	if(rpcChannel.IsInvalid()) {
		OutputDebug("No Channel!");
		assert(!rpcChannel.IsInvalid());
		return;
	}

	::node::WorkerService_Stub workerService_stub(&*rpcChannel->GetRpcChannel(), false);

	rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);
	::node::DataPacket dpResponse;
	workerService_stub.HandleNotification(*pDpRequest, &dpResponse, &controller, NULL);
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

bool CWorkerModule::CreatChannel(uint16_t serverId, const std::string& endPoint, uint16_t serverType)
{
	if(ID_NULL == serverId) {
		OutputError("ID_NULL == serverId");
		assert(false);
		return false;
	}
	CScopedWriteLock writeLock(m_rwDirServer);
	CAutoPointer<IChannelValue> rpcChannel;
	std::pair<DIRSERVID_TO_CHNL_T::iterator, bool> pairIB(
	m_dirServIdChnls.insert(DIRSERVID_TO_CHNL_T::value_type(
	serverId, rpcChannel)));
	if(!pairIB.second) {
		return false;
	}
	pairIB.first->second.SetRawPointer(new CWorkerChannel0(serverId));
	rpcChannel = pairIB.first->second;
	if(rpcChannel.IsInvalid()) {
		return false;
	}
	rpcChannel->SetTimeoutCount(0);
	rpcChannel->SetEndPoint(endPoint);
	rpcChannel->SetServerId(serverId);
	rpcChannel->SetServerType(serverType);
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	rpcChannel->SetRpcChannel(pChlMgr->GetRpczChannel(endPoint));

	if(true) {
		CScopedWriteLock wDbLock(m_rwDbChnls);
		DBSERVID_TO_CHNL_T::iterator it(m_dbServIdChnls.lower_bound(serverId));
		if(m_dbServIdChnls.end() != it && it->first == serverId) {
			util::CAutoPointer<CWorkPQItem> pUserPQItem(rpcChannel->GetUserPQItem());
			if(pUserPQItem.IsInvalid()) {
				assert(false);
			} else {
				pUserPQItem->SetCount(it->second.GetUserCount());
			}
			util::CAutoPointer<CWorkPQItem> pServPQItem(rpcChannel->GetServerPQItem());
			if(pServPQItem.IsInvalid()) {
				assert(false);
			} else {
				pServPQItem->SetCount(it->second.GetServCount());
			}
		} else {
			m_dbServIdChnls.insert(it, DBSERVID_TO_CHNL_T::value_type(serverId, CDBChannel()));
		}
	}

	if(true) {
		CScopedWriteLock writeLock(m_rwBalUser);
		m_lessUserQueue.Push(rpcChannel->GetUserPQItem());
	}

	if(true) {
		CScopedWriteLock writeLock(m_rwBalServer);
		m_lessServQueue.Push(rpcChannel->GetServerPQItem());
	}
	return true;
}

bool CWorkerModule::CreatChannel(uint16_t serverId, const std::string& endPoint, uint16_t serverType,
	const std::string& acceptAddress, const std::string& processPath, const std::string& projectName,
	uint16_t serverRegion)
{
	if(ID_NULL == serverId) {
		OutputError("ID_NULL == serverId");
		assert(false);
		return false;
	}
	CScopedWriteLock writeLock(m_rwDirServer);
	CAutoPointer<IChannelValue> rpcChannel;
	std::pair<DIRSERVID_TO_CHNL_T::iterator, bool> pairIB(
	m_dirServIdChnls.insert(DIRSERVID_TO_CHNL_T::value_type(
	serverId, rpcChannel)));
	if(!pairIB.second) {
		return false;
	}
	pairIB.first->second.SetRawPointer(new CWorkerChannel1(serverId));
	rpcChannel = pairIB.first->second;
	if(rpcChannel.IsInvalid()) {
		return false;
	}
	rpcChannel->SetTimeoutCount(0);
	rpcChannel->SetEndPoint(endPoint);
	rpcChannel->SetServerId(serverId);
	rpcChannel->SetServerType(serverType);
	rpcChannel->SetAcceptAddress(acceptAddress);
	rpcChannel->SetProcessPath(processPath);
	rpcChannel->SetProjectName(projectName);
	rpcChannel->SetServerRegion(serverRegion);
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	rpcChannel->SetRpcChannel(pChlMgr->GetRpczChannel(endPoint));

	if(true) {
		CScopedWriteLock wDbLock(m_rwDbChnls);
		DBSERVID_TO_CHNL_T::iterator it(m_dbServIdChnls.lower_bound(serverId));
		if(m_dbServIdChnls.end() != it && it->first == serverId) {
			util::CAutoPointer<CWorkPQItem> pUserPQItem(rpcChannel->GetUserPQItem());
			if(pUserPQItem.IsInvalid()) {
				assert(false);
			} else {
				pUserPQItem->SetCount(it->second.GetUserCount());
			}
			util::CAutoPointer<CWorkPQItem> pServPQItem(rpcChannel->GetServerPQItem());
			if(pServPQItem.IsInvalid()) {
				assert(false);
			} else {
				pServPQItem->SetCount(it->second.GetServCount());
			}
		} else {
			m_dbServIdChnls.insert(it, DBSERVID_TO_CHNL_T::value_type(serverId, CDBChannel()));
		}
	}

	if(true) {
		CScopedWriteLock writeLock(m_rwBalUser);
		m_lessUserQueue.Push(rpcChannel->GetUserPQItem());
	}

	if(true) {
		CScopedWriteLock writeLock(m_rwBalServer);
		m_lessServQueue.Push(rpcChannel->GetServerPQItem());
	}
	return true;
}

bool CWorkerModule::RemoveChannel(uint16_t serverId)
{
	if(ID_NULL == serverId) {
		OutputError("ID_NULL == serverId");
		assert(false);
		return false;
	}
	CScopedWriteLock writeLock(m_rwDirServer);
	DIRSERVID_TO_CHNL_T::const_iterator it(
		m_dirServIdChnls.find(serverId));
	if(m_dirServIdChnls.end() == it) {
		return false;
	}

	if(true) {
		CScopedWriteLock writeLock(m_rwBalUser);
		m_lessUserQueue.Remove(it->second->GetUserPQItem());
	}

	if(true) {
		CScopedWriteLock writeLock(m_rwBalServer);
		m_lessServQueue.Remove(it->second->GetServerPQItem());
	}

	if(true) {
		CScopedWriteLock wDbLock(m_rwDbChnls);
		m_dbServIdChnls.erase(serverId);
	}

	m_dirServIdChnls.erase(it);
	return true;
}

int CWorkerModule::ChannelCount() const
{
	CScopedReadLock readLock(m_rwDirServer);
	return (int)m_dirServIdChnls.size();
}

void CWorkerModule::IterateChannel(std::vector<util::CAutoPointer<IChannelValue> >& outChannels) const
{
	CScopedReadLock readLock(m_rwDirServer);
	DIRSERVID_TO_CHNL_T::const_iterator it(m_dirServIdChnls.begin());
	for(; m_dirServIdChnls.end() != it; ++it) {
		outChannels.push_back(it->second);
	}
}

CAutoPointer<IChannelValue> CWorkerModule::GetLowLoadUserChnl() const
{
	CScopedReadLock rDirLock(m_rwDirServer);
	CAutoPointer<IChannelValue> pChlValue;
	if(true) {
		CScopedReadLock rUserLock(m_rwBalUser);

		util::CAutoPointer<CWorkPQItem> pUserPQItem(
			m_lessUserQueue.Peek());
		if(!pUserPQItem.IsInvalid()) {
			DIRSERVID_TO_CHNL_T::const_iterator it(
				m_dirServIdChnls.find(pUserPQItem->GetServerId()));
			if(m_dirServIdChnls.end() != it) {
				pChlValue = it->second;
			}
		}	
	}
	return pChlValue;
}

CAutoPointer<IChannelValue> CWorkerModule::GetChnlByDirServId(
	uint16_t serverId) const 
{
	if(ID_NULL == serverId) {
		OutputError("ID_NULL == serverId");
		assert(false);
		return CAutoPointer<IChannelValue>();
	}
	CScopedReadLock readLock(m_rwDirServer);
	DIRSERVID_TO_CHNL_T::const_iterator it(
		m_dirServIdChnls.find(serverId));

	if(m_dirServIdChnls.end() == it) {
		return CAutoPointer<IChannelValue>();
	}
	return it->second;
}

bool CWorkerModule::GetChnlByBalUserId(uint64_t userId,
	CAutoPointer<IChannelValue>& outChannel) const
{
	if(ID_NULL == userId) {
		OutputError("ID_NULL == userId");
		assert(false);
		return false;
	}
	CScopedReadLock rDirLock(m_rwDirServer);
	bool bUpdate = false;
	if(true) {
		CScopedReadLock rUserLock(m_rwBalUser);

		BALUSERID_TO_CHNL_T::const_iterator itU(
			m_balUserIdChnls.find(userId));
		if(m_balUserIdChnls.end() != itU) {

			DIRSERVID_TO_CHNL_T::const_iterator itD(
				m_dirServIdChnls.find(itU->second));
			if(m_dirServIdChnls.end() != itD) {
				outChannel = itD->second;
			} else {
				bool bReroute = false;
				if(true) {
					CScopedReadLock rDbLock(m_rwDbChnls);
					DBSERVID_TO_CHNL_T::const_iterator it(
						m_dbServIdChnls.find(itU->second));
					if(m_dbServIdChnls.end() == it) {
						bReroute = true;
					}
				}
				if(bReroute) {
					outChannel = m_lessUserQueue.Peek();
					bUpdate = true;
				}
			}
		} else {
			outChannel = m_lessUserQueue.Peek();
			bUpdate = true;
		}
	}
	return bUpdate;
}

void CWorkerModule::UpdateChnlByBalUserId(uint64_t userId,
	CAutoPointer<IChannelValue>& channel)
{
	if(ID_NULL == userId) {
		OutputError("ID_NULL == userId");
		assert(false);
		return;
	}
	uint16_t nOldServId = ID_NULL;
	if(true) {
		CScopedWriteLock writeLock(m_rwBalUser);
		if(channel.IsInvalid()) {
			if(!m_balUserIdChnls.empty()) {
				m_balUserIdChnls.clear();
			}
		} else {
			util::CAutoPointer<CWorkPQItem> pUserPQItem(
				channel->GetUserPQItem());
			if(pUserPQItem.IsInvalid()) {
				assert(false);
			} else {
				pUserPQItem->AtomicIncCount();
				m_lessUserQueue.Update(pUserPQItem->GetIndex());
				if(true) {
					CScopedReadLock rDbLock(m_rwDbChnls);
					DBSERVID_TO_CHNL_T::iterator it(m_dbServIdChnls.find(
						channel->GetServerId()));
					if(m_dbServIdChnls.end() != it) {
						it->second.SetUserCount(pUserPQItem->GetCount());
					}
				}
			}

			std::pair<BALUSERID_TO_CHNL_T::iterator, bool> pairIB(
				m_balUserIdChnls.insert(BALUSERID_TO_CHNL_T
				::value_type(userId, channel->GetServerId())));
			if(!pairIB.second) {
				nOldServId = pairIB.first->second;
				pairIB.first->second = channel->GetServerId();
			}
		}
	}
	if(ID_NULL != nOldServId) {
		CScopedReadLock rDirLock(m_rwDirServer);
		DIRSERVID_TO_CHNL_T::const_iterator itD(
			m_dirServIdChnls.find(nOldServId));
		if(m_dirServIdChnls.end() != itD) {
			util::CAutoPointer<CWorkPQItem> pUserPQItem(itD->second);
			if(pUserPQItem.IsInvalid()) {
				assert(false);
			} else {
				pUserPQItem->AtomicDecCount();
				m_lessUserQueue.Update(pUserPQItem->GetIndex());
				if(true) {
					CScopedReadLock rDbLock(m_rwDbChnls);
					DBSERVID_TO_CHNL_T::iterator it(m_dbServIdChnls.find(nOldServId));
					if(m_dbServIdChnls.end() != it) {
						it->second.SetUserCount(pUserPQItem->GetCount());
					}
				}
			}
		} else {
			CScopedReadLock rDbLock(m_rwDbChnls);
			DBSERVID_TO_CHNL_T::iterator it(m_dbServIdChnls.find(nOldServId));
			if(m_dbServIdChnls.end() != it) {
				it->second.AtomicDecUserCount();
			}
		}
	}
}

void CWorkerModule::RemoveBalUserId(uint64_t userId)
{
	if(ID_NULL == userId) {
		OutputError("ID_NULL == userId");
		assert(false);
		return;
	}
	CScopedReadLock readLock(m_rwDirServer);
	if(true) {
		CScopedWriteLock writeLock(m_rwBalUser);

		BALUSERID_TO_CHNL_T::const_iterator itU(
			m_balUserIdChnls.find(userId));
		if(m_balUserIdChnls.end() != itU) {
			DIRSERVID_TO_CHNL_T::const_iterator itD(
				m_dirServIdChnls.find(itU->second));	
			if(m_dirServIdChnls.end() != itD) {
				util::CAutoPointer<CWorkPQItem> pUserPQItem(
					itD->second->GetUserPQItem());
				if(!pUserPQItem.IsInvalid()) {
					pUserPQItem->AtomicDecCount();
					m_lessUserQueue.Update(pUserPQItem->GetIndex());
					if(true) {
						CScopedReadLock rDbLock(m_rwDbChnls);
						DBSERVID_TO_CHNL_T::iterator it(m_dbServIdChnls.find(
							itU->second));
						if(m_dbServIdChnls.end() != it) {
							it->second.SetUserCount(pUserPQItem->GetCount());
						}
					}
				}
			}
			m_balUserIdChnls.erase(itU);
		}
	}
}

bool CWorkerModule::GetChnlByBalServId(uint16_t serverId,
	CAutoPointer<IChannelValue>& outChannel) const
{
	if(ID_NULL == serverId) {
		OutputError("ID_NULL == serverId");
		assert(false);
		return false;
	}
	CScopedReadLock rDirLock(m_rwDirServer);
	bool bUpdate = false;
	if(true) {
		CScopedReadLock rSerLock(m_rwBalServer);

		BALSERVID_TO_CHNL_T::const_iterator itS(
			m_balServIdChnls.find(serverId));
		if(m_balServIdChnls.end() != itS) {

			DIRSERVID_TO_CHNL_T::const_iterator itD(
				m_dirServIdChnls.find(itS->second));
			if(m_dirServIdChnls.end() != itD) {
				outChannel = itD->second;
			} else {
				bool bReroute = false;
				if(true) {
					CScopedReadLock rDbLock(m_rwDbChnls);
					DBSERVID_TO_CHNL_T::const_iterator it(
						m_dbServIdChnls.find(itS->second));
					if(m_dbServIdChnls.end() == it) {
						bReroute = true;
					}
				}
				if(bReroute) {
					outChannel = m_lessServQueue.Peek();
					bUpdate = true;
				}	
			}
		} else {
			outChannel = m_lessServQueue.Peek();
			bUpdate = true;
		}	
	}
	return bUpdate;
}

void CWorkerModule::UpdateChnlByBalServId(uint16_t serverId,
	util::CAutoPointer<IChannelValue>& channel)
{
	if(ID_NULL == serverId) {
		OutputError("ID_NULL == serverId");
		assert(false);
		return;
	}
	uint16_t nOldServId = ID_NULL;
	if(true) {
		CScopedWriteLock writeLock(m_rwBalServer);
		if(channel.IsInvalid()) {
			if(!m_balServIdChnls.empty()) {
				m_balServIdChnls.clear();
			}
		} else {
			util::CAutoPointer<CWorkPQItem> pServPQItem(
				channel->GetServerPQItem());
			if(pServPQItem.IsInvalid()) {
				assert(false);
			} else {
				pServPQItem->AtomicIncCount();
				m_lessServQueue.Update(pServPQItem->GetIndex());
				if(true) {
					CScopedReadLock rDbLock(m_rwDbChnls);
					DBSERVID_TO_CHNL_T::iterator it(m_dbServIdChnls.find(
						channel->GetServerId()));
					if(m_dbServIdChnls.end() != it) {
						it->second.SetServCount(pServPQItem->GetCount());
					}
				}
			}

			std::pair<BALSERVID_TO_CHNL_T::iterator, bool> pairIB(
				m_balServIdChnls.insert(BALSERVID_TO_CHNL_T
				::value_type(serverId, channel->GetServerId())));
			if(!pairIB.second) {
				nOldServId = pairIB.first->second;
				pairIB.first->second = channel->GetServerId();
			}
		}
	}
	if(ID_NULL != nOldServId) {
		CScopedReadLock rDirLock(m_rwDirServer);
		DIRSERVID_TO_CHNL_T::const_iterator itD(
			m_dirServIdChnls.find(nOldServId));
		if(m_dirServIdChnls.end() != itD) {
			util::CAutoPointer<CWorkPQItem> pServPQItem(itD->second);
			if(pServPQItem.IsInvalid()) {
				assert(false);
			} else {
				pServPQItem->AtomicDecCount();
				m_lessServQueue.Update(pServPQItem->GetIndex());
				if(true) {
					CScopedReadLock rDbLock(m_rwDbChnls);
					DBSERVID_TO_CHNL_T::iterator it(m_dbServIdChnls.find(nOldServId));
					if(m_dbServIdChnls.end() != it) {
						it->second.SetServCount(pServPQItem->GetCount());
					}
				}
			}
		} else {
			CScopedReadLock rDbLock(m_rwDbChnls);
			DBSERVID_TO_CHNL_T::iterator it(m_dbServIdChnls.find(nOldServId));
			if(m_dbServIdChnls.end() != it) {
				it->second.AtomicDecServCount();
			}
		}
	}
}

void CWorkerModule::RemoveBalServId(uint16_t serverId)
{
	if(ID_NULL == serverId) {
		OutputError("ID_NULL == serverId");
		assert(false);
		return;
	}
	CScopedReadLock readLock(m_rwDirServer);
	if(true) {
		CScopedWriteLock writeLock(m_rwBalServer);

		BALSERVID_TO_CHNL_T::const_iterator itS(
			m_balServIdChnls.find(serverId));
		if(m_balServIdChnls.end() != itS) {
			DIRSERVID_TO_CHNL_T::const_iterator itD(
				m_dirServIdChnls.find(itS->second));	
			if(m_dirServIdChnls.end() != itD) {
				util::CAutoPointer<CWorkPQItem> pServPQItem(
					itD->second->GetServerPQItem());
				if(!pServPQItem.IsInvalid()) {
					pServPQItem->AtomicDecCount();
					m_lessServQueue.Update(pServPQItem->GetIndex());
					if(true) {
						CScopedReadLock rDbLock(m_rwDbChnls);
						DBSERVID_TO_CHNL_T::iterator it(m_dbServIdChnls.find(
							itS->second));
						if(m_dbServIdChnls.end() != it) {
							it->second.SetServCount(pServPQItem->GetCount());
						}
					}
				}
			}
			m_balServIdChnls.erase(itS);
		}
	}
}

void CWorkerModule::RemoveInvalidServId()
{
	CScopedReadLock rDirLock(m_rwDirServer);
	if(true) {
		CScopedWriteLock wDbLock(m_rwDbChnls);
		DBSERVID_TO_CHNL_T::iterator it(m_dbServIdChnls.begin());
		while(m_dbServIdChnls.end() != it) {
			if(m_dirServIdChnls.end() == m_dirServIdChnls.find(it->first)) {
				m_dbServIdChnls.erase(it++);
			} else {
				++it;
			}
		}
	}
}

void CWorkerModule::HandleLogout(const util::CWeakPointer<::node::DataPacket>& pDpRequest,
	util::CWeakPointer<::node::DataPacket>& pDpResponse)
{
	HandleDefault(pDpRequest, pDpResponse);
	uint64_t userId = pDpRequest->route();
    RemoveBalUserId(userId);
}

void CWorkerModule::HandleDefault(const util::CWeakPointer<::node::DataPacket>& pDpRequest,
	util::CWeakPointer<::node::DataPacket>& pDpResponse)
{
	uint64_t userId = pDpRequest->route();

	CAutoPointer<IChannelValue> rpcChannel;
	if(GetChnlByBalUserId(userId, rpcChannel)) {
		UpdateChnlByBalUserId(userId, rpcChannel);
	}
	if(rpcChannel.IsInvalid()) {
		OutputError("No Channel!");
		if(!pDpResponse.IsInvalid()) {
			pDpResponse->set_result(FALSE);
		}
		return;
	}

	::node::WorkerService_Stub workerService_stub(&*rpcChannel->GetRpcChannel(), false);

	rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);
	::node::DataPacket dpResponse;
	workerService_stub.HandleProtocol(*pDpRequest, &dpResponse, &controller, NULL);
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

