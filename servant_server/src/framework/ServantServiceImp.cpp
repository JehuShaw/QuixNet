#include "ServantServiceImp.h"
#include "ModuleManager.h"
#include "BodyMessage.h"
#include "NodeDefines.h"
#include "SpinLock.h"
#include "NodeModule.h"
#include <assert.h>
#include "rpc_channel.hpp"
#include "rpc_controller.hpp"
#include "ServantStubImp.h"
#include "TimerManager.h"

using namespace mdl;
using namespace util;
using namespace evt;

CServantServiceImp::SERVER_ID_SET_T CServantServiceImp::m_serverIds;
thd::CSpinRWLock CServantServiceImp::m_wrLock;

CServantServiceImp::CServantServiceImp(
	CAutoPointer<rpcz::rpc_channel> pMasterChannel,
	const std::string& endPoint, uint32_t servantId)
	: m_pMasterChannel(pMasterChannel)
	, m_endPoint(endPoint)
	, m_servantId(servantId)
{
	assert(!m_pMasterChannel.IsInvalid());
}

void CServantServiceImp::RegisterModule(const ::node::RegisterRequest& request,
	::rpcz::reply< ::node::RegisterResponse> response)
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	// read data
	uint32_t serverId = request.serverid();
	if(!InsertServerId(serverId)) {
		::node::RegisterResponse registerResponse;
		registerResponse.set_result(CSR_CHANNEL_ALREADY_EXIST);
		response.send(registerResponse);
		return;
	}

	int32_t serverType = request.servertype();
	std::string serverName(request.servername());
	std::string endPoint(request.endpoint());
	std::string projectName(request.projectname());
    std::string acceptAddress(request.acceptaddress());

	std::string processPath(request.processpath());
	uint16_t serverRegion = (uint16_t)request.serverregion();

	CAutoPointer<IModule> module(pFacade->RetrieveModule(serverName));
	if(module.IsInvalid()) {
		switch(serverType) {
		case REGISTER_TYPE_CACHE:
		case REGISTER_TYPE_WORKER:
		case REGISTER_TYPE_NODE:
		case REGISTER_TYPE_SERVANT:
			module.SetRawPointer(new CNodeModule(serverName, endPoint, serverId,
				serverType, acceptAddress, processPath, projectName, serverRegion));
			break;
		default:
			break;
		};

		if(module.IsInvalid()) {
			::node::RegisterResponse registerResponse;
			registerResponse.set_result(CSR_WITHOUT_REGISTER_TYPE);
			response.send(registerResponse);
			EraseServerId(serverId);
			return;
		}
		// send data
		pFacade->RegisterModule(module);
	} else {
		CAutoPointer<IChannelControl> pChannelModule(module);
		if(pChannelModule.IsInvalid()) {
			::node::RegisterResponse registerResponse;
			registerResponse.set_result(CSR_NO_ICHANNELCONTROL);
			response.send(registerResponse);
			EraseServerId(serverId);
			return;
		}

		if(!pChannelModule->CreatChannel(serverId, endPoint, serverType,
			acceptAddress, processPath, projectName, serverRegion)) 
		{
			// register to control master
			int nResult = RegisterToMaster(m_endPoint, m_servantId, serverName,
				serverId, serverRegion, projectName, acceptAddress, processPath);
			// send result
			::node::RegisterResponse registerResponse;
			registerResponse.set_result(nResult);
			response.send(registerResponse);
			////////////////////////////////////////////////////////////////////
			CAutoPointer<CallBackFuncP1<uint32_t> >
				callback(new CallBackFuncP1<uint32_t> 
				(&CServantServiceImp::KeepTimeoutCallback, serverId));

			CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
			pTMgr->SetTimeout(serverId, KEEP_REGISTER_TIMEOUT, callback);
			return;
		}
	}
	// register to control master
	int nResult = RegisterToMaster(m_endPoint, m_servantId, serverName,
		serverId, serverRegion, projectName, acceptAddress, processPath);

	// send result
	::node::RegisterResponse registerResponse;
	registerResponse.set_result(nResult);
	response.send(registerResponse);

	// notification register
	pFacade->SendNotification(N_CMD_NODE_REGISTER, serverId);
	////////////////////////////////////////////////////////////////////
	CAutoPointer<CallBackFuncP1<uint32_t> >
		callback(new CallBackFuncP1<uint32_t> 
		(&CServantServiceImp::KeepTimeoutCallback, serverId));

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->SetTimeout(serverId, KEEP_REGISTER_TIMEOUT, callback);
}

