#include "MasterServiceImp.h"
#include "ModuleManager.h"
#include "BodyMessage.h"
#include "NodeDefines.h"
#include "SpinLock.h"
#include "NodeModule.h"
#include "CacheModuleEx.h"
#include "WorkerModule.h"
#include "CacheOperateHelper.h"
#include "TimestampManager.h"
#include "SeparatedStream.h"
#include "TimerManager.h"
#include "GuidFactory.h"
#include "ValueStream.h"
#include "db_cxx.h"
#include "dbstl_common.h"
#include "Utf8.h"

#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 )
#include <io.h>
#include <direct.h>
#else
#include <sys/stat.h>
#endif

using namespace mdl;
using namespace util;
using namespace evt;

inline static bool IsMasterModule(uint16_t serverType) {
	return (REGISTER_TYPE_CACHE == serverType
		|| REGISTER_TYPE_WORKER == serverType);
}

CMasterServiceImp::SERVER_ID_SET_T CMasterServiceImp::m_serverIds;
thd::CSpinRWLock CMasterServiceImp::m_wrLock;


CMasterServiceImp::CMasterServiceImp(
	uint16_t serverId,
	const std::string& strCurPath)
	: m_cacheCount(0)
	, m_serverId(serverId)
{
#if COMPILER == COMPILER_MICROSOFT
	BUILD_BUG_ON(sizeof(int32_t) < sizeof(long));
#endif

	std::string strEnvHome(strCurPath);
	TrimStringEx(strEnvHome, '\"');
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

CMasterServiceImp::~CMasterServiceImp(void)
{
	ClearAllTimer();
	dbstl::close_db_env(m_pDbEnv);
}

void CMasterServiceImp::RegisterModule(const ::node::RegisterRequest& request,
	::rpcz::reply< ::node::OperateResponse> response)
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	// read data
	uint16_t serverType = (uint16_t)request.servertype();
	uint16_t serverId = (uint16_t)request.serverid();

	uint32_t serverKey = GetServerKey(serverId, serverType);
	if(!InsertServerKey(serverKey)) {
		::node::OperateResponse operateResponse;
		operateResponse.set_result(CSR_CHANNEL_ALREADY_EXIST);
		response.send(operateResponse);
		return;
	}

	bool bMasterModule = IsMasterModule(serverType);
	std::string serverName;
	if(bMasterModule) {
		serverName = MASTER_MODULE_PERFIX;
		serverName += request.servername();
	} else {
		serverName = request.servername();
	}

	std::string acceptAddress(request.acceptaddress());
	std::string processPath(request.processpath());
	std::string projectName(request.projectname());
	uint16_t servantId = (uint16_t)request.servantid();
	uint16_t serverRegion = (uint16_t)request.serverregion();

	CAutoPointer<IModule> module(pFacade->RetrieveModule(serverName));
	if(module.IsInvalid()) {
		switch(serverType) {
		case REGISTER_TYPE_CACHE:
			module.SetRawPointer(new CCacheModuleEx(serverName, request.endpoint(), serverId));
			break;
		case REGISTER_TYPE_WORKER:
			module.SetRawPointer(new CWorkerModule(m_pDbEnv, m_serverId,
				serverName, request.endpoint(), serverId));
			break;
		case REGISTER_TYPE_NODE:
		case REGISTER_TYPE_SERVANT:
			module.SetRawPointer(new CNodeModule(serverName, request.endpoint(), serverId,
				serverType, acceptAddress, processPath, projectName, serverRegion));
			break;
		default:
			break;
		};

		if(module.IsInvalid()) {
			::node::OperateResponse operateResponse;
			operateResponse.set_result(CSR_WITHOUT_REGISTER_TYPE);
			response.send(operateResponse);
			EraseServerKey(serverKey);
			return;
		}
		// send data
		pFacade->RegisterModule(module);
		//
		if(REGISTER_TYPE_NODE == serverType) {
			RouteCreateTable(serverName, serverId);
		}
	} else {

		CAutoPointer<IChannelControl> pChannelModule(module);
		if(pChannelModule.IsInvalid()) {
			::node::OperateResponse operateResponse;
			operateResponse.set_result(CSR_NO_ICHANNELCONTROL);
			response.send(operateResponse);
			EraseServerKey(serverKey);
			return;
		}

		bool bRet = true;
		if(bMasterModule) {
			bRet = pChannelModule->CreatChannel(serverId, request.endpoint(), serverType);
		} else {
			bRet = pChannelModule->CreatChannel(serverId, request.endpoint(), serverType,
				acceptAddress, processPath, projectName, serverRegion);
		}

		if(!bRet)  {
			::node::OperateResponse operateResponse;
			if(SERVER_STATUS_START == g_serverStatus) {
				operateResponse.set_result(CSR_SUCCESS_AND_START);
			} else {
				operateResponse.set_result(CSR_SUCCESS);
			}
			response.send(operateResponse);
			////////////////////////////////////////////////////////////////////
			CAutoPointer<CallBackFuncP4<const std::string, uint16_t, uint16_t, volatile int32_t*> >
				callback(new CallBackFuncP4<const std::string, uint16_t, uint16_t, volatile int32_t*>
				(&CMasterServiceImp::KeepTimeoutCallback, serverName, serverId, serverType, &m_cacheCount));

			CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
			pTMgr->SetTimeout(serverKey, KEEP_REGISTER_TIMEOUT, callback);
			////////////////////////////////////////////////////////////////////
			return;
		}
	}
	// send result
	::node::OperateResponse operateResponse;
	if(SERVER_STATUS_START == g_serverStatus) {
		operateResponse.set_result(CSR_SUCCESS_AND_START);
	} else {
		operateResponse.set_result(CSR_SUCCESS);
	}
	response.send(operateResponse);
	///////////////////////////////////////////////////////
	if(bMasterModule) {
		if(REGISTER_TYPE_CACHE == serverType) {
			// If have added the global cache.
			if((int32_t)atomic_inc(&m_cacheCount) == 1) {
				AddAllNode();
				pFacade->SendNotification(N_CMD_MASTER_REGISTER_CACHE);
			}
		}
		// notification register
		pFacade->SendNotification(N_CMD_NODE_REGISTER, serverId);
	} else {
		// all global cache ready then ...
		if(0 != (int32_t)m_cacheCount) {
			util::CValueStream datas;
			datas.Serialize(NODE_STATE_REGISTER);
			CRequestStoredProcs addRequest;
			addRequest.SetKey(datas);

			datas.Reset();
			char szTimeBuf[TIME_BUFFER_SIZE] = {'\0'};
			CTimestampManager::Pointer()->GetLocalDateTimeStr(szTimeBuf, sizeof(szTimeBuf));
			datas.Serialize(serverId);
			datas.Serialize(acceptAddress);
			datas.Serialize(serverName);
			datas.Serialize(0);
			datas.Serialize(0);
			datas.Serialize(" ");
			datas.Serialize(projectName);
			datas.Serialize(szTimeBuf);

			addRequest.SetParams(datas);

			CResponseRows addResponse;
			McDBStoredProcBalServId(serverId, addRequest, addResponse);
		}
		///////////////////////////////////////////////////////
		// notification register
		pFacade->SendNotification(N_CMD_SERVANT_NODE_REGISTER, serverId);
		//////////////////////////////////////////////////////////////////////////
		if((uint16_t)REGISTER_TYPE_NODE == serverType) {
			CNodeModule::AddServerId(servantId, serverId);
		}
	}
	////////////////////////////////////////////////////////////////////
	CAutoPointer<CallBackFuncP4<const std::string, uint16_t, uint16_t, volatile int32_t*> >
		callback(new CallBackFuncP4<const std::string, uint16_t, uint16_t, volatile int32_t*>
		(&CMasterServiceImp::KeepTimeoutCallback, serverName, serverId, serverType, &m_cacheCount));

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->SetTimeout(serverKey, KEEP_REGISTER_TIMEOUT, callback);
}

