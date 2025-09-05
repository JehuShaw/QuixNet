/* 
 * File:   ControlCentreStubImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef CONTROLCENTRESTUBIMP_H
#define CONTROLCENTRESTUBIMP_H

#include "controlcentre.rpcz.h"
#include "ChannelManager.h"
#include "rpc_controller.hpp"
#include "NodeDefines.h"
#include "Singleton.h"
#include "Log.h"

class CControlCentreStubImp
	: public util::Singleton<CControlCentreStubImp>
{
public:
	CControlCentreStubImp(void) {}

	int RegisterServer(
		const std::string& connect, 
		const std::string& serverName,
		const std::string& endPoint,
		uint32_t serverId,
		uint16_t serverRegion,
		int deadline_ms = CALL_DEADLINE_MS)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<CRpczChannel> pRpczChannel(pChlMgr->GetRpczChannelEx(connect));
		if (pRpczChannel.IsInvalid()) {
			OutputError("pRpczChannel.IsInvalid() %s ", connect.c_str());
			return CSR_FAIL;
		}
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pRpczChannel->GetChannel());
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return CSR_FAIL;
		}

		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		::node::RegisterRequest request;
		
		request.set_servername(serverName);
		request.set_endpoint(endPoint);
		request.set_serverid(serverId);
		request.set_serverregion(serverRegion);
		request.set_servertype(NODE_REGISTER_TYPE);
		request.set_agentsize(pRpczChannel->GetAgentSize());
		uint64_t routeAddressId = AddressToInteger(connect.c_str());
		request.set_routeserver(pChlMgr->ExistRouteServer(routeAddressId));
		request.set_routeaddressid(routeAddressId);
		pChlMgr->IteratorClient(request.mutable_routeuserids(), connect);

		::node::RegisterResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(deadline_ms);
		controlCentre_stub.RegisterModule(request, &response, &controller, NULL);
		controller.wait();

		int nAgSize = response.agentids_size();
		if (nAgSize > 0) {
			pRpczChannel->SetAgentSize(nAgSize);
			pChlMgr->AddAgentChannel(response.agentids(), connect);
		}

		return response.result();
	}

	int UnregisterServer(
		const std::string& connect, 
		const std::string& name,
		uint32_t serverId,
		int deadline_ms = CALL_DEADLINE_MS)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return CSR_FAIL;
		}

		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		::node::RemoveRequest request;
		request.set_servername(name);
		request.set_serverid(serverId);
		request.set_servertype(NODE_REGISTER_TYPE);

		::node::RemoveResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(deadline_ms);
		controlCentre_stub.RemoveModule(request, &response, &controller, NULL);
		controller.wait();

		return response.result();
	}

	int KeepRegister(const std::string& connect, uint32_t serverId)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		util::CAutoPointer<CRpczChannel> pRpczChannel(pChlMgr->GetRpczChannelEx(connect));
		if (pRpczChannel.IsInvalid()) {
			OutputError("pRpczChannel.IsInvalid() %s ", connect.c_str());
			return CSR_FAIL;
		}
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pRpczChannel->GetChannel());
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return CSR_FAIL;
		}
		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);

		::node::KeepRegisterRequest request;
		request.set_serverid(serverId);
		request.set_servertype(NODE_REGISTER_TYPE);
		request.set_agentsize(pRpczChannel->GetAgentSize());

		::node::KeepRegisterResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.KeepRegister(request, &response, &controller, NULL);
		controller.wait();

		int nAgSize = response.agentids_size();
		if (nAgSize > 0) {
			pRpczChannel->SetAgentSize(nAgSize);
			pChlMgr->AddAgentChannel(response.agentids(), connect);
		}

		if(controller.ok()) {
			return response.result();
		}
		return CSR_TIMEOUT;
	}
};

#endif /* CONTROLCENTRESTUBIMP_H */