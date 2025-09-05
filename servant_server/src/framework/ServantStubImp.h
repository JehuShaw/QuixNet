/*
 * File:   ServantStubImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef SERVANTSTUBIMP_H
#define	SERVANTSTUBIMP_H

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
#include "Log.h"


class CServantStubImp
	: public util::Singleton<CServantStubImp>
{
public:
	int RegisterServer(const std::string& connect,
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

		::node::ControlCentreService_Stub controlCentre_stub(
			pChannel.operator->(), false);

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
		::node::RegisterResponse response;
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
		uint32_t serverId,
		int deadline_ms = CALL_DEADLINE_MS)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return CSR_FAIL;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			pChannel.operator->(), false);

		::node::RemoveRequest request;
		request.set_servername(serverName);
		request.set_serverid(serverId);
		request.set_servertype(NODE_REGISTER_TYPE);
		::node::RemoveResponse response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(deadline_ms);
		controlCentre_stub.RemoveModule(request, &response, &controller, NULL);
		controller.wait();
		//////////////////////////////////////////////////////////////////////////
		UnregisterAllNode(controlCentre_stub);
		return response.result();
	}

	int KeepRegister(const std::string& connect,
		const std::string& serverName,
		const std::string& endPoint,
		uint32_t serverId,
		int32_t serverStatus,
		uint32_t serverLoad = 0)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(connect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", connect.c_str());
			return CSR_FAIL;
		}

		::node::ControlCentreService_Stub controlCentre_stub(
			pChannel.operator->(), false);
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
			return response.result();
		}
		return CSR_TIMEOUT;
	}

    inline void UserLogin(
        util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
        const ::node::UserLoginRequest& request,
        ::node::ControlCentreVoid& response)
    {
        if(pMasterChannel.IsInvalid()) {
			OutputError("pMasterChannel.IsInvalid() ");
            return;
        }
        ::node::ControlCentreService_Stub controlCentre_stub(
			pMasterChannel.operator->(), false);

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
			OutputError("pMasterChannel.IsInvalid() ");
            return;
        }
        ::node::ControlCentreService_Stub controlCentre_stub(
			pMasterChannel.operator->(), false);

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
			OutputError("pMasterChannel.IsInvalid() ");
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			pMasterChannel.operator->(), false);

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
			OutputError("pMasterChannel.IsInvalid() ");
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			pMasterChannel.operator->(), false);

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
			OutputError("pMasterChannel.IsInvalid() ");
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			pMasterChannel.operator->(), false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.GetNodeList(request, &response, &controller, NULL);
		controller.wait();
	}

	inline void GetUsers(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::GetUserRequest& request,
		::node::GetUserResponse& response)
	{
		if (pMasterChannel.IsInvalid()) {
			OutputError("pMasterChannel.IsInvalid() ");
			response.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			pMasterChannel.operator->(), false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.GetUsers(request, &response, &controller, NULL);
		controller.wait();
	}

	inline void CreateUser(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::CreateUserRequest& request,
		::node::CreateUserResponse& response)
	{
		if(pMasterChannel.IsInvalid()) {
			OutputError("pMasterChannel.IsInvalid() ");
			response.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			pMasterChannel.operator->(), false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.CreateUser(request, &response, &controller, NULL);
		controller.wait();
	}

	inline void CheckUser(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::CheckUserRequest& request,
		::node::CheckUserResponse& response)
	{
		if(pMasterChannel.IsInvalid()) {
			OutputError("pMasterChannel.IsInvalid() ");
			response.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			pMasterChannel.operator->(), false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.CheckUser(request, &response, &controller, NULL);
		controller.wait();
	}

	inline void UpdateUser(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::UpdateUserRequest& request,
		::node::UpdateUserResponse& response)
	{
		if (pMasterChannel.IsInvalid()) {
			OutputError("pMasterChannel.IsInvalid() ");
			response.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			pMasterChannel.operator->(), false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.UpdateUser(request, &response, &controller, NULL);
		controller.wait();
	}

	inline void DeleteUser(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::DeleteUserRequest& request,
		::node::DeleteUserResponse& response)
	{
		if (pMasterChannel.IsInvalid()) {
			OutputError("pMasterChannel.IsInvalid() ");
			response.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			pMasterChannel.operator->(), false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.DeleteUser(request, &response, &controller, NULL);
		controller.wait();
	}

	inline void SeizeServer(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::SeizeRequest& request,
		::node::SeizeResponse& response)
	{
		if (pMasterChannel.IsInvalid()) {
			OutputError("pMasterChannel.IsInvalid() ");
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			pMasterChannel.operator->(), false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.SeizeServer(request, &response, &controller, NULL);
		controller.wait();
	}

	inline void FreeServer(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::FreeRequest& request,
		::node::FreeResponse& response)
	{
		if (pMasterChannel.IsInvalid()) {
			OutputError("pMasterChannel.IsInvalid() ");
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			pMasterChannel.operator->(), false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.FreeServer(request, &response, &controller, NULL);
		controller.wait();
	}

	inline void GenerateGuid(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
		const ::node::ControlCentreVoid& request,
		::node::GuidResponse& response)
	{
		if (pMasterChannel.IsInvalid()) {
			OutputError("pMasterChannel.IsInvalid() ");
			return;
		}
		::node::ControlCentreService_Stub controlCentre_stub(
			pMasterChannel.operator->(), false);

		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		controlCentre_stub.GenerateGuid(request, &response, &controller, NULL);
		controller.wait();
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
		uint32_t servantId, const std::string& projectName, int deadline_ms = CALL_DEADLINE_MS) {

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
				::node::RegisterResponse response;
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
				::node::RemoveResponse response;
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

#endif /* SERVANTSTUBIMP_H */