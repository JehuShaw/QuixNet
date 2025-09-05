/* 
 * File:   MasterModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef MASTERMODULE_H
#define	MASTERMODULE_H

#include "INodeControl.h"

class CMasterModule : public INodeControl 
{
public:
	CMasterModule(const char* name, uint32_t serverId,
		const std::string& endPoint, const std::string& acceptAddress,
		const std::string& processPath, const std::string& projectName);

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

	virtual int ChannelCount() const;

	virtual void IterateChannel(std::vector<util::CAutoPointer<IChannelValue> >& outChannels) const;

	virtual util::CAutoPointer<IChannelValue> GetLowLoadUserChnl() const;

	virtual util::CAutoPointer<IChannelValue> GetLowLoadByRegion(uint16_t serverRegion) const;

	virtual util::CAutoPointer<IChannelValue> GetChnlByDirServId(uint32_t serverId) const;

	virtual void UpdateChannelLoad(uint32_t serverId, int32_t serverLoad) {}

private:
	util::CAutoPointer<IChannelValue> m_channel;
	thd::CSpinRWLock m_rwLock;
};

#endif	/* MASTERMODULE_H */

