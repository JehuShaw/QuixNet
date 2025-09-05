/*
 * File:   CacheModule.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef CACHEMODULE_H
#define	CACHEMODULE_H


#include "cache.rpcz.h"
#include "IChannelControl.h"
#include "IChannelValue.h"


class CCacheModule : public IChannelControl
{
public:
	CCacheModule(
		const std::string& moduleName,
		const std::string& endPoint,
		uint32_t serverId,
		bool routeServer,
		uint64_t routeAddressId,
		const ROUTE_USERIDS_T& routeUserIds);

	~CCacheModule(void);

	virtual void OnRegister();

	virtual void OnRemove();

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual void HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	virtual bool CreatChannel(uint32_t serverId, const std::string& endPoint, uint16_t serverType,
		bool routeServer, uint64_t routeAddressId, const ROUTE_USERIDS_T& routeUserIds);

	virtual bool RemoveChannel(uint32_t serverId);

	virtual int ChannelCount() const;

	virtual void IterateChannel(std::vector<util::CAutoPointer<IChannelValue> >& outChannels) const;

	virtual util::CAutoPointer<IChannelValue> GetChnlByDirServId(uint32_t serverId) const;

protected:
	virtual bool FilterProtocolInterest(int nProtocal) const { return false; }

	util::CAutoPointer<IChannelValue> GetChnlByBalUserId(uint64_t userId);

	util::CAutoPointer<IChannelValue> GetChnlByBalServId(uint32_t serverId);

	void RemoveBalUserId(uint64_t userId, bool bRemoveRoute = true);

	util::CAutoPointer<IChannelValue> GetFirstChnl() const;

	util::CAutoPointer<IChannelValue> GetChnlByHash32Key(uint32_t u32Key);

	void UpdateRouteUserIds(uint32_t serverId, const ROUTE_USERIDS_T& routeUserIds);

	void ClearRouteUserIds(uint32_t serverId);

	void HandleMessage(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void HandleDefault(const util::CWeakPointer<::node::DataPacket>& pDpRequest,
		util::CWeakPointer<::node::DataPacket>& pDpResponse);

	void HandleLogout(const util::CWeakPointer<::node::DataPacket>& pDpRequest,
		util::CWeakPointer<::node::DataPacket>& pDpResponse);

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

#endif /* CACHEMODULE_H */