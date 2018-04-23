/* 
 * File:   ControlCentreStubImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#pragma once

#include "controlcentre.rpcz.h"
#include "ChannelManager.h"
#include "rpc_controller.hpp"
#include "NodeDefines.h"
#include "Singleton.h"

class CControlCentreStubImp
	: public util::Singleton<CControlCentreStubImp>
{
public:
	CControlCentreStubImp(void) {}

	int RegisterServer(const std::string& connect, 
		const std::string& serverName,
		const std::string& endPoint,
		uint16_t serverId,
		uint16_t serverRegion,
		int deadline_ms = CALL_DEADLINE_MS)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pChlMgr->GetRpczChannel(connect), false);
		::node::RegisterRequest request;
		
		request.set_servername(serverName);
		request.set_endpoint(endPoint);
		request.set_serverid(serverId);
		request.set_serverregion(serverRegion);
		request.set_servertype(NODE_REGISTER_TYPE);

		::node::OperateResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(deadline_ms);
		controlCentre_stub.RegisterModule(request, &response, &controller, NULL);
		controller.wait();
		return response.result();
	}

	int UnregisterServer(
		const std::string& connect, 
		const std::string& name,
		uint16_t serverId,
		int deadline_ms = CALL_DEADLINE_MS)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pChlMgr->GetRpczChannel(connect), false);
		::node::RemoveRequest request;

		request.set_servername(name);
		request.set_serverid(serverId);
		request.set_servertype(NODE_REGISTER_TYPE);

		::node::OperateResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(deadline_ms);
		controlCentre_stub.RemoveModule(request, &response, &controller, NULL);
		controller.wait();
		return response.result();
	}

	bool KeepRegister(const std::string& connect, uint16_t serverId)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pChlMgr->GetRpczChannel(connect), false);
		::node::KeepRegisterRequest request;

		request.set_serverid(serverId);
		request.set_servertype(NODE_REGISTER_TYPE);

		::node::KeepRegisterResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.KeepRegister(request, &response, &controller, NULL);
		controller.wait();
		if(controller.ok()) {
			return response.reregister();
		}
		return false;
	}

};

