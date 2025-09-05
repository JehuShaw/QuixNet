/* 
 * File:   AgentWorkerModule.h
 * Author: Jehu Shaw
 *
 * Created on 2020_4_26 11:25
 */

#ifndef AGENTWORKERMODULE_H
#define	AGENTWORKERMODULE_H

#include "WorkerModule.h"

class CAgentWorkerModule : public CWorkerModule
{
public:
	CAgentWorkerModule(const std::string& moduleName);

	CAgentWorkerModule(
		const std::string& moduleName,
		const std::string& endPoint,
		uint32_t serverId,
		bool routeServer,
		uint64_t routeAddressId,
		const ROUTE_USERIDS_T& routeUserIds);

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual util::CAutoPointer<mdl::IObserverRestricted> FullProtocolInterests();

	virtual void HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

protected:
	void HandleAgentMessage(
		const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void CaseCheckSwithMapID(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	static uint32_t GetBalMapID(uint64_t route);

protected:
	std::set<uint32_t> m_maps;
	thd::CSpinRWLock m_mapRwLock;
	static std::vector<uint32_t> s_allMaps;
	static thd::CSpinRWLock s_allMapRwLock;
};

#endif /* AGENTWORKERMODULE_H */