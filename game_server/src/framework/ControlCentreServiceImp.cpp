#include "ControlCentreServiceImp.h"
#include "ModuleManager.h"
#include "BodyMessage.h"
#include "NodeDefines.h"
#include "WorkerModule.h"
#include "CacheModule.h"
#include "SpinLock.h"
#include "rpc_channel.hpp"
#include "TimerManager.h"
#include "ChannelManager.h"
#include "BodyBitStream.h"

#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 ) || defined( _WIN64 )
#include <io.h>
#include <direct.h>
#else 
#include <sys/stat.h> 
#endif

using namespace mdl;
using namespace util;
using namespace evt;

CControlCentreServiceImp::SERVER_ID_SET_T CControlCentreServiceImp::s_serverIds;
thd::CSpinRWLock CControlCentreServiceImp::s_wrLock;

CControlCentreServiceImp::CControlCentreServiceImp(
	uint16_t serverRegion,
	CreateMethod createWorkerMethod /*=NULL*/,
	CreateMethod createCacheMethod /*=NULL*/)

	: m_serverRegion(serverRegion)
	, m_createWorkerMethod(createWorkerMethod)
	, m_createCacheMethod(createCacheMethod)
{
}


CControlCentreServiceImp::~CControlCentreServiceImp(void)
{
	ClearAllTimer();
}

void CControlCentreServiceImp::RegisterModule(const ::node::RegisterRequest& request,
	::rpcz::reply< ::node::RegisterResponse> response)
{
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	// read data
	uint16_t serverRegion = (uint16_t)request.serverregion();
	if(ID_NULL != serverRegion 
		&& ID_NULL != m_serverRegion 
		&& serverRegion != m_serverRegion)
	{
		::node::RegisterResponse registerResponse;
		registerResponse.set_result(CSR_REGION_INCONFORMITY);
		response.send(registerResponse);
		return;
	}

	uint32_t serverId = request.serverid();
	if(!InsertServerId(serverId)) {
		::node::RegisterResponse registerResponse;
		registerResponse.set_result(CSR_CHANNEL_ALREADY_EXIST);
		response.send(registerResponse);
		return;
	}

	int32_t registerType = request.servertype();
	const std::string serverName(request.servername());
	const std::string& endPoint = request.endpoint();
	bool routServer = request.routeserver();
	uint64_t routeAddressId = request.routeaddressid();
	
	CAutoPointer<IModule> module(pFacade->RetrieveModule(serverName));
	if(module.IsInvalid()) {

		switch(registerType) {
		case REGISTER_TYPE_WORKER:
			if(NULL != m_createWorkerMethod) {
				module.SetRawPointer((*m_createWorkerMethod)(serverName, endPoint,
					serverId, routServer, routeAddressId, request.routeuserids()));
			} else {
				module.SetRawPointer(new CWorkerModule(serverName, endPoint,
					serverId, routServer, routeAddressId, request.routeuserids()));
			}
			break;
		case REGISTER_TYPE_CACHE:
			if(NULL != m_createCacheMethod) {
				module.SetRawPointer((*m_createCacheMethod)(serverName, endPoint, 
					serverId, routServer, routeAddressId, request.routeuserids()));
			} else {
				module.SetRawPointer(new CCacheModule(serverName, endPoint,
					serverId, routServer, routeAddressId, request.routeuserids()));
			}
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
		if(!pChannelModule->CreatChannel(serverId, endPoint, registerType,
			routServer, routeAddressId, request.routeuserids())) {
			// send result
			::node::RegisterResponse registerResponse;
			if(SERVER_STATUS_START == g_serverStatus) {
				registerResponse.set_result(CSR_SUCCESS_AND_START);
			} else {
				registerResponse.set_result(CSR_SUCCESS);
			}
			CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
			pChlMgr->CheckAndGetAgentIDs(request.agentsize(), registerResponse.mutable_agentids());
			response.send(registerResponse);
			/////////////////////////////////////////////////////////////////
			CAutoPointer<CallBackFuncP1<uint32_t> >
				callback(new CallBackFuncP1<uint32_t> 
				(&CControlCentreServiceImp::KeepTimeoutCallback, serverId));

			CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
			pTMgr->SetTimeout(serverId, KEEP_REGISTER_TIMEOUT, callback);
			return;
		}
	}
	// send result
	::node::RegisterResponse registerResponse;
	if(SERVER_STATUS_START == g_serverStatus) {
		registerResponse.set_result(CSR_SUCCESS_AND_START);
	} else {
		registerResponse.set_result(CSR_SUCCESS);
	}
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	pChlMgr->CheckAndGetAgentIDs(request.agentsize(), registerResponse.mutable_agentids());
	response.send(registerResponse);
	// notification register
	CBodyBitStream bsArg;
	bsArg.WriteInt32(registerType);
	bsArg.WriteUInt32(serverId);
	CAutoPointer<CBodyBitStream> pArg(&bsArg, false);
	pFacade->SendNotification(N_CMD_NODE_REGISTER, pArg);
	////////////////////////////////////////////////////////////////////
	CAutoPointer<CallBackFuncP1<uint32_t> >
		callback(new CallBackFuncP1<uint32_t> 
		(&CControlCentreServiceImp::KeepTimeoutCallback, serverId));

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->SetTimeout(serverId, KEEP_REGISTER_TIMEOUT, callback);
}

void CControlCentreServiceImp::RemoveModule(const ::node::RemoveRequest& request,
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

		CAutoPointer<IChannelControl> pChannelModule(module);
		if(pChannelModule.IsInvalid()) {
			::node::RemoveResponse removeResponse;
			removeResponse.set_result(CSR_NO_ICHANNELCONTROL);
			response.send(removeResponse);
		} else {
			// notification register
			pFacade->SendNotification(N_CMD_NODE_REMOVE, serverId);

			if(pChannelModule->ChannelCount() < 2) {
				pFacade->RemoveModule(serverName);
			}

			if(pChannelModule->RemoveChannel(serverId)) {
				::node::RemoveResponse removeResponse;
				removeResponse.set_result(CSR_SUCCESS);
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

void CControlCentreServiceImp::KeepRegister(const ::node::KeepRegisterRequest& request,
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

		if (SERVER_STATUS_START == g_serverStatus) {
			keepResponse.set_result(CSR_SUCCESS_AND_START);
		} else {
			keepResponse.set_result(CSR_SUCCESS);
		}
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		pChlMgr->CheckAndGetAgentIDs(request.agentsize(), keepResponse.mutable_agentids());
	} else {
		keepResponse.set_result(CSR_NOT_FOUND);
	}
	response.send(keepResponse);
}

void CControlCentreServiceImp::KeepTimeoutCallback(uint32_t& serverId)
{
	EraseServerId(serverId);
	CFacade::PTR_T pFacade(CFacade::Pointer());
	pFacade->SendNotification(N_CMD_NODE_KEEPTIMEOUT, serverId);
}

void CControlCentreServiceImp::ClearAllTimer()
{
	thd::CScopedReadLock rdLock(s_wrLock);
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	SERVER_ID_SET_T::const_iterator it = s_serverIds.begin();
	for(; s_serverIds.end() != it; ++it) {
		pTMgr->Remove(*it, true);
	}
}



