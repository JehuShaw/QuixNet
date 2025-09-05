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
#include "GlobalIDFactory.h"
#include "ValueStream.h"
#include "Utf8.h"
#include "ControlCentreStubImpEx.h"

#if defined( __WIN32__) || defined( WIN32 ) || defined ( _WIN32 ) || defined( _WIN64 )
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
	const std::string& strCurPath)
	: m_cacheCount(0)
{
#if COMPILER == COMPILER_MICROSOFT
	BUILD_BUG_ON(sizeof(int32_t) < sizeof(long));
#endif
}

void CMasterServiceImp::RegisterModule(const ::node::RegisterRequest& request,
	::rpcz::reply< ::node::RegisterResponse> response)
{
	CFacade::PTR_T pFacade(CFacade::Pointer());
	// read data
	uint16_t serverType = static_cast<uint16_t>(request.servertype());
	uint32_t serverId = request.serverid();

	uint64_t serverKey = GetServerKey(serverId, serverType);
	if(!InsertServerKey(serverKey)) {
		::node::RegisterResponse registerResponse;
		registerResponse.set_result(CSR_CHANNEL_ALREADY_EXIST);
		response.send(registerResponse);
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

	bool routServer = request.routeserver();
	uint64_t routeAddressId = request.routeaddressid();
	std::string acceptAddress(request.acceptaddress());
	std::string processPath(request.processpath());
	std::string projectName(request.projectname());
	uint32_t servantId = request.servantid();
	uint16_t serverRegion = static_cast<uint16_t>(request.serverregion());

	CAutoPointer<IModule> module(pFacade->RetrieveModule(serverName));
	if(module.IsInvalid()) {
		switch(serverType) {
		case REGISTER_TYPE_CACHE:
			module.SetRawPointer(new CCacheModuleEx(serverName, request.endpoint(), serverId,
				routServer, routeAddressId, request.routeuserids()));
			break;
		case REGISTER_TYPE_WORKER:
			module.SetRawPointer(new CWorkerModule(serverName, request.endpoint(), serverId,
				routServer, routeAddressId, request.routeuserids()));
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
			::node::RegisterResponse registerResponse;
			registerResponse.set_result(CSR_WITHOUT_REGISTER_TYPE);
			response.send(registerResponse);
			EraseServerKey(serverKey);
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
			EraseServerKey(serverKey);
			return;
		}

		bool bRet = true;
		if(bMasterModule) {
			bRet = pChannelModule->CreatChannel(serverId, request.endpoint(), serverType,
				routServer, routeAddressId, request.routeuserids());
		} else {
			bRet = pChannelModule->CreatChannel(serverId, request.endpoint(), serverType, 
				acceptAddress, processPath, projectName, serverRegion);
		}

		if(!bRet)  {
			::node::RegisterResponse registerResponse;
			if(SERVER_STATUS_START == g_serverStatus) {
				registerResponse.set_result(CSR_SUCCESS_AND_START);
			} else {
				registerResponse.set_result(CSR_SUCCESS);
			}
			response.send(registerResponse);
			////////////////////////////////////////////////////////////////////
			CAutoPointer<CallBackFuncP4<const std::string, uint32_t, uint16_t, volatile int32_t*> >
				callback(new CallBackFuncP4<const std::string, uint32_t, uint16_t, volatile int32_t*> 
				(&CMasterServiceImp::KeepTimeoutCallback, serverName, serverId, serverType, &m_cacheCount));

			CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
			pTMgr->SetTimeout(serverKey, KEEP_REGISTER_TIMEOUT, callback);
			////////////////////////////////////////////////////////////////////
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
	response.send(registerResponse);
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
			McDBStoredProcHash32Key(serverId, addRequest, addResponse);
		}
		///////////////////////////////////////////////////////
		// notification register
		pFacade->SendNotification(N_CMD_SERVANT_NODE_REGISTER, serverId);
		//////////////////////////////////////////////////////////////////////////
		if(static_cast<uint16_t>(REGISTER_TYPE_NODE) == serverType) {
			CNodeModule::AddServerId(servantId, serverId);
		}
	}
	////////////////////////////////////////////////////////////////////
	CAutoPointer<CallBackFuncP4<const std::string, uint32_t, uint16_t, volatile int32_t*> >
		callback(new CallBackFuncP4<const std::string, uint32_t, uint16_t, volatile int32_t*> 
		(&CMasterServiceImp::KeepTimeoutCallback, serverName, serverId, serverType, &m_cacheCount));

	CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
	pTMgr->SetTimeout(serverKey, KEEP_REGISTER_TIMEOUT, callback);
}

void CMasterServiceImp::RemoveModule(const ::node::RemoveRequest& request,
	::rpcz::reply< ::node::RemoveResponse> response)
{
	// read data
	uint32_t serverId = request.serverid();
	int32_t serverType = request.servertype();
	uint64_t serverKey = GetServerKey(serverId, serverType);
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
					McDBStoredProcHash32Key(serverId, removeRequest, removeResponse);
				}
			}

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
	EraseServerKey(serverKey);
}

