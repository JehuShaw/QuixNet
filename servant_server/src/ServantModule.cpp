/*
 * File:   ServantModule.cpp
 * Author: Jehu Shaw
 *
 * Created on 2014_4_28, 13:15
 */
#include "ServantModule.h"

#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ )
#include <process.h>
#include <direct.h>
#include <ShellAPI.h>
#else
#include <unistd.h>
#endif

#include "ServantServiceImp.h"
#include "SpinRWLock.h"
#include "worker.rpcz.h"
#include "ChannelManager.h"
#include "rpc_controller.hpp"
#include "BodyMessage.h"
#include "Log.h"
#include "msg_master_transport.pb.h"
#include "WorkerOperateHelper.h"
#include "NodeDefines.h"
#include "NodeModule.h"

#include "CacheOperateHelper.h"

using namespace mdl;
using namespace rpcz;
using namespace util;


CServantModule::CServantModule(const char* name, uint16_t serverId)
	: CModule(name)
	, m_serverId(serverId)
{
}

CServantModule::~CServantModule(void)
{
}

void CServantModule::OnRegister()
{

}

void CServantModule::OnRemove()
{

}

std::vector<int> CServantModule::ListNotificationInterests()
{
    std::vector<int> interests;
    interests.push_back(N_CMD_MASTER_TRANSPORT);
	interests.push_back(N_CMD_MASTER_RESTART);
	interests.push_back(N_CMD_MASTER_SHUTDOWN);
	interests.push_back(N_CMD_MASTER_ERASE);
	interests.push_back(N_CMD_SERVER_PLAY);
	interests.push_back(N_CMD_SERVER_STOP);
	interests.push_back(N_CMD_SERVER_STORE);
    return interests;
}

IModule::InterestList CServantModule::ListProtocolInterests()
{
	return InterestList();
}

void CServantModule::HandleNotification(const util::CWeakPointer<INotification>& request,
	util::CWeakPointer<IResponse>& reply)
{
    int32_t nCmd = request->GetName();
    switch(nCmd) {
    case N_CMD_MASTER_TRANSPORT:
        CaseTransport(request, reply);
        break;
	case N_CMD_MASTER_RESTART:
		CaseRestart(request, reply);
		break;
	case N_CMD_MASTER_SHUTDOWN:
		CaseShutdown(request, reply);
		break;
	case N_CMD_MASTER_ERASE:
		CaseErase(request, reply);
		break;
	case N_CMD_SERVER_PLAY:
		CasePlay(request,reply);
		break;
	case N_CMD_SERVER_STOP:
		CaseStop(request,reply);
		break;
	case N_CMD_SERVER_STORE:
		CaseCacheStore(request, reply);
		break;
    default:
        break;
    }
}

int CServantModule::SendCacheMessage(
	CAutoPointer<IChannelValue>& rpcChannel,
	const ::node::DataPacket& request,
	::node::DataPacket& response)
{

	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid()");
		return FALSE;
	}

	::node::CacheService_Stub cacheService_stub(&*rpcChannel->GetRpcChannel());

	rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);

	cacheService_stub.HandleNotification(request, &response, &controller, NULL);
	controller.wait();

	if(!controller.ok()) {
		OutputError("controller.get_status() = %d", controller.get_status());
	}

	return response.result();
}

void CServantModule::CaseTransport(const util::CWeakPointer<mdl::INotification>& request,
    util::CWeakPointer<mdl::IResponse>& reply)
{
    util::CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
    if(pRequest.IsInvalid()) {
        return;
    }

	// response
	util::CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if(pResponse.IsInvalid()) {
		return;
	}

	uint16_t serverId = (uint16_t)pRequest->route();
	CAutoPointer<IChannelValue> rpcChannel(CNodeModule::GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid()");
		pResponse->set_result(FALSE);
		return;
	}

	if(rpcChannel->GetServerType() != REGISTER_TYPE_WORKER) {
		// Only Worker can transport.
		return;
	}

	int32_t nResult = CNodeModule::SendNodeMessage(rpcChannel, *pRequest);
	pResponse->set_result(nResult);
}

void CServantModule::CaseRestart(const util::CWeakPointer<mdl::INotification>& request,
	util::CWeakPointer<mdl::IResponse>& reply)
{
	util::CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if(pRequest.IsInvalid()) {
		return;
	}

	// response
	util::CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if(pResponse.IsInvalid()) {
		return;
	}

	uint16_t serverId = (uint16_t)pRequest->route();
	CAutoPointer<IChannelValue> rpcChannel(CNodeModule::GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid()");
		pResponse->set_result(FALSE);
		return;
	}

	if(CServantServiceImp::IsServerAlive(serverId)) {
		pResponse->set_result(FALSE);
		return;
	}

	const std::string& strProcessPath = rpcChannel->GetProcessPath();
	char szCurDir[MAX_PATH] = {'\0'};
	getcwd(szCurDir, sizeof(szCurDir));
	std::string::size_type nPos = strProcessPath.find_last_of('/');
	if(nPos == std::string::npos) {
		OutputError("nPos == std::string::npos ");
		pResponse->set_result(FALSE);
		return;
	}
	std::string strProcessDir(strProcessPath.substr(0, nPos));
	chdir(strProcessDir.c_str());

	std::string strCmd;
#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ )
	strCmd = "start \"\" \"";
	strCmd += strProcessPath;
	strCmd += '\"';
#else
	// Under Gnome, it's gnome-terminal.
	// Under KDE, it's konsole.
	strCmd = "xterm -e \"";
	strCmd += strProcessPath;
	strCmd += '\"';
#endif

	system(strCmd.c_str());
	chdir(szCurDir);
	pResponse->set_result(TRUE);
}

