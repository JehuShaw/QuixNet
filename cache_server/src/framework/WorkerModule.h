/* 
 * File:   WorkerModule.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 11:25
 */

#ifndef WORKERMODULE_H
#define	WORKERMODULE_H

#include "TPriorityQueue.h"
#include "worker.rpcz.h"
#include "IChannelControl.h"
#include "IChannelValue.h"


class CWorkerModule : public IChannelControl
{
public:
	CWorkerModule(const std::string& moduleName);

	CWorkerModule(
		const std::string& moduleName,
		const std::string& endPoint,
		uint32_t serverId,
		bool routeServer,
		uint64_t routeAddressId,
		const ROUTE_USERIDS_T& routeUserIds);

	~CWorkerModule(void);

	virtual void OnRegister();

	virtual void OnRemove();

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual util::CAutoPointer<mdl::IObserverRestricted> FullProtocolInterests();

	virtual void HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	virtual bool CreatChannel(uint32_t serverId, const std::string& endPoint, uint16_t serverType,
		bool routeServer, uint64_t routeAddressId, const ROUTE_USERIDS_T& routeUserIds);

	virtual bool RemoveChannel(uint32_t serverId);

	virtual int ChannelCount() const;

	virtual void IterateChannel(std::vector<util::CAutoPointer<IChannelValue> >& outChannels) const;

	virtual util::CAutoPointer<IChannelValue> GetChnlByDirServId(uint32_t serverId) const;

protected:
	void HandleMessage(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void HandleDefault(const util::CWeakPointer<::node::DataPacket>& pDpRequest,
		util::CWeakPointer<::node::DataPacket>& pDpResponse);

	void HandleLogout(const util::CWeakPointer<::node::DataPacket>& pDpRequest,
		util::CWeakPointer<::node::DataPacket>& pDpResponse);

	util::CAutoPointer<IChannelValue> GetChnlByBalUserId(uint64_t userId);

	void RemoveBalUserId(uint64_t userId, bool bRemoveRoute = true);

	util::CAutoPointer<IChannelValue> GetChnlByBalServId(uint32_t serverId);

	util::CAutoPointer<IChannelValue> GetFirstChnl() const;

	void UpdateRouteUserIds(uint32_t serverId, const ROUTE_USERIDS_T& routeUserIds);

	void ClearRouteUserIds(uint32_t serverId);

protected:
	util::CAutoPointer<std::vector<int> > m_notifications;
	thd::CCriticalSection m_notifMutex;
	util::CAutoPointer<std::vector<int> > m_protocols;
	thd::CCriticalSection m_protoMutex;

	typedef std::map<uint64_t, uint32_t> BALUERID_TO_SERVID_T;
	BALUERID_TO_SERVID_T m_balUserIds;
	thd::CSpinRWLock m_rwBalUser;

	volatile uint32_t m_dirServerId;

	typedef std::map<uint32_t, util::CAutoPointer<IChannelValue> > DIRSERVID_TO_CHNL_T;
	DIRSERVID_TO_CHNL_T m_dirServIdChnls;
	thd::CSpinRWLock m_rwDirServer;
};

#endif /* WORKERMODULE_H */