void CMasterServiceImp::RemoveModule(const ::node::RemoveRequest& request,
	::rpcz::reply< ::node::OperateResponse> response)
{
	// read data
	uint16_t serverId = (uint16_t)request.serverid();
	int32_t serverType = request.servertype();
	uint32_t serverKey = GetServerKey(serverId, serverType);
	///////////////////////////////////////////////////////////
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->Remove(serverKey, true);
	///////////////////////////////////////////////////////////
	std::string serverName;

	bool bMasterModule = IsMasterModule(serverType);

	if(bMasterModule) {
		serverName = MASTER_MODULE_PERFIX;
		serverName += request.servername();
	} else {
		serverName = request.servername();
	}

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
			if(bMasterModule) {
				// notification register
				pFacade->SendNotification(N_CMD_NODE_REMOVE, serverId);

				if(REGISTER_TYPE_CACHE == serverType) {
					// if no global cache
					if((int32_t)atomic_dec(&m_cacheCount) == 0) {
						pFacade->SendNotification(N_CMD_MASTER_REMOVE_CACHE);
						DelAllNode();
					}
				}
			} else {
				// notification register
				pFacade->SendNotification(N_CMD_SERVANT_NODE_REMOVE, serverId);
				//////////////////////////////////////////////////
				if(0 != (int32_t)m_cacheCount) {
					util::CValueStream datas;
					datas.Serialize(NODE_STATE_UNREGISTER);
					CRequestStoredProcs removeRequest;
					removeRequest.SetKey(datas);

					datas.Reset();
					datas.Serialize(serverId);
					removeRequest.SetParams(datas);

					CResponseRows removeResponse;
					McDBStoredProcBalServId(serverId, removeRequest, removeResponse);
				}
			}

			if(pChannelModule->ChannelCount() < 2) {
				pFacade->RemoveModule(serverName);
				//
				if(REGISTER_TYPE_NODE == serverType) {
					RouteDropTable(serverName, serverId);
				}
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
	EraseServerKey(serverKey);
}

void CMasterServiceImp::KeepRegister(const ::node::KeepRegisterRequest& request,
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

	int32_t serverType = request.servertype();
	uint32_t serverKey = GetServerKey(serverId, serverType);
	bool bExist = FindServerKey(serverKey);
	//////////////////////////////////////////////////////////
	if(bExist) {
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		pTMgr->Modify(serverKey, TIMER_OPERATER_RESET);
	}

	bool bMasterModule = IsMasterModule(serverType);

	if(bMasterModule) {
		node::KeepRegisterResponse registerResponse;
		registerResponse.set_reregister(!bExist);
		response.send(registerResponse);
		return;
	}

	//////////////////////////////////////////////////////////
	std::string strServerName(request.servername());
	//std::string strEndPoint(request.endpoint());
	//
	uint32_t serverLoad = request.serverload();
	int32_t serverStatus = request.serverstatus();
	std::string strState(request.serverstate());
	//////////////////////////////////////////////////////////
	node::KeepRegisterResponse registerResponse;
	registerResponse.set_reregister(!bExist);
	response.send(registerResponse);

	CFacade::PTR_T pFacade(CFacade::Pointer());
	CAutoPointer<INodeControl> pNodeModule(pFacade->RetrieveModule(strServerName));
	if(!pNodeModule.IsInvalid()) {
		pNodeModule->UpdateChannelLoad(serverId, serverLoad);
	}

	if(0 != (int32_t)m_cacheCount) {
		util::CValueStream datas;
		CRequestStoredProcs refreshRequest;
		datas.Serialize(NODE_STATE_REFRESH);
		refreshRequest.SetKey(datas);

		datas.Reset();
		char szTimeBuf[TIME_BUFFER_SIZE] = {'\0'};
		CTimestampManager::Pointer()->GetLocalDateTimeStr(szTimeBuf, sizeof(szTimeBuf));
		datas.Serialize(serverId);
		datas.Serialize(strServerName);
		datas.Serialize(serverLoad);
		datas.Serialize(serverStatus);
		datas.Serialize(strState);
		datas.Serialize(szTimeBuf);
		refreshRequest.SetParams(datas);

		CResponseRows refreshResponse;
		McDBStoredProcBalServId(serverId, refreshRequest, refreshResponse);
	}
}

void CMasterServiceImp::KeepTimeoutCallback(
	const std::string& serverName,
	uint16_t& serverId,
	uint16_t& serverType,
	volatile int32_t*& pCacheCount)
{
	OutputDebug("serverName = %s serverId= %d serverType=%d cacheCount=%d",
		serverName.c_str(), serverId, serverType, *pCacheCount);
	bool bMasterModule = IsMasterModule(serverType);
	uint32_t serverKey = GetServerKey(serverId, serverType);

	CFacade::PTR_T pFacade(CFacade::Pointer());
	CAutoPointer<IModule> module(pFacade->RetrieveModule(serverName));

	if(!module.IsInvalid()) {
		CAutoPointer<IChannelControl> pChannelModule(module);
		if(!pChannelModule.IsInvalid()) {
			if(bMasterModule) {
				pFacade->SendNotification(N_CMD_NODE_KEEPTIMEOUT, serverId);

				if(REGISTER_TYPE_CACHE == serverType) {
					if(atomic_dec(pCacheCount) == 0) {
						pFacade->SendNotification(N_CMD_MASTER_KEEPTIMEOUT_CACHE);
						// CacheServer 已经推出，所以没有必要删除数据库状态的数据。
						//DelAllNode();
					}
				}
			} else {
				pFacade->SendNotification(N_CMD_SERVANT_NODE_KEEPTIMEOUT, serverId);

				if(0 != (int32_t)*pCacheCount) {
					util::CValueStream datas;
					datas.Serialize(NODE_STATE_UNREGISTER);
					CRequestStoredProcs removeRequest;
					removeRequest.SetKey(datas);

					datas.Reset();
					datas.Serialize(serverId);
					removeRequest.SetParams(datas);

					CResponseRows removeResponse;
					McDBStoredProcBalServId(serverId, removeRequest, removeResponse);
				}
			}

			if(pChannelModule->ChannelCount() < 2) {
				pFacade->RemoveModule(serverName);
			}

			pChannelModule->RemoveChannel(serverId);
		}
	}

	EraseServerKey(serverKey);
}

void CMasterServiceImp::AddAllNode() {

	std::vector<CAutoPointer<mdl::IModule> > modules;
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	pFacade->IterateModule(modules);
	// register nodes
	std::vector<CAutoPointer<mdl::IModule> >::const_iterator itM;
	for(itM = modules.begin(); modules.end() != itM; ++itM) {
		CAutoPointer<INodeControl> pNodeModule(*itM);
		if(pNodeModule.IsInvalid()) {
			continue;
		}

		util::CValueStream datas;
		CRequestStoredProcs addRequest;
		datas.Serialize(NODE_STATE_REGISTER);
		addRequest.SetKey(datas);

		std::vector<CAutoPointer<IChannelValue> > channels;
		pNodeModule->IterateChannel(channels);
		std::vector<CAutoPointer<IChannelValue> >::const_iterator itC(channels.begin());
		for(; channels.end() != itC; ++itC) {
			CAutoPointer<IChannelValue> pNodeChannel(*itC);
			if(pNodeChannel.IsInvalid()) {
				continue;
			}

			datas.Reset();
			char szTimeBuf[TIME_BUFFER_SIZE] = {'\0'};
			CTimestampManager::Pointer()->GetLocalDateTimeStr(szTimeBuf, sizeof(szTimeBuf));
			datas.Serialize(pNodeChannel->GetServerId());
			datas.Serialize(pNodeChannel->GetAcceptAddress());
			datas.Serialize(pNodeModule->GetModuleName());
			datas.Serialize(0);
			datas.Serialize(0);
			datas.Serialize(" ");
			datas.Serialize(pNodeChannel->GetProjectName());
			datas.Serialize(szTimeBuf);
			addRequest.SetParams(datas);

			CResponseRows addResponse;
			McDBStoredProcBalServId(pNodeChannel->GetServerId(), addRequest, addResponse);
		}
	}
}

void CMasterServiceImp::DelAllNode() {

	std::vector<CAutoPointer<mdl::IModule> > modules;
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	pFacade->IterateModule(modules);

	std::vector<CAutoPointer<mdl::IModule> >::iterator itM(modules.begin());
	for(; modules.end() != itM; ++itM) {
		CAutoPointer<INodeControl> pNodeModule(*itM);
		if(pNodeModule.IsInvalid()) {
			continue;
		}

		util::CValueStream datas;
		CRequestStoredProcs removeRequest;
		datas.Serialize(NODE_STATE_UNREGISTER);
		removeRequest.SetKey(datas);

		std::vector<CAutoPointer<IChannelValue> > channels;
		pNodeModule->IterateChannel(channels);
		std::vector<CAutoPointer<IChannelValue> >::const_iterator itC(channels.begin());
		for(; channels.end() != itC; ++itC) {
			CAutoPointer<IChannelValue> pNodeChannel(*itC);
			if(pNodeChannel.IsInvalid()) {
				continue;
			}

			datas.Reset();
			datas.Serialize(pNodeChannel->GetServerId());
			removeRequest.SetParams(datas);

			CResponseRows removeResponse;
			McDBStoredProcBalServId(pNodeChannel->GetServerId(), removeRequest, removeResponse);
		}
	}
}

void CMasterServiceImp::UserLogin(const ::node::UserLoginRequest& request,
	::rpcz::reply< ::node::ControlCentreVoid> response)
{
	RouteInsertRecord(request.servername(), request.userid(), (uint16_t)request.serverid());

	::node::ControlCentreVoid voidResponse;
	response.send(voidResponse);
}

void CMasterServiceImp::UserLogout(const ::node::UserLogoutRequest& request,
	::rpcz::reply< ::node::ControlCentreVoid> response)
{
	RouteRemoveRecord(request.servername(), request.userid());

	::node::ControlCentreVoid voidResponse;
	response.send(voidResponse);
}

void CMasterServiceImp::GetLowLoadNode(const ::node::LowLoadNodeRequest& request,
	::rpcz::reply< ::node::LowLoadNodeResponse> response) {

	::node::LowLoadNodeResponse lowLoadNodeResponse;
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	CAutoPointer<INodeControl> pNodeModule(pFacade->RetrieveModule(request.servername()));
	if(!pNodeModule.IsInvalid()) {
		CAutoPointer<IChannelValue> pChannel(pNodeModule->GetLowLoadUserChnl());
		if(!pChannel.IsInvalid()) {
			lowLoadNodeResponse.set_acceptaddress(pChannel->GetAcceptAddress());
			lowLoadNodeResponse.set_serverregion(pChannel->GetServerRegion());
		}
	}
	response.send(lowLoadNodeResponse);
}

void CMasterServiceImp::GetRegionLowLoad(const ::node::RegionLowLoadRequest& request,
	::rpcz::reply< ::node::RegionLowLoadResponse> response) {

		::node::RegionLowLoadResponse regionLowLoadResponse;
		mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
		CAutoPointer<INodeControl> pNodeModule(pFacade->RetrieveModule(request.servername()));
		if(!pNodeModule.IsInvalid()) {
			CAutoPointer<IChannelValue> pChannel(pNodeModule->GetLowLoadByRegion(request.serverregion()));
			if(!pChannel.IsInvalid()) {
				regionLowLoadResponse.set_acceptaddress(pChannel->GetAcceptAddress());
			}
		}
		response.send(regionLowLoadResponse);
}

void CMasterServiceImp::GetNodeList(const ::node::NodeListRequest& request,
    ::rpcz::reply< ::node::NodeListResponse> response)
{
    mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
    CAutoPointer<INodeControl> pNodeModule(pFacade->RetrieveModule(request.servername()));
    if(pNodeModule.IsInvalid()) {
        ::node::NodeListResponse nodeListResponse;
        response.send(nodeListResponse);
        return;
    }

    ::node::NodeListResponse nodeListResponse;
    std::vector<CAutoPointer<IChannelValue> > channels;
    pNodeModule->IterateChannel(channels);
    for(int i = 0; i < (int)channels.size(); ++i) {
        CAutoPointer<IChannelValue> pChannel(channels[i]);
        if(pChannel.IsInvalid()) {
            continue;
        }
        ::node::NodeDetail* pNodeDetail = nodeListResponse.add_nodes();
        if(NULL == pNodeDetail) {
            continue;
        }
        pNodeDetail->set_serverregion(pChannel->GetServerRegion());
        pNodeDetail->set_acceptaddress(pChannel->GetAcceptAddress());
        pNodeDetail->set_serverload(pChannel->GetServerLoad());
    }
    response.send(nodeListResponse);
}

void CMasterServiceImp::CreateUserId(const ::node::CreateIdRequest& request,
	::rpcz::reply< ::node::CreateIdResponse> response)
{
	util::CValueStream datas;
	datas.Serialize(NODE_USER_LOGIN);
	CRequestStoredProcs cacheRequest;
	cacheRequest.SetKey(datas);

	datas.Reset();
	datas.Serialize(request.account());
	// 数据库设置成 %serverid% 数据类型，会把该值替换成对应CacheServer的serverId值
	// 但是参数个数必须与数据库设置的个数一致，所以这边填0
	datas.Serialize(0);
	cacheRequest.SetParams(datas);

	CResponseRows cacheResponse;
	eServerError serError = McDBStoredProcBalUserId(request.account(),
		cacheRequest, cacheResponse);
	if(SERVER_SUCCESS != serError) {
		::node::CreateIdResponse createResponse;
		createResponse.set_result(serError);
		response.send(createResponse);
		return;
	}

	if(MCERR_OK != cacheResponse.GetFirstRecordResult()) {
		::node::CreateIdResponse createResponse;
		createResponse.set_result(SERVER_ERROR_UNKNOW);
		response.send(createResponse);
		return;
	}

	::node::CreateIdResponse createResponse;
	int nValuesSize = cacheResponse.GetSize();
	if(nValuesSize > 0) {
		util::CValueStream tsValues(cacheResponse.GetValue(0));
		uint32_t index = ID_NULL;
		uint16_t serverId = ID_NULL;
		std::string createTime;
		uint32_t serverRegion = 0;
		tsValues.Parse(index);
		tsValues.Parse(serverId);
		tsValues.Parse(createTime);
		tsValues.Parse(serverRegion);
		uint64_t userId = CombineUserId(serverId, index);
		createResponse.set_result(SERVER_SUCCESS);
		createResponse.set_userid(userId);
		createResponse.set_serverregion(serverRegion);
		createResponse.set_createtime(createTime);
	} else {
		createResponse.set_result(SERVER_ERROR_UNKNOW);
	}
	response.send(createResponse);
}

void CMasterServiceImp::CheckUserId(const ::node::CheckIdRequest& request,
	::rpcz::reply< ::node::CheckIdResponse> response)
{
	CRequestSnglRow cacheRequest;
	mc_record_t* pMCRequest = cacheRequest.AddRecord();
	if(NULL == pMCRequest) {
		::node::CheckIdResponse checkResponse;
		checkResponse.set_result(CACHE_ERROR_POINTER_NULL);
		response.send(checkResponse);
		return;
	}
	util::CValueStream datas;
	datas.Serialize(NODE_USER_CHECK_KEY);
	datas.Serialize(request.account());
	cacheRequest.SetKey(pMCRequest, datas);

	CResponseRows cacheResponse;
	eServerError serError = McGetsBalUserId(request.account(),
		cacheRequest, cacheResponse);
	if(SERVER_SUCCESS != serError) {
		::node::CheckIdResponse checkResponse;
		checkResponse.set_result(serError);
		response.send(checkResponse);
		return;
	}

	MCResult nResult = cacheResponse.GetFirstRecordResult();
	if(MCERR_OK != nResult) {
		::node::CheckIdResponse checkResponse;
		checkResponse.set_result(SERVER_ERROR_UNKNOW);
		response.send(checkResponse);
		return;
	}

	int nValuesSize = cacheResponse.GetSize();
	if(nValuesSize > 0) {
		CValueStream tsValues(cacheResponse.GetValue(0));
		uint64_t userId = ID_NULL;
		std::string createTime;
		uint32_t serverRegion = 0;
		tsValues.Parse(userId);
		tsValues.Parse(createTime);
		tsValues.Parse(serverRegion);
		::node::CheckIdResponse checkResponse;
		checkResponse.set_result(SERVER_SUCCESS);
		checkResponse.set_userid(userId);
		checkResponse.set_createtime(createTime);
		checkResponse.set_serverregion(serverRegion);
		checkResponse.set_cas(cacheResponse.GetCas(0));
		response.send(checkResponse);
	} else {
		::node::CheckIdResponse checkResponse;
		checkResponse.set_result(SERVER_ERROR_UNKNOW);
		response.send(checkResponse);
	}
}

void CMasterServiceImp::UpdateUserRegion(const ::node::UpdateRegionRequest& request,
	::rpcz::reply< ::node::UpdateRegionResponse> response)
{
	CRequestSnglRow cacheRequest;
	mc_record_t* pMCRequest = cacheRequest.AddRecord();
	if(NULL == pMCRequest) {
		::node::UpdateRegionResponse updateResponse;
		updateResponse.set_result(CACHE_ERROR_POINTER_NULL);
		response.send(updateResponse);
		return;
	}

	util::CValueStream datas;
	datas.Serialize(NODE_USER_CHECK_KEY);
	datas.Serialize(request.account());
	cacheRequest.SetKey(pMCRequest, datas);
	datas.Reset();
	datas.Serialize((uint32_t)ID_NULL, false);
	datas.Serialize(std::string(), false);
	datas.Serialize(request.serverregion(), true);
	cacheRequest.SetValue(pMCRequest, datas);
	cacheRequest.SetCas(pMCRequest, request.cas());

	CResponseRows cacheResponse;
	eServerError serError = McCasBalUserId(request.account(),
		cacheRequest, cacheResponse);
	if(SERVER_SUCCESS != serError) {
		::node::UpdateRegionResponse updateResponse;
		updateResponse.set_result(serError);
		response.send(updateResponse);
		return;
	}

	::node::UpdateRegionResponse updateResponse;
	MCResult nResult = cacheResponse.GetFirstRecordResult();
	if(MCERR_OK == nResult) {
		updateResponse.set_result(SERVER_SUCCESS);
	} else if(MCERR_EXISTS == nResult) {
		updateResponse.set_result(CACHE_ERROR_RECORD_EXISTS);
	} else {
		updateResponse.set_result(SERVER_ERROR_UNKNOW);
	}
	response.send(updateResponse);
}

void CMasterServiceImp::CacheServerStore(const ::node::CacheStoreRequest& request,
	::rpcz::reply<::node::CacheStoreResponse> response)
{
	uint16_t nServerId = (uint16_t)request.serverid();
	CAutoPointer<IChannelValue> rpcChannel(
		CNodeModule::GetStaticChannel(nServerId));
	if(rpcChannel.IsInvalid()) {
		::node::CacheStoreResponse storeResponse;
		storeResponse.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		response.send(storeResponse);
		return;
	}

	mc_request_t mcRequest;

	int nKeySize = request.keys_size();
	for(int i = 0; i < nKeySize; ++i) {
		mc_record_t* pMcRecord = SetRecord(mcRequest);
		SetRecordKey(pMcRecord, request.keys(i));
	}

	::node::DataPacket cacheRequest;
	if(!SerializeCacheData(cacheRequest, mcRequest)) {
		::node::CacheStoreResponse storeResponse;
		storeResponse.set_result(CACHE_ERROR_SERIALIZE_REQUEST);
		response.send(storeResponse);
		return;
	}
	cacheRequest.set_cmd(N_CMD_STORE);
	cacheRequest.set_route_type(request.routetype());
	cacheRequest.set_route(request.route());

	::node::DataPacket servantRequest;
	if(!SerializeCacheData(servantRequest, cacheRequest)) {
		::node::CacheStoreResponse storeResponse;
		storeResponse.set_result(CACHE_ERROR_SERIALIZE_REQUEST);
		response.send(storeResponse);
		return;
	}
	servantRequest.set_cmd(N_CMD_SERVER_STORE);
	servantRequest.set_route(nServerId);

	::node::DataPacket servantResponse;
	eServerError nResult = (eServerError)
		CNodeModule::SendNodeMessage(
		rpcChannel, servantRequest, servantResponse);

	::node::CacheStoreResponse storeResponse;
	storeResponse.set_result(nResult);

	mc_response_t cacheResponse;
	ParseCacheData(cacheResponse, servantResponse);
	int nResSize = GetRecordSize(cacheResponse);
	for(int i = 0; i < nResSize; ++i) {
		const mc_record_t& mcRecord = GetRecord(cacheResponse, i);
		::node::CacheKeyResult* pKeyResult =
			storeResponse.add_keyresults();
		if(NULL == pKeyResult) {
			OutputError("NULL == pKeyResult 1");
			assert(false);
			continue;
		}
		pKeyResult->set_key(GetRecordKey(mcRecord));
		pKeyResult->set_result(GetRecordResult(mcRecord));
	}

	response.send(storeResponse);
}

void CMasterServiceImp::ClearAllTimer()
{
	thd::CScopedReadLock rdLock(m_wrLock);
	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	SERVER_ID_SET_T::const_iterator it(m_serverIds.begin());
	for(; m_serverIds.end() != it; ++it) {
		pTMgr->Remove(*it, true);
	}
}

void CMasterServiceImp::RouteCreateTable(const std::string& serverName, uint16_t serverId) {

	std::vector<CAutoPointer<mdl::IModule> > modules;
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	pFacade->IterateModule(modules);
	std::vector<CAutoPointer<mdl::IModule> >::const_iterator itM(modules.begin());
	for(; modules.end() != itM; ++itM) {
		CAutoPointer<CCacheModule> pCacheModule(*itM);
		if(pCacheModule.IsInvalid()) {
			continue;
		}

		CAutoPointer<IChannelValue> pChnlValue(pCacheModule->GetChnlByDirServId(serverId));
		if(!pChnlValue.IsInvalid()) {
			// This node is center cache server
			std::vector<CAutoPointer<mdl::IModule> >::const_iterator itM2(modules.begin());
			for(; modules.end() != itM2; ++itM2) {
				CAutoPointer<INodeControl> pNodeModule(*itM2);
				if(pNodeModule.IsInvalid()) {
					continue;
				}

				util::CValueStream datas;
				datas.Serialize(NODE_ROUTE_CREATE);

				CRequestStoredProcs addRequest;
				addRequest.SetKey(datas);
				addRequest.SetNoReply(true);

				datas.Reset();
				datas.Serialize(pNodeModule->GetModuleName());
				addRequest.SetParams(datas);

				CResponseRows addResponse;
				McDBAsyncStoredProcDirServId(serverId, addRequest, addResponse);
			}
			break;
		}

		util::CValueStream datas;
		CRequestStoredProcs addRequest;
		datas.Serialize(NODE_ROUTE_CREATE);

		addRequest.SetKey(datas);
		addRequest.SetNoReply(true);

		datas.Reset();
		datas.Serialize(serverName);
		addRequest.SetParams(datas);

		std::vector<CAutoPointer<IChannelValue> > channels;
		pCacheModule->IterateChannel(channels);
		std::vector<CAutoPointer<IChannelValue> >::const_iterator itC(channels.begin());
		for(; channels.end() != itC; ++itC) {
			CAutoPointer<IChannelValue> pCacheChnl(*itC);
			if(pCacheChnl.IsInvalid()) {
				continue;
			}

			CResponseRows addResponse;
			McDBAsyncStoredProcDirServId(pCacheChnl->GetServerId(), addRequest, addResponse);
		}
	}
}

void CMasterServiceImp::RouteDropTable(const std::string& serverName, uint16_t serverId)
{
	std::vector<CAutoPointer<mdl::IModule> > modules;
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	pFacade->IterateModule(modules);
	std::vector<CAutoPointer<mdl::IModule> >::const_iterator itM(modules.begin());
	for(; modules.end() != itM; ++itM) {
		CAutoPointer<CCacheModule> pCacheModule(*itM);
		if(pCacheModule.IsInvalid()) {
			continue;
		}

		CAutoPointer<IChannelValue> pChnlValue(pCacheModule->GetChnlByDirServId(serverId));
		if(!pChnlValue.IsInvalid()) {
			// This node is center cache server, do nothing.
			break;
		}

		util::CValueStream datas;
		CRequestStoredProcs dropRequest;
		datas.Serialize(NODE_ROUTE_DROP);

		dropRequest.SetKey(datas);
		dropRequest.SetNoReply(true);

		datas.Reset();
		datas.Serialize(serverName);
		dropRequest.SetParams(datas);

		std::vector<CAutoPointer<IChannelValue> > channels;
		pCacheModule->IterateChannel(channels);
		std::vector<CAutoPointer<IChannelValue> >::const_iterator itC(channels.begin());
		for(; channels.end() != itC; ++itC) {
			CAutoPointer<IChannelValue> pCacheChnl(*itC);
			if(pCacheChnl.IsInvalid()) {
				continue;
			}

			CResponseRows dropResponse;
			McDBAsyncStoredProcDirServId(pCacheChnl->GetServerId(), dropRequest, dropResponse);
		}
	}
}

void CMasterServiceImp::RouteInsertRecord(const std::string& serverName, uint64_t userId, uint16_t serverId)
{
	util::CValueStream datas;
	CRequestStoredProcs insertRequest;
	datas.Serialize(NODE_ROUTE_INSERT);
	insertRequest.SetKey(datas);
	insertRequest.SetNoReply(true);

	datas.Reset();
	datas.Serialize(serverName);
	datas.Serialize(userId);
	datas.Serialize(serverId);
	insertRequest.SetParams(datas);

	uint16_t cacheId = DepartUserId2ServId(userId);
	CResponseRows insertResponse;
	McDBAsyncStoredProcBalServId(cacheId, insertRequest, insertResponse);
}

void CMasterServiceImp::RouteRemoveRecord(const std::string& serverName, uint64_t userId)
{
	util::CValueStream datas;
	CRequestStoredProcs removeRequest;
	datas.Serialize(NODE_ROUTE_REMOVE);
	removeRequest.SetKey(datas);
	removeRequest.SetNoReply(true);

	datas.Reset();
	datas.Serialize(serverName);
	datas.Serialize(userId);
	removeRequest.SetParams(datas);

	uint16_t cacheId = DepartUserId2ServId(userId);
	CResponseRows removeResponse;
	McDBAsyncStoredProcBalServId(cacheId, removeRequest, removeResponse);
}

uint16_t CMasterServiceImp::RouteGetServerId(const std::string& serverName, uint64_t userId)
{
	util::CValueStream datas;
	CRequestStoredProcs getRequest;
	datas.Serialize(NODE_ROUTE_GET);
	getRequest.SetKey(datas);

	datas.Reset();
	datas.Serialize(serverName);
	datas.Serialize(userId);
	getRequest.SetParams(datas);

	uint16_t cacheId = DepartUserId2ServId(userId);
	CResponseRows getResponse;
	McDBAsyncStoredProcBalServId(cacheId, getRequest, getResponse);
	int nSize = getResponse.GetSize();
	if(nSize < 1) {
		return ID_NULL;
	}
	CValueStream rValue(getResponse.GetValue(0));
	uint16_t serverId = ID_NULL;
	rValue.Parse(serverId);
	return serverId;
}

void CMasterServiceImp::OnServerPlay(void)
{
	if(0 == (int32_t)m_cacheCount) {
		char szError[eBUF_SIZE_256] = {0};
		snprintf(szError, sizeof(szError), "The MasterServer must have one CacheServer.");
		throw std::logic_error(szError);
	}
}