void CServantModule::CaseShutdown(const util::CWeakPointer<mdl::INotification>& request,
	util::CWeakPointer<mdl::IResponse>& reply)
{
	util::CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if(pRequest.IsInvalid()) {
		return;
	}

	// response
	util::CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if(pResponse.IsInvalid()) {
		return;
	}

	uint16_t serverId = (uint16_t)pRequest->route();
	CAutoPointer<IChannelValue> rpcChannel(CNodeModule::GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid()");
		pResponse->set_result(FALSE);
		return;
	}

	int32_t nResult = CNodeModule::SendNodeMessage(rpcChannel, *pRequest);
	pResponse->set_result(nResult);
}

void CServantModule::CaseErase(const util::CWeakPointer<mdl::INotification>& request,
	util::CWeakPointer<mdl::IResponse>& reply)
{
	util::CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if(pRequest.IsInvalid()) {
		return;
	}

	// response
	util::CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if(pResponse.IsInvalid()) {
		return;
	}

	uint16_t serverId = (uint16_t)pRequest->route();
	if(CServantServiceImp::IsServerAlive(serverId)) {
		pResponse->set_result(FALSE);
		return;
	}

	CNodeModule::RemoveStaticChannel(serverId);
	pResponse->set_result(TRUE);
}

void CServantModule::CaseCacheStore(const util::CWeakPointer<mdl::INotification>& request,
	util::CWeakPointer<mdl::IResponse>& reply)
{
	util::CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if(pRequest.IsInvalid()) {
		return;
	}

	// response
	util::CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if(pResponse.IsInvalid()) {
		return;
	}

	uint16_t serverId = (uint16_t)pRequest->route();
	CAutoPointer<IChannelValue> rpcChannel(CNodeModule::GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid()");
		pResponse->set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		return;
	}

	if(rpcChannel->GetServerType() != REGISTER_TYPE_CACHE) {
		OutputError("rpcChannel->GetServerType() != REGISTER_TYPE_CACHE");
		pResponse->set_result(CACHE_ERROR_NOTFOUND);
		return;
	}

	::node::DataPacket cacheRequest;
	if(!ParseCacheData(cacheRequest, pRequest)) {
		OutputError("!ParseWorkerData serverId = %d", serverId);
		pResponse->set_result(PARSE_PACKAGE_FAIL);
		return;
	}

	SendCacheMessage(rpcChannel, cacheRequest, *pResponse);
}

void CServantModule::CasePlay(const util::CWeakPointer<mdl::INotification>& request,
	util::CWeakPointer<mdl::IResponse>& reply)
{
	util::CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if(pRequest.IsInvalid()) {
		return;
	}

	// response
	util::CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if(pResponse.IsInvalid()) {
		return;
	}

	const uint16_t serverId = (uint16_t)pRequest->route();
	if(serverId == m_serverId) {
		// This server
		atomic_xchg(&g_serverStatus, SERVER_STATUS_START);
		pResponse->set_result(TRUE);
		return;
	}

	CAutoPointer<IChannelValue> rpcChannel(CNodeModule::GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid()");
		pResponse->set_result(FALSE);
		return;
	}

	const int nServerType = rpcChannel->GetServerType();
	if(nServerType == REGISTER_TYPE_WORKER) {
		CNodeModule::SendNodeMessage(rpcChannel, *pRequest, *pResponse);
	} else if(nServerType == REGISTER_TYPE_CACHE) {
		SendCacheMessage(rpcChannel, *pRequest, *pResponse);
	}
}

void CServantModule::CaseStop(const util::CWeakPointer<mdl::INotification>& request,
	util::CWeakPointer<mdl::IResponse>& reply)
{
	util::CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if(pRequest.IsInvalid()) {
		return;
	}

	// response
	util::CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if(pResponse.IsInvalid()) {
		return;
	}

	const uint16_t serverId = (uint16_t)pRequest->route();
	if(serverId == m_serverId) {
		// This server
		atomic_xchg(&g_serverStatus, SERVER_STATUS_STOP);
		pResponse->set_result(TRUE);
		return;
	}

	CAutoPointer<IChannelValue> rpcChannel(CNodeModule::GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid()");
		pResponse->set_result(FALSE);
		return;
	}

	const int nServerType = rpcChannel->GetServerType();
	if(nServerType == REGISTER_TYPE_WORKER) {
		CNodeModule::SendNodeMessage(rpcChannel, *pRequest, *pResponse);
	} else if(nServerType == REGISTER_TYPE_CACHE) {
		SendCacheMessage(rpcChannel, *pRequest, *pResponse);
	}
}