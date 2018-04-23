/*
 * File:   ServantStubImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#pragma once

#include <map>
#include "controlcentre.rpcz.h"
#include "ChannelManager.h"
#include "rpc_controller.hpp"
#include "NodeDefines.h"
#include "AgentMethod.h"
#include "SpinRWLock.h"
#include "json/json.h"
#include "ModuleManager.h"
#include "NodeModule.h"
#include "Singleton.h"


class CServantStubImp
	: public util::Singleton<CServantStubImp>
{
public:
	int RegisterServer(const std::string& connect,
		const std::string& serverName,
		const std::string& endPoint,
		uint16_t serverId,
		uint16_t serverRegion,
		const std::string& projectName,
		const std::string& acceptAddress,
		const std::string& processPath,
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
		request.set_projectname(projectName);
		if(!acceptAddress.empty()) {
			request.set_acceptaddress(acceptAddress);
		}
		if(!processPath.empty()) {
			request.set_processpath(processPath);
		}
		::node::OperateResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(deadline_ms);
		controlCentre_stub.RegisterModule(request, &response, &controller, NULL);
		controller.wait();
		//////////////////////////////////////////////////////////////////////////
		RegisterAllNode(controlCentre_stub, endPoint, serverId, projectName, deadline_ms);
		return response.result();
	}

	int UnregisterServer(const std::string& connect,
		const std::string& serverName,
		uint16_t serverId,
		int deadline_ms = CALL_DEADLINE_MS)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pChlMgr->GetRpczChannel(connect), false);

		::node::RemoveRequest request;
		request.set_servername(serverName);
		request.set_serverid(serverId);
		request.set_servertype(NODE_REGISTER_TYPE);
		::node::OperateResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(deadline_ms);
		controlCentre_stub.RemoveModule(request, &response, &controller, NULL);
		controller.wait();
		//////////////////////////////////////////////////////////////////////////
		UnregisterAllNode(controlCentre_stub);
		return response.result();
	}

	bool KeepRegister(const std::string& connect,
		const std::string& serverName,
		const std::string& endPoint,
		uint16_t serverId,
		int32_t serverStatus,
		uint32_t serverLoad = 0)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pChlMgr->GetRpczChannel(connect), false);
		::node::KeepRegisterRequest request;
		request.set_servername(serverName);
		request.set_endpoint(endPoint);
		request.set_serverid(serverId);
		request.set_servertype(NODE_REGISTER_TYPE);
		request.set_serverload(serverLoad);
		request.set_serverstatus(serverStatus);
		std::string strState;
		GetState(strState);
		request.set_serverstate(strState);

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

    inline void UserLogin(
        util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
        const ::node::UserLoginRequest& request,
        ::node::ControlCentreVoid& response)
    {
        if(pMasterChannel.IsInvalid()) {
            return;
        }
        ::node::ControlCentreService_Stub controlCentre_stub(
            &*pMasterChannel, false);

        rpcz::rpc_controller controller;
        controller.set_deadline_ms(CALL_DEADLINE_MS);
        controlCentre_stub.UserLogin(request, &response, &controller, NULL);
        controller.wait();
    }

    inline void UserLogout(
        util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
        const ::node::UserLogoutRequest& request,
        ::node::ControlCentreVoid& response)
    {
        if(pMasterChannel.IsInvalid()) {
            return;
        }
        ::node::ControlCentreService_Stub controlCentre_stub(
            &*pMasterChannel, false);

        rpcz::rpc_controller controller;
        controller.set_deadline_ms(CALL_DEADLINE_MS);
        controlCentre_stub.UserLogout(request, &response, &controller, NULL);
        controller.wait();
    }

	inline void GetLowLoadNode(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::LowLoadNodeRequest& request,
		::node::LowLoadNodeResponse& response)
	{
		if(pMasterChannel.IsInvalid()) {
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pMasterChannel, false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.GetLowLoadNode(request, &response, &controller, NULL);
		controller.wait();
	}

	inline void GetRegionLowLoad(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::RegionLowLoadRequest& request,
		::node::RegionLowLoadResponse& response)
	{
		if(pMasterChannel.IsInvalid()) {
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pMasterChannel, false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.GetRegionLowLoad(request, &response, &controller, NULL);
		controller.wait();
	}

	inline void GetNodeList(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::NodeListRequest& request,
		::node::NodeListResponse& response)
	{
		if(pMasterChannel.IsInvalid()) {
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pMasterChannel, false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.GetNodeList(request, &response, &controller, NULL);
		controller.wait();
	}

	inline void CreateUserId(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::CreateIdRequest& request,
		::node::CreateIdResponse& response)
	{
		if(pMasterChannel.IsInvalid()) {
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pMasterChannel, false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.CreateUserId(request, &response, &controller, NULL);
		controller.wait();
	}

	inline void CheckUserId(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::CheckIdRequest& request,
		::node::CheckIdResponse& response)
	{
		if(pMasterChannel.IsInvalid()) {
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pMasterChannel, false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.CheckUserId(request, &response, &controller, NULL);
		controller.wait();
	}

	inline void UpdateUserRegion(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::UpdateRegionRequest& request,
		::node::UpdateRegionResponse& response)
	{
		if(pMasterChannel.IsInvalid()) {
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pMasterChannel, false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.UpdateUserRegion(request, &response, &controller, NULL);
		controller.wait();
	}

	inline eServerError CacheServerStore(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::CacheStoreRequest& request,
		::node::CacheStoreResponse& response)
	{
		if(pMasterChannel.IsInvalid()) {
			return SERVER_ERROR_NOTFOUND_CHANNEL;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pMasterChannel, false);
		
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.CacheServerStore(request, &response, &controller, NULL);
		controller.wait();

		return (eServerError)response.result();
	}

	inline void AddInfoMethod(std::string strName,
		util::CAutoPointer<evt::MethodRSBase> pInfoMethod) {

		thd::CScopedWriteLock scopedWriteLock(m_rwInfoLock);
		if(pInfoMethod.IsInvalid()) {
			return;
		}
		m_infoMethods[strName] = pInfoMethod;
	}

	inline bool RemoveInfoMethod(std::string strName) {

		thd::CScopedWriteLock scopedWriteLock(m_rwInfoLock);
		INFO_METHOD_MAP_T::iterator it = m_infoMethods.find(strName);
		if(m_infoMethods.end() != it) {
			m_infoMethods.erase(it);
			return true;
		}
		return false;
	}

	inline void ClearInfoMethod() {

		thd::CScopedWriteLock scopedWriteLock(m_rwInfoLock);
		m_infoMethods.clear();
	}

private:
	inline void GetState(std::string& outJson) {
		Json::Value json;
		GetJsonValue(json);
		if(json.empty()) {
			return;
		}
		Json::FastWriter fastWriter;
		outJson = fastWriter.write(json);
	}

	void GetJsonValue(Json::Value& json) {

		thd::CScopedReadLock scopedReadLock(m_rwInfoLock);
		INFO_METHOD_MAP_T::iterator it = m_infoMethods.begin();
		for(; m_infoMethods.end() != it; ++it) {
			json[it->first] = it->second->Invoke();
		}
	}

private:
	void RegisterAllNode(::node::ControlCentreService_Stub& controlCentre_stub, const std::string& servantAddress,
		uint16_t servantId, const std::string& projectName, int deadline_ms = CALL_DEADLINE_MS) {

		std::vector<util::CAutoPointer<mdl::IModule> > modules;
		mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
		pFacade->IterateModule(modules);
		std::vector<util::CAutoPointer<mdl::IModule> >::iterator itM = modules.begin();
		for(; modules.end() != itM; ++itM) {
			util::CAutoPointer<CNodeModule> pNodeModule(*itM);
			if(pNodeModule.IsInvalid()) {
				continue;
			}
			std::vector<util::CAutoPointer<IChannelValue> > channels;
			pNodeModule->IterateChannel(channels);
			std::vector<util::CAutoPointer<IChannelValue> >::iterator itC = channels.begin();
			for(; channels.end() != itC; ++itC) {
				util::CAutoPointer<IChannelValue> pNodeChannel(*itC);

				::node::RegisterRequest request;
				request.set_servertype(REGISTER_TYPE_NODE);
				request.set_servername(pNodeModule->GetModuleName());
				request.set_endpoint(servantAddress);
				request.set_serverid(pNodeChannel->GetServerId());
				request.set_serverregion(pNodeChannel->GetServerRegion());
				request.set_servantid(servantId);
				request.set_projectname(projectName);
				const std::string& strAcceptAddress = pNodeChannel->GetAcceptAddress();
				if(!strAcceptAddress.empty()) {
					request.set_acceptaddress(strAcceptAddress);
				}
				const std::string& strProcessPath = pNodeChannel->GetProcessPath();
				if(!strProcessPath.empty()) {
					request.set_processpath(strProcessPath);
				}
				::node::OperateResponse response;
				rpcz::rpc_controller controller;
				controller.set_deadline_ms(deadline_ms);
				controlCentre_stub.RegisterModule(request, &response, &controller, NULL);
				controller.wait();
			}
		}
	}

	void UnregisterAllNode(::node::ControlCentreService_Stub& controlCentre_stub) {
		std::vector<util::CAutoPointer<mdl::IModule> > modules;
		mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
		pFacade->IterateModule(modules);
		std::vector<util::CAutoPointer<mdl::IModule> >::iterator itM = modules.begin();
		for(; modules.end() != itM; ++itM) {
			util::CAutoPointer<CNodeModule> pNodeModule(*itM);
			if(pNodeModule.IsInvalid()) {
				continue;
			}
			std::vector<util::CAutoPointer<IChannelValue> > channels;
			pNodeModule->IterateChannel(channels);
			std::vector<util::CAutoPointer<IChannelValue> >::iterator itC = channels.begin();
			for(; channels.end() != itC; ++itC) {
				util::CAutoPointer<IChannelValue> pNodeChannel(*itC);

				::node::RemoveRequest request;
				request.set_servername(pNodeModule->GetModuleName());
				request.set_serverid(pNodeChannel->GetServerId());
				request.set_servertype(REGISTER_TYPE_NODE);
				::node::OperateResponse response;
				rpcz::rpc_controller controller;
				controller.set_deadline_ms(CALL_DEADLINE_MS);
				controlCentre_stub.RemoveModule(request, &response, &controller, NULL);
				controller.wait();
			}
		}
	}

private:
	typedef std::map<std::string, util::CAutoPointer<evt::MethodRSBase> > INFO_METHOD_MAP_T;
	INFO_METHOD_MAP_T m_infoMethods;
	thd::CSpinRWLock m_rwInfoLock;

};