void CServantServiceImp::RemoveModule(const ::node::RemoveRequest& request,
	::rpcz::reply< ::node::RemoveResponse> response)
{
	// read data
	const std::string& serverName = request.servername();
	uint32_t serverId = request.serverid();

	///////////////////////////////////////////////////////////
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->Remove(serverId, true);
	///////////////////////////////////////////////////////////
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	CAutoPointer<IModule> module(pFacade->RetrieveModule(serverName));
	if(module.IsInvalid()) {
		::node::RemoveResponse removeResponse;
		removeResponse.set_result(CSR_WITHOUT_THIS_MODULE);
		response.send(removeResponse);
	} else {
		// notification register
		pFacade->SendNotification(N_CMD_NODE_REMOVE, serverId);

		CAutoPointer<IChannelControl> pChannelModule(module);
		if(pChannelModule.IsInvalid()) {
			::node::RemoveResponse removeResponse;
			removeResponse.set_result(CSR_NO_ICHANNELCONTROL);
			response.send(removeResponse);
		} else {

			if(pChannelModule->ChannelCount() < 2) {
				pFacade->RemoveModule(serverName);
			}

			if(pChannelModule->RemoveChannel(serverId)) {
				// unregister to control master
				int nResult = UnregisterToMaster(m_pMasterChannel, serverName, serverId);
				::node::RemoveResponse removeResponse;
				removeResponse.set_result(nResult);
				response.send(removeResponse);
			} else {
				::node::RemoveResponse removeResponse;
				removeResponse.set_result(CSR_REMOVE_CHANNEL_FAIL);
				response.send(removeResponse);
			}
		}
	}
	EraseServerId(serverId);
}

void CServantServiceImp::KeepRegister(const ::node::KeepRegisterRequest& request,
	::rpcz::reply< ::node::KeepRegisterResponse> response)
{
	uint32_t serverId = request.serverid();
	if(ID_NULL == serverId) {
		// If serverId is NULL, can be used to check if this server exists.  
		node::KeepRegisterResponse keepResponse;
		keepResponse.set_result(CSR_FAIL);
		response.send(keepResponse);
		return;
	}
	bool bExist = FindServerId(serverId);
	//////////////////////////////////////////////////////////
	node::KeepRegisterResponse keepResponse;
	if(bExist) {
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		pTMgr->Modify(serverId, TIMER_OPERATER_RESET);

		if(SERVER_STATUS_START == g_serverStatus) {
			keepResponse.set_result(CSR_SUCCESS_AND_START);
		} else {
			keepResponse.set_result(CSR_SUCCESS);
		}
	} else {
		keepResponse.set_result(CSR_NOT_FOUND);
	}
	std::string strServerName(request.servername());
	//std::string strEndPoint(request.endpoint());
	std::string strState(request.serverstate());

	uint32_t serverLoad = (uint32_t)request.serverload();
	int32_t serverStatus = (int32_t)request.serverstatus();

	response.send(keepResponse);

	if(!bExist) {
		return;
	}
	// Keep register to control master
	if(KeepRegisterToMaster(strServerName, serverId, serverLoad, serverStatus, strState) == CSR_TIMEOUT) {
		CFacade::PTR_T pFacade(CFacade::Pointer());
		CAutoPointer<IChannelControl> pChannelModule(pFacade->RetrieveModule(strServerName));
		if(!pChannelModule.IsInvalid()) {
			CAutoPointer<IChannelValue> pChannel(pChannelModule->GetChnlByDirServId(serverId));
			if(!pChannel.IsInvalid()) {
				RegisterToMaster(m_endPoint, m_servantId, strServerName, serverId, pChannel->GetServerRegion(),
					pChannel->GetProjectName(), pChannel->GetAcceptAddress(), pChannel->GetProcessPath());
			}
		}
	}
}