void CMasterServiceImp::KeepRegister(const ::node::KeepRegisterRequest& request,
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

	int32_t serverType = request.servertype();
	uint64_t serverKey = GetServerKey(serverId, serverType);
	bool bExist = FindServerKey(serverKey);
	//////////////////////////////////////////////////////////
	node::KeepRegisterResponse keepResponse;
	if(bExist) {
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		pTMgr->Modify(serverKey, TIMER_OPERATER_RESET);

		if(SERVER_STATUS_START == g_serverStatus) {
			keepResponse.set_result(CSR_SUCCESS_AND_START);
		} else {
			keepResponse.set_result(CSR_SUCCESS);
		}
	} else {
		keepResponse.set_result(CSR_NOT_FOUND);
	}

	bool bMasterModule = IsMasterModule(serverType);

	if(bMasterModule) {
		response.send(keepResponse);
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
	response.send(keepResponse);

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
		McDBStoredProcHash32Key(serverId, refreshRequest, refreshResponse);
	}
}

void CMasterServiceImp::KeepTimeoutCallback(
	const std::string& serverName,
	uint32_t& serverId,
	uint16_t& serverType,
	volatile int32_t*& pCacheCount)
{
	OutputDebug("serverName = %s serverId= %d serverType=%d cacheCount=%d",
		serverName.c_str(), serverId, serverType, *pCacheCount);
	bool bMasterModule = IsMasterModule(serverType);
	uint64_t serverKey = GetServerKey(serverId, serverType);

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
					McDBStoredProcHash32Key(serverId, removeRequest, removeResponse);
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
			McDBStoredProcHash32Key(pNodeChannel->GetServerId(), addRequest, addResponse);
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
			McDBStoredProcHash32Key(pNodeChannel->GetServerId(), removeRequest, removeResponse);
		}
	}
}

void CMasterServiceImp::UserLogin(const ::node::UserLoginRequest& request,
	::rpcz::reply< ::node::ControlCentreVoid> response)
{
	::node::ControlCentreVoid voidResponse;
	response.send(voidResponse);
}

