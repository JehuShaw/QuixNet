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
	const std::string& strBind, uint16_t servantId)
	: m_pMasterChannel(pMasterChannel)
	, m_strBind(strBind)
	, m_servantId(servantId)
{
	assert(!m_pMasterChannel.IsInvalid());
}


CServantServiceImp::~CServantServiceImp(void)
{
	ClearAllTimer();
}

void CServantServiceImp::RegisterModule(const ::node::RegisterRequest& request,
	::rpcz::reply< ::node::OperateResponse> response)
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	// read data
	uint16_t serverId = (uint16_t)request.serverid();
	if(!InsertServerId(serverId)) {
		::node::OperateResponse operateResponse;
		operateResponse.set_result(CSR_CHANNEL_ALREADY_EXIST);
		response.send(operateResponse);
		return;
	}

	int32_t serverType = request.servertype();
	std::string serverName(request.servername());
	std::string endPoint(request.endpoint());
	std::string projectName(request.projectname());
    std::string acceptAddress;
    if(request.has_acceptaddress()) {
        acceptAddress = request.acceptaddress();
    }
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
			::node::OperateResponse operateResponse;
			operateResponse.set_result(CSR_WITHOUT_REGISTER_TYPE);
			response.send(operateResponse);
			EraseServerId(serverId);
			return;
		}
		// send data
		pFacade->RegisterModule(module);
	} else {
		CAutoPointer<IChannelControl> pChannelModule(module);
		if(pChannelModule.IsInvalid()) {
			::node::OperateResponse operateResponse;
			operateResponse.set_result(CSR_NO_ICHANNELCONTROL);
			response.send(operateResponse);
			EraseServerId(serverId);
			return;
		}

		if(!pChannelModule->CreatChannel(serverId, endPoint, serverType,
			acceptAddress, processPath, projectName, serverRegion)) 
		{
			// register to control master
			int nResult = RegisterToMaster(m_strBind, m_servantId, serverName,
				serverId, serverRegion, projectName, acceptAddress, processPath);
			// send result
			::node::OperateResponse operateResponse;
			operateResponse.set_result(nResult);
			response.send(operateResponse);
			////////////////////////////////////////////////////////////////////
			CAutoPointer<CallBackFuncP1<uint16_t> >
				callback(new CallBackFuncP1<uint16_t> 
				(&CServantServiceImp::KeepTimeoutCallback, serverId));

			CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
			pTMgr->SetTimeout(serverId, KEEP_REGISTER_TIMEOUT, callback);
			return;
		}
	}
	// register to control master
	int nResult = RegisterToMaster(m_strBind, m_servantId, serverName,
		serverId, serverRegion, projectName, acceptAddress, processPath);

	// send result
	::node::OperateResponse operateResponse;
	operateResponse.set_result(nResult);
	response.send(operateResponse);

	// notification register
	pFacade->SendNotification(N_CMD_NODE_REGISTER, serverId);
	////////////////////////////////////////////////////////////////////
	CAutoPointer<CallBackFuncP1<uint16_t> >
		callback(new CallBackFuncP1<uint16_t> 
		(&CServantServiceImp::KeepTimeoutCallback, serverId));

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->SetTimeout(serverId, KEEP_REGISTER_TIMEOUT, callback);
}

void CServantServiceImp::RemoveModule(const ::node::RemoveRequest& request,
	::rpcz::reply< ::node::OperateResponse> response)
{
	// read data
	const std::string& serverName = request.servername();
	uint16_t serverId = (uint16_t)request.serverid();

	///////////////////////////////////////////////////////////
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->Remove(serverId, true);
	///////////////////////////////////////////////////////////
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	CAutoPointer<IModule> module(pFacade->RetrieveModule(serverName));
	if(module.IsInvalid()) {
		::node::OperateResponse operateResponse;
		operateResponse.set_result(CSR_WITHOUT_THIS_MODULE);
		response.send(operateResponse);
	} else {
		// notification register
		pFacade->SendNotification(N_CMD_NODE_REMOVE, serverId);

		CAutoPointer<IChannelControl> pChannelModule(module);
		if(pChannelModule.IsInvalid()) {
			::node::OperateResponse operateResponse;
			operateResponse.set_result(CSR_NO_ICHANNELCONTROL);
			response.send(operateResponse);
		} else {

			if(pChannelModule->ChannelCount() < 2) {
				pFacade->RemoveModule(serverName);
			}

			if(pChannelModule->RemoveChannel(serverId)) {
				// unregister to control master
				int nResult = UnregisterToMaster(m_pMasterChannel, serverName, serverId);
				::node::OperateResponse operateResponse;
				operateResponse.set_result(nResult);
				response.send(operateResponse);
			} else {
				::node::OperateResponse operateResponse;
				operateResponse.set_result(CSR_REMOVE_CHANNEL_FAIL);
				response.send(operateResponse);
			}
		}
	}
	EraseServerId(serverId);
}