void CServantServiceImp::UserLogin(const ::node::UserLoginRequest& request,
    ::rpcz::reply< ::node::ControlCentreVoid> response)
{
	::node::ControlCentreVoid voidResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->UserLogin(m_pMasterChannel, request, voidResponse);

    response.send(voidResponse);  
}

void CServantServiceImp::UserLogout(const ::node::UserLogoutRequest& request,
    ::rpcz::reply< ::node::ControlCentreVoid> response)
{
	::node::ControlCentreVoid voidResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->UserLogout(m_pMasterChannel, request, voidResponse);

    response.send(voidResponse);
}

void CServantServiceImp::GetLowLoadNode(const ::node::LowLoadNodeRequest& request,
	::rpcz::reply< ::node::LowLoadNodeResponse> response)
{
	::node::LowLoadNodeResponse nodeResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->GetLowLoadNode(m_pMasterChannel, request, nodeResponse);

	response.send(nodeResponse);
}

void CServantServiceImp::GetRegionLowLoad(const ::node::RegionLowLoadRequest& request,
	::rpcz::reply< ::node::RegionLowLoadResponse> response)
{
	::node::RegionLowLoadResponse loadResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->GetRegionLowLoad(m_pMasterChannel, request, loadResponse);

	response.send(loadResponse);
}

void CServantServiceImp::GetNodeList(const ::node::NodeListRequest& request,
	::rpcz::reply< ::node::NodeListResponse> response)
{
	::node::NodeListResponse listResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->GetNodeList(m_pMasterChannel, request, listResponse);

	response.send(listResponse);
}

void CServantServiceImp::GetUsers(const ::node::GetUserRequest& request,
	::rpcz::reply< ::node::GetUserResponse> response)
{
	::node::GetUserResponse getResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->GetUsers(m_pMasterChannel, request, getResponse);

	response.send(getResponse);
}

void CServantServiceImp::CreateUser(const ::node::CreateUserRequest& request,
	::rpcz::reply< ::node::CreateUserResponse> response)
{
	::node::CreateUserResponse idResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->CreateUser(m_pMasterChannel, request, idResponse);

	response.send(idResponse);
}

void CServantServiceImp::CheckUser(const ::node::CheckUserRequest& request,
	::rpcz::reply< ::node::CheckUserResponse> response)
{
	::node::CheckUserResponse idResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->CheckUser(m_pMasterChannel, request, idResponse);

	response.send(idResponse);
}

void CServantServiceImp::UpdateUser(const ::node::UpdateUserRequest& request,
	::rpcz::reply< ::node::UpdateUserResponse> response)
{
	::node::UpdateUserResponse updateResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->UpdateUser(m_pMasterChannel, request, updateResponse);

	response.send(updateResponse);
}

void CServantServiceImp::DeleteUser(const ::node::DeleteUserRequest& request,
	::rpcz::reply< ::node::DeleteUserResponse> response)
{
	::node::DeleteUserResponse delResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->DeleteUser(m_pMasterChannel, request, delResponse);

	response.send(delResponse);
}

void CServantServiceImp::GetEndPointFromServant(const ::node::EndPointRequest& request,
	::rpcz::reply< ::node::EndPointResponse> response)
{
	::node::EndPointResponse endPointResponse;
	util::CAutoPointer<IChannelValue> pChlValue(
		CNodeModule::GetStaticChannel(request.serverid()));
	if (!pChlValue.IsInvalid()) {
		endPointResponse.set_endpoint(pChlValue->GetEndPoint());
	}
	response.send(endPointResponse);
}

