/*
 * File:   ControlCentreStubImpEx.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef CONTROLCENTRESTUBIMPEX_H
#define CONTROLCENTRESTUBIMPEX_H

#include <map>
#include "controlcentre.rpcz.h"
#include "rpc_controller.hpp"
#include "ChannelManager.h"
#include "NodeDefines.h"
#include "AgentMethod.h"
#include "SpinRWLock.h"
#include "json/json.h"
#include "Singleton.h"
#include "Log.h"

class CControlCentreStubImpEx
	: public util::Singleton<CControlCentreStubImpEx>
{
public:
	CControlCentreStubImpEx(void) {}

	int RegisterServer(
		const std::string& connect,
		const std::string& serverName,
		const std::string& endPoint,
		uint32_t serverId,
		uint16_t serverRegion,
		const std::string& projectName,
        const std::string& acceptAddress,
		const std::string& processPath,
		int deadline_ms = CALL_DEADLINE_MS)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
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
		request.set_projectname(projectName);
        if(!acceptAddress.empty()) {
            request.set_acceptaddress(acceptAddress);
        }
		if(!processPath.empty()) {
			request.set_processpath(processPath);
		}
		request.set_agentsize((uint32_t)-1);

		::node::RegisterResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(deadline_ms);
		controlCentre_stub.RegisterModule(request, &response, &controller, NULL);
		controller.wait();
		return response.result();
	}

	int UnregisterServer(
		const std::string& connect,
		const std::string& serverName,
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

		request.set_servername(serverName);
		request.set_serverid(serverId);
		request.set_servertype(NODE_REGISTER_TYPE);

		::node::RemoveResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(deadline_ms);
		controlCentre_stub.RemoveModule(request, &response, &controller, NULL);
		controller.wait();
		return response.result();
	}

	int KeepRegister(
		const std::string& connect,
		const std::string& serverName,
		uint32_t serverId,
		int32_t serverStatus,
		uint32_t serverLoad)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return CSR_FAIL;
		}

		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		::node::KeepRegisterRequest request;

		request.set_servername(serverName);
		request.set_serverid(serverId);
		request.set_servertype(NODE_REGISTER_TYPE);
		request.set_serverstatus(serverStatus);
		request.set_serverload(serverLoad);
		std::string strState;
		GetState(strState);
		request.set_serverstate(strState);
		request.set_agentsize((uint32_t)-1);

		::node::KeepRegisterResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.KeepRegister(request, &response, &controller, NULL);
		controller.wait();
		if(controller.ok()) {
			return response.result();
		}
		return CSR_TIMEOUT;
	}

	inline void GetLowLoadNode(
		const std::string& inConnect,
		const std::string& inServerName,
		uint16_t& outServerRegion,
		std::string* pOutAcceptAddress,
		std::string* pOutEndPoint)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(inConnect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", inConnect.c_str());
			return;
		}

		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		::node::LowLoadNodeRequest request;
		request.set_servername(inServerName);
		::node::LowLoadNodeResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.GetLowLoadNode(request, &response, &controller, NULL);
		controller.wait();
		outServerRegion = response.serverregion();
		if(NULL != pOutAcceptAddress) {
			*pOutAcceptAddress = response.acceptaddress();
		}
		if(NULL != pOutEndPoint) {
			*pOutEndPoint = response.endpoint();
		}
	}

	inline void GetLowLoadByRegion(
		const std::string& inConnect,
		const std::string& inServerName,
		const uint16_t inServerRegion,
		std::string* pOutAcceptAddress,
		std::string* pOutEndPoint)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(inConnect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", inConnect.c_str());
			return;
		}

		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		::node::RegionLowLoadRequest request;
		request.set_servername(inServerName);
		request.set_serverregion(inServerRegion);
		::node::RegionLowLoadResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.GetRegionLowLoad(request, &response, &controller, NULL);
		controller.wait();
		if(NULL != pOutAcceptAddress) {
			*pOutAcceptAddress = response.acceptaddress();
		}
		if(NULL != pOutEndPoint) {
			*pOutEndPoint = response.endpoint();
		}
	}

    inline void UserLogin(
        const std::string& connect,
        const std::string& serverName,
        uint32_t serverId,
        uint64_t userId)
    {
        CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return;
		}

        ::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
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
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return;
		}

        ::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
        ::node::UserLogoutRequest request;
        request.set_servername(serverName);
        request.set_userid(userId);
        ::node::ControlCentreVoid response;
        rpcz::rpc_controller controller;
        controller.set_deadline_ms(CALL_DEADLINE_MS);
        controlCentre_stub.UserLogout(request, &response, &controller, NULL);
        controller.wait();
    }

	inline eServerError GetUsers(
		const std::string& connect,
		uint64_t account,
		::node::GetUserResponse& outResponse)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return SERVER_ERROR_NOTFOUND_CHANNEL;
		}
		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		::node::GetUserRequest request;
		request.set_account(account);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.GetUsers(request, &outResponse, &controller, NULL);
		controller.wait();

		return (eServerError)outResponse.result();
	}

	inline eServerError CreateUser(
		const std::string& connect,
		uint64_t account,
		::node::CreateUserResponse& outResponse,
		uint32_t mapId,
		uint32_t maxSize = 0)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return SERVER_ERROR_NOTFOUND_CHANNEL;
		}

		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		
		::node::CreateUserRequest inRequest;
		inRequest.set_account(account);
		inRequest.set_mapid(mapId);
		inRequest.set_maxsize(maxSize);
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.CreateUser(inRequest, &outResponse, &controller, NULL);
		controller.wait();

		return (eServerError)outResponse.result();
	}

	inline eServerError CheckUser(
		const std::string& connect,
		uint64_t userId,
		::node::CheckUserResponse& outResponse)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return SERVER_ERROR_NOTFOUND_CHANNEL;
		}
		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		::node::CheckUserRequest request;
		request.set_userid(userId);
		
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.CheckUser(request, &outResponse, &controller, NULL);
		controller.wait();

		return (eServerError)outResponse.result();
	}

	inline eServerError UpdateUser(
		const std::string& connect,
		uint64_t userId,
		uint32_t mapId,
		uint32_t serverRegion = 0,
		bool bLogin = false)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return SERVER_ERROR_NOTFOUND_CHANNEL;
		}

		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		::node::UpdateUserRequest request;
		request.set_userid(userId);
		request.set_mapid(mapId);
		request.set_serverregion(serverRegion);
		request.set_login(bLogin);
		::node::UpdateUserResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.UpdateUser(request, &response, &controller, NULL);
		controller.wait();

		return (eServerError)response.result();
	}

	inline eServerError DeleteUser(
		const std::string& connect,
		uint64_t userId)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return SERVER_ERROR_NOTFOUND_CHANNEL;
		}
		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		::node::DeleteUserRequest request;
		request.set_userid(userId);
		::node::DeleteUserResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.DeleteUser(request, &response, &controller, NULL);
		controller.wait();

		return (eServerError)response.result();
	}

	inline std::string GetEndPointFromServant(
		const std::string& connect,
		uint32_t serverId)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return std::string();
		}
		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);

		::node::EndPointRequest request;
		request.set_serverid(serverId);
		::node::EndPointResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.GetEndPointFromServant(request, &response, &controller, NULL);
		controller.wait();

		return response.endpoint();
	}

	inline std::string SeizeServerReturnIP(
		const std::string& connect,
		const std::string& serverName,
		uint64_t userId,
		bool bLogin)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return std::string();
		}
		
		::node::SeizeRequest request;
		request.set_userid(userId);
		request.set_servername(serverName);
		request.set_login(bLogin);

		::node::SeizeResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		controlCentre_stub.SeizeServer(request, &response, &controller, NULL);
		controller.wait();

		return response.acceptaddress();
	}

	inline void SeizeServerReturnID(
		uint32_t& outAgentId,
		uint32_t& outMapId,
		const std::string& connect,
		const std::string& serverName,
		uint64_t userId,
		bool bLogin)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return;
		}

		::node::SeizeRequest request;
		request.set_userid(userId);
		request.set_servername(serverName);
		request.set_login(bLogin);

		::node::SeizeResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		controlCentre_stub.SeizeServer(request, &response, &controller, NULL);
		controller.wait();

		outAgentId = response.serverid();
		outMapId = response.mapid();
	}

	inline eServerError FreeServer(
		const std::string& connect,
		uint64_t userId,
		bool bLogout)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return SERVER_ERROR_NOTFOUND_CHANNEL;
		}

		::node::FreeRequest request;
		request.set_userid(userId);
		request.set_logout(bLogout);

		::node::FreeResponse freeResponse;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		controlCentre_stub.FreeServer(request, &freeResponse, &controller, NULL);
		controller.wait();

		return (eServerError)freeResponse.result();
	}

	inline uint64_t GenerateGuid(const std::string& connect)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return ID_NULL;
		}

		::node::ControlCentreVoid request;
		::node::GuidResponse guidResponse;

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		::node::ControlCentreService_Stub controlCentre_stub(pChannel.operator->(), false);
		controlCentre_stub.GenerateGuid(request, &guidResponse, &controller, NULL);
		controller.wait();

		return guidResponse.id();
	}

	/////////////////////////////////////////////////////////////////////////////
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

#endif /* CONTROLCENTRESTUBIMPEX_H */