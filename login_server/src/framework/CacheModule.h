/*
 * File:   CacheModule.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#pragma once

#include "cache.rpcz.h"
#include "IChannelControl.h"
#include "IChannelValue.h"
#include "ArrayMap.h"


class CCacheModule : public IChannelControl
{
public:
	CCacheModule(void);

	CCacheModule(const std::string& moduleName,
		const std::string& endPoint,
		uint16_t serverId);

	CCacheModule(const std::string& moduleName,
		const std::string& endPoint,
		uint16_t serverId,
		const std::string& acceptAddress,
		const std::string& processPath,
		const std::string& projectName,
		uint16_t serverRegion);

	~CCacheModule(void);

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

	virtual int ChannelCount() const;

	virtual void IterateChannel(std::vector<util::CAutoPointer<IChannelValue> >& outChannels) const;

	virtual util::CAutoPointer<IChannelValue> GetLowLoadUserChnl() const { return util::CAutoPointer<IChannelValue>(); }

	virtual util::CAutoPointer<IChannelValue> GetChnlByDirServId(uint16_t serverId) const;

protected:
	virtual bool FilterProtocolInterest(int nProtocal) const { return false; }

	util::CAutoPointer<IChannelValue> GetChnlByBalUserId(uint64_t userId) const;

	util::CAutoPointer<IChannelValue> GetChnlByBalServId(uint16_t serverId) const;

    inline util::CAutoPointer<IChannelValue> GetFirstChnl() const {
        thd::CScopedReadLock readLock(m_rwServer);
        if(m_serverIdChnls.Size() == 0) {
            return util::CAutoPointer<IChannelValue>();
        }
        return m_serverIdChnls[0];
    }

	void HandleMessage(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	static int ServidArrMapComparison(const uint16_t& a, const util::CAutoPointer<IChannelValue>& b);

protected:
	util::CAutoPointer<std::vector<int> > m_notifications;
	thd::CCriticalSection m_notifMutex;
	util::CAutoPointer<std::vector<int> > m_protocols;
	thd::CCriticalSection m_protoMutex;

	typedef util::CArrayMap<uint16_t, util::CAutoPointer<IChannelValue>, ServidArrMapComparison> SERVERID_TO_CHNL_T;
	SERVERID_TO_CHNL_T m_serverIdChnls;
	thd::CSpinRWLock m_rwServer;
};