void CServantServiceImp::SeizeServer(const ::node::SeizeRequest& request,
	::rpcz::reply< ::node::SeizeResponse> response)
{
	::node::SeizeResponse seizeResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->SeizeServer(m_pMasterChannel, request, seizeResponse);

	response.send(seizeResponse);
}

void CServantServiceImp::FreeServer(const ::node::FreeRequest& request,
	::rpcz::reply< ::node::FreeResponse> response)
{
	::node::FreeResponse freeResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->FreeServer(m_pMasterChannel, request, freeResponse);

	response.send(freeResponse);
}

void CServantServiceImp::GenerateGuid(const ::node::ControlCentreVoid& request,
	::rpcz::reply< ::node::GuidResponse> response)
{
	::node::GuidResponse guidResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->GenerateGuid(m_pMasterChannel, request, guidResponse);
	response.send(guidResponse);
}


int CServantServiceImp::RegisterToMaster(
	const std::string& endPoint,
	uint32_t servantId,
	const std::string& serverName,
	uint32_t serverId,
	uint16_t serverRegion,
	const std::string& projectName,
    const std::string& acceptAddress,
	const std::string& processPath,
	int deadline_ms/* = CALL_DEADLINE_MS*/)
{
	if(m_pMasterChannel.IsInvalid()) {
		return CSR_FAIL;
	}
	::node::ControlCentreService_Stub controlCentre_stub(
		m_pMasterChannel.operator->(), false);
	::node::RegisterRequest request;
	request.set_servertype(REGISTER_TYPE_NODE);
	request.set_servername(serverName);
	request.set_endpoint(endPoint);
	request.set_serverid(serverId);
	request.set_serverregion(serverRegion);
	request.set_servantid(servantId);
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
	return response.result();
}

int CServantServiceImp::UnregisterToMaster(
	util::CAutoPointer<rpcz::rpc_channel>& pMasterChannel,
	const std::string& serverName, uint32_t serverId)
{
	if(pMasterChannel.IsInvalid()) {
		return CSR_FAIL;
	}
	::node::ControlCentreService_Stub controlCentre_stub(
		pMasterChannel.operator->(), false);
	::node::RemoveRequest request;
	request.set_servername(serverName);
	request.set_serverid(serverId);
	request.set_servertype(REGISTER_TYPE_NODE);

	::node::RemoveResponse response;
	rpcz::rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);
	controlCentre_stub.RemoveModule(request, &response, &controller, NULL);
	controller.wait();
	return response.result();
}

int CServantServiceImp::KeepRegisterToMaster(
	const std::string& serverName,
	uint32_t serverId,
	uint32_t serverLoad,
	int32_t serverStatus,
	const std::string& serverState)
{
	if(m_pMasterChannel.IsInvalid()) {
		return false;
	}
	::node::ControlCentreService_Stub controlCentre_stub(
		m_pMasterChannel.operator->(), false);
	::node::KeepRegisterRequest request;
	request.set_servername(serverName);
	request.set_serverid(serverId);
	request.set_servertype(REGISTER_TYPE_NODE);
	request.set_serverload(serverLoad);
	request.set_serverstatus(serverStatus);
	request.set_serverstate(serverState);

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

void CServantServiceImp::KeepTimeoutCallback(uint32_t& serverId)
{
	EraseServerId(serverId);
	CFacade::PTR_T pFacade(CFacade::Pointer());
	pFacade->SendNotification(N_CMD_NODE_KEEPTIMEOUT, serverId);
}

void CServantServiceImp::ClearAllTimer()
{
	thd::CScopedReadLock rdLock(m_wrLock);
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	SERVER_ID_SET_T::const_iterator it = m_serverIds.begin();
	for(; m_serverIds.end() != it; ++it) {		
		pTMgr->Remove(*it, true);
	}
}