void CServantServiceImp::KeepRegister(const ::node::KeepRegisterRequest& request,
	::rpcz::reply< ::node::KeepRegisterResponse> response)
{
	uint16_t serverId = (uint16_t)request.serverid();
	if(ID_NULL == serverId) {
		// If serverId is NULL, can be used to check if this server exists.  
		node::KeepRegisterResponse registerResponse;
		registerResponse.set_reregister(false);
		response.send(registerResponse);
		return;
	}
	bool bExist = FindServerId(serverId);
	//////////////////////////////////////////////////////////
	if(bExist) {
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		pTMgr->Modify(serverId, TIMER_OPERATER_RESET);
	}
	std::string strServerName(request.servername());
	//std::string strEndPoint(request.endpoint());
	std::string strState(request.serverstate());

	uint32_t serverLoad = (uint32_t)request.serverload();
	int32_t serverStatus = (int32_t)request.serverstatus();

	node::KeepRegisterResponse registerResponse;
	registerResponse.set_reregister(!bExist);
	response.send(registerResponse);

	if(!bExist) {
		return;
	}
	// Keep register to control master
	if(KeepRegisterToMaster(strServerName, serverId, serverLoad, serverStatus, strState))
	{
		CFacade::PTR_T pFacade(CFacade::Pointer());
		CAutoPointer<IChannelControl> pChannelModule(pFacade->RetrieveModule(strServerName));
		if(!pChannelModule.IsInvalid()) {
			CAutoPointer<IChannelValue> pChannel(pChannelModule->GetChnlByDirServId(serverId));
			if(!pChannel.IsInvalid()) {
				RegisterToMaster(m_strBind, m_servantId, strServerName, serverId, pChannel->GetServerRegion(),
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

void CServantServiceImp::CreateUserId(const ::node::CreateIdRequest& request,
	::rpcz::reply< ::node::CreateIdResponse> response)
{
	::node::CreateIdResponse idResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->CreateUserId(m_pMasterChannel, request, idResponse);

	response.send(idResponse);
}

void CServantServiceImp::CheckUserId(const ::node::CheckIdRequest& request,
	::rpcz::reply< ::node::CheckIdResponse> response)
{
	::node::CheckIdResponse idResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->CheckUserId(m_pMasterChannel, request, idResponse);

	response.send(idResponse);
}

void CServantServiceImp::UpdateUserRegion(const ::node::UpdateRegionRequest& request,
	::rpcz::reply< ::node::UpdateRegionResponse> response)
{
	::node::UpdateRegionResponse regionResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->UpdateUserRegion(m_pMasterChannel, request, regionResponse);

	response.send(regionResponse);
}

void CServantServiceImp::CacheServerStore(const ::node::CacheStoreRequest& request,
	::rpcz::reply< ::node::CacheStoreResponse> response)
{
	::node::CacheStoreResponse storeResponse;

	CServantStubImp::PTR_T pServantStubImp(CServantStubImp::Pointer());
	pServantStubImp->CacheServerStore(m_pMasterChannel, request, storeResponse);

	response.send(storeResponse);
}

int CServantServiceImp::RegisterToMaster(
	const std::string& endPoint,
	uint16_t servantId,
	const std::string& serverName,
	uint16_t serverId,
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
		&*m_pMasterChannel, false);
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
	::node::OperateResponse response;
	rpcz::rpc_controller controller;
	controller.set_deadline_ms(deadline_ms);
	controlCentre_stub.RegisterModule(request, &response, &controller, NULL);
	controller.wait();
	return response.result();
}

int CServantServiceImp::UnregisterToMaster(
	util::CAutoPointer<rpcz::rpc_channel>& pMasterChannel,
	const std::string& serverName, uint16_t serverId)
{
	if(pMasterChannel.IsInvalid()) {
		return CSR_FAIL;
	}
	::node::ControlCentreService_Stub controlCentre_stub(
		&*pMasterChannel, false);
	::node::RemoveRequest request;
	request.set_servername(serverName);
	request.set_serverid(serverId);
	request.set_servertype(REGISTER_TYPE_NODE);

	::node::OperateResponse response;
	rpcz::rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);
	controlCentre_stub.RemoveModule(request, &response, &controller, NULL);
	controller.wait();
	return response.result();
}

bool CServantServiceImp::KeepRegisterToMaster(
	const std::string& serverName,
	uint16_t serverId,
	uint32_t serverLoad,
	int serverStatus,
	const std::string& serverState)
{
	if(m_pMasterChannel.IsInvalid()) {
		return false;
	}
	::node::ControlCentreService_Stub controlCentre_stub(
		&*m_pMasterChannel, false);
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
		return response.reregister();
	}
	return false;
}

void CServantServiceImp::KeepTimeoutCallback(uint16_t& serverId)
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