void CMasterServiceImp::UserLogout(const ::node::UserLogoutRequest& request,
	::rpcz::reply< ::node::ControlCentreVoid> response)
{
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
			std::string realEndPoint(CControlCentreStubImpEx::Pointer()->GetEndPointFromServant(
				pChannel->GetEndPoint(), pChannel->GetServerId()));
			lowLoadNodeResponse.set_acceptaddress(pChannel->GetAcceptAddress());
			lowLoadNodeResponse.set_endpoint(realEndPoint);
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
				std::string realEndPoint(CControlCentreStubImpEx::Pointer()->GetEndPointFromServant(
					pChannel->GetEndPoint(), pChannel->GetServerId()));
				regionLowLoadResponse.set_acceptaddress(pChannel->GetAcceptAddress());
				regionLowLoadResponse.set_endpoint(realEndPoint);
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

void CMasterServiceImp::GetUsers(const ::node::GetUserRequest& request,
	::rpcz::reply< ::node::GetUserResponse> response)
{
	util::CValueStream datas;
	datas.Serialize(NODE_USER_GET);
	CRequestStoredProcs cacheRequest;
	cacheRequest.SetKey(datas);

	datas.Reset();

	datas.Serialize(request.account());
	cacheRequest.SetParams(datas);

	uint32_t u32Key = GenerateU32Key(request.account());

	CResponseRows cacheResponse;
	eServerError serError = McDBStoredProcHash32Key(
		u32Key, cacheRequest, cacheResponse);
	if (SERVER_SUCCESS != serError) {
		::node::GetUserResponse getResponse;
		getResponse.set_result(serError);
		response.send(getResponse);
		return;
	}

	::node::GetUserResponse getResponse;
	getResponse.set_result(SERVER_SUCCESS);
	int nValuesSize = cacheResponse.GetSize();
	for (int i = 0; i < nValuesSize; ++i) {
		util::CValueStream tsValues(cacheResponse.GetValue(i));
		uint32_t index = ID_NULL;
		u32Key = ID_NULL;
		std::string createTime;
		uint32_t serverRegion = 0;
		uint32_t loginCount = 0;
		uint32_t mapId = ID_NULL;
		tsValues.Parse(index);
		tsValues.Parse(u32Key);
		tsValues.Parse(createTime);
		tsValues.Parse(serverRegion);
		tsValues.Parse(loginCount);
		tsValues.Parse(mapId);

		uint64_t userId = CombineUserId(u32Key, index);

		::node::UserPacket* pUserData = getResponse.add_rows();
		pUserData->set_userid(userId);
		pUserData->set_serverregion(serverRegion);
		pUserData->set_createtime(createTime);
		pUserData->set_logincount(loginCount);
		pUserData->set_mapid(mapId);
	}
	response.send(getResponse);
}

void CMasterServiceImp::CreateUser(const ::node::CreateUserRequest& request,
	::rpcz::reply< ::node::CreateUserResponse> response)
{
	util::CValueStream datas;
	datas.Serialize(NODE_USER_CREATE);
	CRequestStoredProcs cacheRequest;
	cacheRequest.SetKey(datas);

	datas.Reset();

	uint32_t u32Key = GenerateU32Key(request.account());
	uint32_t mapId = request.mapid();
	uint32_t maxSize = request.maxsize();

	datas.Serialize(request.account());
	datas.Serialize(u32Key);
	datas.Serialize(mapId);
	datas.Serialize(maxSize);
	cacheRequest.SetParams(datas);

	CResponseRows cacheResponse;
	eServerError serError = McDBStoredProcHash32Key(
		u32Key, cacheRequest, cacheResponse);
	if(SERVER_SUCCESS != serError) {
		::node::CreateUserResponse createResponse;
		createResponse.set_result(serError);
		response.send(createResponse);
		return;
	}

	MCResult mcRet = cacheResponse.GetFirstRecordResult();
	if(MCERR_OK != mcRet) {
		::node::CreateUserResponse createResponse;
		createResponse.set_result(SERVER_ERROR_UNKNOW);
		response.send(createResponse);
		return;
	}

	::node::CreateUserResponse createResponse;
	int nValuesSize = cacheResponse.GetSize();
	if(nValuesSize > 0) {
		util::CValueStream tsValues(cacheResponse.GetValue(0));
		uint32_t curSize = 0;
		tsValues.Parse(curSize);
		if (curSize >= maxSize) {
			createResponse.set_result(MAX_CHARARER_LIMIT);
			response.send(createResponse);
			return;
		}
		uint32_t index = ID_NULL;
		u32Key = ID_NULL;
		std::string createTime;
		uint32_t serverRegion = 0;
		uint32_t loginCount = 0;
		
		tsValues.Parse(index);
		tsValues.Parse(u32Key);
		tsValues.Parse(createTime);
		tsValues.Parse(serverRegion);
		tsValues.Parse(loginCount);
		uint64_t userId = CombineUserId(u32Key, index);

		createResponse.set_result(SERVER_SUCCESS);
		::node::UserPacket* pUserData = createResponse.mutable_row();
		pUserData->set_userid(userId);
		pUserData->set_serverregion(serverRegion);
		pUserData->set_createtime(createTime);
		pUserData->set_logincount(loginCount);
		pUserData->set_mapid(mapId);
	} else {
		createResponse.set_result(SERVER_ERROR_UNKNOW);
	}
	response.send(createResponse);
}

void CMasterServiceImp::CheckUser(const ::node::CheckUserRequest& request,
	::rpcz::reply< ::node::CheckUserResponse> response)
{
	util::CValueStream datas;
	datas.Serialize(NODE_USER_CHECK);
	CRequestStoredProcs cacheRequest;
	cacheRequest.SetKey(datas);

	datas.Reset();

	uint32_t u32Key = DepartUserId2U32Key(request.userid());

	datas.Serialize(DepartUserId2Index(request.userid()));
	datas.Serialize(u32Key);
	cacheRequest.SetParams(datas);


	CResponseRows cacheResponse;
	eServerError serError = McDBStoredProcHash32Key(
		u32Key, cacheRequest, cacheResponse);
	if (SERVER_SUCCESS != serError) {
		::node::CheckUserResponse checkResponse;
		checkResponse.set_result(serError);
		response.send(checkResponse);
		return;
	}

	if (MCERR_OK != cacheResponse.GetFirstRecordResult()) {
		::node::CheckUserResponse checkResponse;
		checkResponse.set_result(SERVER_ERROR_NOTFOUND_CHARACTER);
		response.send(checkResponse);
		return;
	}

	::node::CheckUserResponse checkResponse;
	int nValuesSize = cacheResponse.GetSize();
	if (nValuesSize > 0) {
		util::CValueStream tsValues(cacheResponse.GetValue(0));
		std::string createTime;
		uint32_t serverRegion = 0;
		uint32_t loginCount = 0;
		uint64_t account = DEFAULT_ACCOUNT;
		uint32_t mapId = ID_NULL;
		tsValues.Parse(createTime);
		tsValues.Parse(serverRegion);
		tsValues.Parse(loginCount);
		tsValues.Parse(account);
		tsValues.Parse(mapId);

		checkResponse.set_result(SERVER_SUCCESS);
		checkResponse.set_serverregion(serverRegion);
		checkResponse.set_createtime(createTime);
		checkResponse.set_logincount(loginCount);
		checkResponse.set_account(account);
		checkResponse.set_mapid(mapId);
	} else {
		checkResponse.set_result(SERVER_ERROR_NOTFOUND_CHARACTER);
	}
	response.send(checkResponse);
}

void CMasterServiceImp::UpdateUser(const ::node::UpdateUserRequest& request,
	::rpcz::reply< ::node::UpdateUserResponse> response)
{
	util::CValueStream datas;
	datas.Serialize(NODE_USER_UPDATE);
	CRequestStoredProcs cacheRequest;
	cacheRequest.SetKey(datas);

	datas.Reset();

	uint32_t u32Key = DepartUserId2U32Key(request.userid());

	datas.Serialize(DepartUserId2Index(request.userid()));
	datas.Serialize(u32Key);
	datas.Serialize((uint8_t)request.login());
	datas.Serialize(request.mapid());
	datas.Serialize(request.serverregion());
	cacheRequest.SetParams(datas);


	CResponseRows cacheResponse;
	eServerError serError = McDBStoredProcHash32Key(
		u32Key, cacheRequest, cacheResponse);
	if (SERVER_SUCCESS != serError) {
		::node::UpdateUserResponse updateResponse;
		updateResponse.set_result(serError);
		response.send(updateResponse);
		return;
	}

	::node::UpdateUserResponse updateResponse;
	updateResponse.set_result(SERVER_SUCCESS);
	response.send(updateResponse);
}

void CMasterServiceImp::DeleteUser(const ::node::DeleteUserRequest& request,
	::rpcz::reply< ::node::DeleteUserResponse> response)
{
	util::CValueStream datas;
	datas.Serialize(NODE_USER_DELETE);
	CRequestStoredProcs cacheRequest;
	cacheRequest.SetKey(datas);

	datas.Reset();

	uint32_t u32Key = DepartUserId2U32Key(request.userid());

	datas.Serialize(DepartUserId2Index(request.userid()));
	datas.Serialize(u32Key);
	cacheRequest.SetParams(datas);

	CResponseRows cacheResponse;
	eServerError serError = McDBStoredProcHash32Key(
		u32Key, cacheRequest, cacheResponse);
	if (SERVER_SUCCESS != serError) {
		::node::DeleteUserResponse delResponse;
		delResponse.set_result(serError);
		response.send(delResponse);
		return;
	}

	::node::DeleteUserResponse delResponse;
	delResponse.set_result(SERVER_SUCCESS);
	response.send(delResponse);
}

void CMasterServiceImp::SeizeServer(const ::node::SeizeRequest& request,
	::rpcz::reply< ::node::SeizeResponse> response)
{
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	CAutoPointer<INodeControl> pNodeModule(pFacade->RetrieveModule(request.servername()));
	if (pNodeModule.IsInvalid()) {
		::node::SeizeResponse seizeResponse;
		response.send(seizeResponse);
		return;
	}
	CAutoPointer<IChannelValue> pChannel(pNodeModule->GetLowLoadUserChnl());
	if (pChannel.IsInvalid()) {
		::node::SeizeResponse seizeResponse;
		response.send(seizeResponse);
		return;
	}

	::node::SeizeResponse seizeResponse;

	util::CValueStream datas;
	datas.Serialize(NODE_SEIZE_SERVER);
	CRequestStoredProcs cacheRequest;
	cacheRequest.SetKey(datas);

	datas.Reset();

	uint32_t u32Key = DepartUserId2U32Key(request.userid());
	datas.Serialize(DepartUserId2Index(request.userid()));
	datas.Serialize(u32Key);
	datas.Serialize(pChannel->GetAcceptAddress());
	datas.Serialize(pChannel->GetServerId());
	datas.Serialize((uint8_t)request.login());
	cacheRequest.SetParams(datas);

	CResponseRows cacheResponse;
	eServerError serError = McDBStoredProcHash32Key(
		u32Key, cacheRequest, cacheResponse);
	if (SERVER_SUCCESS == serError) {
		int nValuesSize = cacheResponse.GetSize();
		if (nValuesSize > 0) {
			util::CValueStream tsValues(cacheResponse.GetValue(0));
			tsValues.Parse(*(std::string*)seizeResponse.mutable_acceptaddress());
			uint32_t serverId = ID_NULL;
			tsValues.Parse(serverId);
			uint32_t mapId = ID_NULL;
			tsValues.Parse(mapId);
			if (seizeResponse.acceptaddress().empty()) {
				seizeResponse.set_acceptaddress(pChannel->GetAcceptAddress());
				seizeResponse.set_serverid(pChannel->GetServerId());
			} else {
				seizeResponse.set_serverid(serverId);
			}
			seizeResponse.set_mapid(mapId);
		} else {
			seizeResponse.set_acceptaddress(pChannel->GetAcceptAddress());
			seizeResponse.set_serverid(pChannel->GetServerId());
			seizeResponse.set_mapid(ID_NULL);
		}
	} else {
		seizeResponse.set_acceptaddress(pChannel->GetAcceptAddress());
		seizeResponse.set_serverid(pChannel->GetServerId());
		seizeResponse.set_mapid(ID_NULL);
	}
	response.send(seizeResponse);
}

void CMasterServiceImp::FreeServer(const ::node::FreeRequest& request,
	::rpcz::reply< ::node::FreeResponse> response)
{
	util::CValueStream datas;
	datas.Serialize(NODE_FREE_SERVER);
	CRequestStoredProcs cacheRequest;
	cacheRequest.SetKey(datas);

	datas.Reset();

	uint32_t u32Key = DepartUserId2U32Key(request.userid());
	datas.Serialize(DepartUserId2Index(request.userid()));
	datas.Serialize(u32Key);
	datas.Serialize((uint8_t)request.logout());
	cacheRequest.SetParams(datas);

	CResponseRows cacheResponse;
	eServerError serError = McDBStoredProcHash32Key(
		u32Key, cacheRequest, cacheResponse);

	::node::FreeResponse freeResponse;
	freeResponse.set_result(serError);
	response.send(freeResponse);
}

void CMasterServiceImp::GenerateGuid(const ::node::ControlCentreVoid& request,
	::rpcz::reply< ::node::GuidResponse> response)
{
	::node::GuidResponse guidResponse;
	guidResponse.set_id(CGlobalIDFactory::Pointer()->GenerateID());
	response.send(guidResponse);
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

void CMasterServiceImp::SeizeServerLocal(
	uint32_t& outAgentId,
	uint32_t& outMapId,
	uint64_t userId,
	const std::string& agentServerName)
{
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	CAutoPointer<INodeControl> pNodeModule(pFacade->RetrieveModule(agentServerName));
	if (pNodeModule.IsInvalid()) {
		OutputError("Can't found AgentServer. agentServerName = %s userId = " I64FMTD, agentServerName.c_str(), userId);
		return;
	}
	CAutoPointer<IChannelValue> pChannel(pNodeModule->GetLowLoadUserChnl());
	if (pChannel.IsInvalid()) {
		OutputError("pChannel.IsInvalid() agentServerName = %s userId = " I64FMTD, agentServerName.c_str(), userId);
		return;
	}

	::node::SeizeResponse seizeResponse;

	util::CValueStream datas;
	datas.Serialize(NODE_SEIZE_SERVER);
	CRequestStoredProcs cacheRequest;
	cacheRequest.SetKey(datas);

	datas.Reset();

	uint32_t u32Key = DepartUserId2U32Key(userId);
	datas.Serialize(DepartUserId2Index(userId));
	datas.Serialize(u32Key);
	datas.Serialize(pChannel->GetAcceptAddress());
	datas.Serialize(pChannel->GetServerId());
	datas.Serialize((uint8_t)false);
	cacheRequest.SetParams(datas);

	CResponseRows cacheResponse;
	eServerError serError = McDBStoredProcHash32Key(
		u32Key, cacheRequest, cacheResponse);
	if (SERVER_SUCCESS == serError) {
		int nValuesSize = cacheResponse.GetSize();
		if (nValuesSize > 0) {
			util::CValueStream tsValues(cacheResponse.GetValue(0));
			tsValues.Parse(*(std::string*)seizeResponse.mutable_acceptaddress());
			uint32_t serverId = ID_NULL;
			tsValues.Parse(serverId);
			uint32_t mapId = ID_NULL;
			tsValues.Parse(mapId);
			if (seizeResponse.acceptaddress().empty()) {
				outAgentId = pChannel->GetServerId();
			} else {
				outAgentId = serverId;
			}
			outMapId = mapId;
		} else {
			outAgentId = pChannel->GetServerId();
			outMapId = ID_NULL;
		}
	} else {
		outAgentId = pChannel->GetServerId();
		outMapId = ID_NULL;
	}
}

void CMasterServiceImp::FreeServerLocal(uint64_t userId)
{
	util::CValueStream datas;
	datas.Serialize(NODE_FREE_SERVER);
	CRequestStoredProcs cacheRequest;
	cacheRequest.SetKey(datas);

	datas.Reset();

	uint32_t u32Key = DepartUserId2U32Key(userId);
	datas.Serialize(DepartUserId2Index(userId));
	datas.Serialize(u32Key);
	datas.Serialize((uint8_t)false);
	cacheRequest.SetParams(datas);

	CResponseRows cacheResponse;
	eServerError serError = McDBStoredProcHash32Key(
		u32Key, cacheRequest, cacheResponse);
	if (SERVER_SUCCESS != serError) {
		OutputError("SERVER_SUCCESS != serError "
			"serError = %d userId = " I64FMTD, serError, userId);
	}
}

void CMasterServiceImp::OnServerPlay(void)
{
	if(0 == (int32_t)m_cacheCount) {
		char szError[eBUF_SIZE_256] = {0};
		snprintf(szError, sizeof(szError), "Masterserver must have a CacheServer.");
		throw std::logic_error(szError);
	}
}

