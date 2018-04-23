/*
 * File:   ControlCentreStubImpEx.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#pragma once

#include <map>
#include "controlcentre.rpcz.h"
#include "rpc_controller.hpp"
#include "ChannelManager.h"
#include "NodeDefines.h"
#include "AgentMethod.h"
#include "SpinRWLock.h"
#include "json/json.h"
#include "Singleton.h"

class CControlCentreStubImpEx
	: public util::Singleton<CControlCentreStubImpEx>
{
public:
	CControlCentreStubImpEx(void) {}

	int RegisterServer(
		const std::string& connect,
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
		return response.result();
	}

	int UnregisterServer(
		const std::string& connect,
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
		return response.result();
	}

	bool KeepRegister(
		const std::string& connect,
		const std::string& serverName,
		uint16_t serverId,
		int32_t serverStatus,
		uint32_t serverLoad)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pChlMgr->GetRpczChannel(connect), false);
		::node::KeepRegisterRequest request;

		request.set_servername(serverName);
		request.set_serverid(serverId);
		request.set_servertype(NODE_REGISTER_TYPE);
		request.set_serverstatus(serverStatus);
		request.set_serverload(serverLoad);
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

	inline void GetLowLoadNode(
		const std::string& inConnect,
		const std::string& inServerName,
		std::string& outAcceptAddress,
		uint16_t& outServerRegion)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pChlMgr->GetRpczChannel(inConnect), false);
		::node::LowLoadNodeRequest request;
		request.set_servername(inServerName);
		::node::LowLoadNodeResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.GetLowLoadNode(request, &response, &controller, NULL);
		controller.wait();
		outAcceptAddress = response.acceptaddress();
		outServerRegion = response.serverregion();
	}

	inline void GetLowLoadByRegion(
		const std::string& inConnect,
		const std::string& inServerName,
		const uint16_t inServerRegion,
		std::string& outAcceptAddress)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pChlMgr->GetRpczChannel(inConnect), false);
		::node::RegionLowLoadRequest request;
		request.set_servername(inServerName);
		request.set_serverregion(inServerRegion);
		::node::RegionLowLoadResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.GetRegionLowLoad(request, &response, &controller, NULL);
		controller.wait();
		outAcceptAddress = response.acceptaddress();
	}

    inline void UserLogin(
        const std::string& connect,
        const std::string& serverName,
        uint16_t serverId,
        uint64_t userId)
    {
        CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        ::node::ControlCentreService_Stub controlCentre_stub(
            &*pChlMgr->GetRpczChannel(connect), false);
        ::node::UserLoginRequest request;
        request.set_servername(serverName);
        request.set_serverid(serverId);
        request.set_userid(userId);
        ::node::ControlCentreVoid response;
        rpcz::rpc_controller controller;
        controller.set_deadline_ms(CALL_DEADLINE_MS);
        controlCentre_stub.UserLogin(request, &response, &controller, NULL);
        controller.wait();
    }

    inline void UserLogout(
        const std::string& connect,
        const std::string& serverName,
        uint64_t userId)
    {
        CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        ::node::ControlCentreService_Stub controlCentre_stub(
            &*pChlMgr->GetRpczChannel(connect), false);
        ::node::UserLogoutRequest request;
        request.set_servername(serverName);
        request.set_userid(userId);
        ::node::ControlCentreVoid response;
        rpcz::rpc_controller controller;
        controller.set_deadline_ms(CALL_DEADLINE_MS);
        controlCentre_stub.UserLogout(request, &response, &controller, NULL);
        controller.wait();
    }

	inline eServerError CreateUserId(
		const std::string& connect,
		uint64_t account,
		uint64_t& outUserId,
		uint32_t& outServerRegion,
		std::string& outCreateTime)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pChlMgr->GetRpczChannel(connect), false);
		::node::CreateIdRequest request;
		request.set_account(account);
		::node::CreateIdResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.CreateUserId(request, &response, &controller, NULL);
		controller.wait();
		outUserId = response.userid();
		outServerRegion = response.serverregion();
		outCreateTime = response.createtime();

		return (eServerError)response.result();
	}

	inline eServerError CheckUserId(
		const std::string& connect,
		uint64_t account,
		uint64_t& outUserId,
		std::string& outCreateTime,
		uint32_t& outServerRegion,
		uint64_t& outCas)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pChlMgr->GetRpczChannel(connect), false);
		::node::CheckIdRequest request;
		request.set_account(account);
		::node::CheckIdResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.CheckUserId(request, &response, &controller, NULL);
		controller.wait();
		outUserId = response.userid();
		outCreateTime = response.createtime();
		outServerRegion = response.serverregion();
		outCas = response.cas();

		return (eServerError)response.result();
	}

	inline eServerError UpdateUserRegion(
		const std::string& connect,
		uint64_t account,
		uint32_t serverRegion,
		uint64_t cas)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pChlMgr->GetRpczChannel(connect), false);
		::node::UpdateRegionRequest request;
		request.set_account(account);
		request.set_serverregion(serverRegion);
		request.set_cas(cas);
		::node::UpdateRegionResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.UpdateUserRegion(request, &response, &controller, NULL);
		controller.wait();

		return (eServerError)response.result();
	}

	inline eServerError CacheServerStore(
		const std::string& connect,
		const ::node::CacheStoreRequest& request)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		::node::ControlCentreService_Stub controlCentre_stub(
			&*pChlMgr->GetRpczChannel(connect), false);

		::node::CacheStoreResponse response;
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
	typedef std::map<std::string, util::CAutoPointer<evt::MethodRSBase> > INFO_METHOD_MAP_T;
	INFO_METHOD_MAP_T m_infoMethods;
	thd::CSpinRWLock m_rwInfoLock;
};

