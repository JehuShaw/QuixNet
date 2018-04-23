/* 
 * File:   ServantModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_4_28, 13:15
 */

#ifndef SERVANTMODULE_H
#define	SERVANTMODULE_H

#include "Common.h"

#include "TPriorityQueue.h"
#include "worker.rpcz.h"
#include "NodeChannel.h"
#include "ModuleManager.h"
#include "IChannelControl.h"

class CServantModule: public mdl::CModule
{
public:
	CServantModule(const char* name, uint16_t serverId);
	~CServantModule(void);

	virtual void OnRegister();

	virtual void OnRemove();

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual void HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

private:
	static int SendCacheMessage(
		util::CAutoPointer<IChannelValue>& rpcChannel,
		const ::node::DataPacket& request,
		::node::DataPacket& response);

private:
    void CaseTransport(const util::CWeakPointer<mdl::INotification>& request,
        util::CWeakPointer<mdl::IResponse>& reply);
	void CaseRestart(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);
	void CaseShutdown(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);
	void CaseErase(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);
	void CaseCacheStore(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);
	void CasePlay(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);
	void CaseStop(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

private:
	uint16_t m_serverId;
};


#endif	/* SERVANTMODULE_H */

