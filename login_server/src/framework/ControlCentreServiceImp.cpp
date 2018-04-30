#include "ControlCentreServiceImp.h"
#include "ModuleManager.h"
#include "BodyMessage.h"
#include "NodeDefines.h"
#include "WorkerModule.h"
#include "CacheModule.h"
#include "SpinLock.h"
#include "rpc_channel.hpp"
#include "TimerManager.h"
#include "db_cxx.h"
#include "dbstl_common.h"

#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )
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
	uint16_t serverId,
	const std::string& strCurPath)

	: m_serverRegion(serverRegion)
	, m_serverId(serverId)
{
	std::string strEnvHome(strCurPath);
	strEnvHome += "/store/";

	if(access(strEnvHome.c_str(), 0) == -1) {
#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )
		mkdir(strEnvHome.c_str());
#else
		mkdir(strEnvHome.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
	}

	m_pDbEnv = dbstl::open_env(strEnvHome.c_str(), 0);
}


CControlCentreServiceImp::~CControlCentreServiceImp(void)
{
	ClearAllTimer();
	dbstl::close_db_env(m_pDbEnv);
}

void CControlCentreServiceImp::RegisterModule(const ::node::RegisterRequest& request,
	::rpcz::reply< ::node::OperateResponse> response)
{
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	// read data
	uint16_t serverRegion = (uint16_t)request.serverregion();
	if(ID_NULL != serverRegion 
		&& ID_NULL != m_serverRegion 
		&& serverRegion != m_serverRegion)
	{
		::node::OperateResponse operateResponse;
		operateResponse.set_result(CSR_REGION_INCONFORMITY);
		response.send(operateResponse);
		return;
	}

	uint16_t serverId = (uint16_t)request.serverid();
	if(!InsertServerId(serverId)) {
		::node::OperateResponse operateResponse;
		operateResponse.set_result(CSR_CHANNEL_ALREADY_EXIST);
		response.send(operateResponse);
		return;
	}

	int32_t registerType = request.servertype();
	const std::string serverName(request.servername());
	const std::string& endPoint = request.endpoint();
	
	CAutoPointer<IModule> module(pFacade->RetrieveModule(serverName));
	if(module.IsInvalid()) {

		switch(registerType) {
		case REGISTER_TYPE_WORKER:
			module.SetRawPointer(new CWorkerModule(m_pDbEnv,
				m_serverId, serverName, endPoint, serverId));
			break;
		case REGISTER_TYPE_CACHE:
			module.SetRawPointer(new CCacheModule(serverName, endPoint, serverId));
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
		if(!pChannelModule->CreatChannel(serverId, endPoint, registerType)) {
			// send result
			::node::OperateResponse operateResponse;
			operateResponse.set_result(CSR_SUCCESS);
			response.send(operateResponse);
			/////////////////////////////////////////////////////////////////
			CAutoPointer<CallBackFuncP1<uint16_t> >
				callback(new CallBackFuncP1<uint16_t> 
				(&CControlCentreServiceImp::KeepTimeoutCallback, serverId));

			CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
			pTMgr->SetTimeout(serverId, KEEP_REGISTER_TIMEOUT, callback);
			return;
		}
	}
	// send result
	::node::OperateResponse operateResponse;
	operateResponse.set_result(CSR_SUCCESS);
	response.send(operateResponse);
	// notification register
	pFacade->SendNotification(N_CMD_NODE_REGISTER, serverId);
	////////////////////////////////////////////////////////////////////
	CAutoPointer<CallBackFuncP1<uint16_t> >
		callback(new CallBackFuncP1<uint16_t> 
		(&CControlCentreServiceImp::KeepTimeoutCallback, serverId));

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->SetTimeout(serverId, KEEP_REGISTER_TIMEOUT, callback);
}

void CControlCentreServiceImp::RemoveModule(const ::node::RemoveRequest& request,
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

		CAutoPointer<IChannelControl> pChannelModule(module);
		if(pChannelModule.IsInvalid()) {
			::node::OperateResponse operateResponse;
			operateResponse.set_result(CSR_NO_ICHANNELCONTROL);
			response.send(operateResponse);
		} else {
			// notification register
			pFacade->SendNotification(N_CMD_NODE_REMOVE, serverId);

			if(pChannelModule->ChannelCount() < 2) {
				pFacade->RemoveModule(serverName);
			}

			if(pChannelModule->RemoveChannel(serverId)) {
				::node::OperateResponse operateResponse;
				operateResponse.set_result(CSR_SUCCESS);
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

void CControlCentreServiceImp::KeepRegister(const ::node::KeepRegisterRequest& request,
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
	node::KeepRegisterResponse registerResponse;
	registerResponse.set_reregister(!bExist);
	response.send(registerResponse);
}

void CControlCentreServiceImp::KeepTimeoutCallback(uint16_t& serverId)
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



