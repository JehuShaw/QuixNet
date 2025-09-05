/*
 * File:   ServantModule.cpp
 * Author: Jehu Shaw
 *
 * Created on 2014_4_28, 13:15
 */
#include "ServantModule.h"

#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ ) || defined( _WIN64 )
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


CServantModule::CServantModule(const char* name, uint32_t serverId)
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
    return std::vector<int>({
		N_CMD_MASTER_TRANSPORT,
		N_CMD_MASTER_RESTART,
		N_CMD_MASTER_SHUTDOWN,
		N_CMD_MASTER_ERASE,
		N_CMD_SERVER_PLAY,
		N_CMD_SERVER_STOP,
		N_CMD_SERVER_STORE
    });
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
		OutputError("rpcChannel.IsInvalid() routeType = %d route = " I64FMTD,
			request.route_type(), request.route());
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::CacheService_Stub cacheService_stub(&*rpcChannel->GetRpcChannel());

	rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);

	cacheService_stub.HandleNotification(request, &response, &controller, NULL);
	controller.wait();

	const rpcz::status_code curStatus = controller.get_status();

	if (curStatus == rpcz::status::DEADLINE_EXCEEDED) {
		response.set_result(SERVER_CALL_DEADLINE);
	}

	if (curStatus != rpcz::status::OK) {
		OutputError("controller.get_status() = %d routeType = %d route = " I64FMTD,
			controller.get_status(), request.route_type(), request.route());
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

	uint32_t serverId = static_cast<uint32_t>(pRequest->route());
	CAutoPointer<IChannelValue> rpcChannel(CNodeModule::GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid() serverId = %u", serverId);
		pResponse->set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		return;
	}

	if(rpcChannel->GetServerType() != REGISTER_TYPE_WORKER) {
		// Only Worker can transport.
		OutputError("GetServerType() != REGISTER_TYPE_WORKER GetServerType() = %d serverId = %u",
			rpcChannel->GetServerType(), serverId);
		pResponse->set_result(SERVER_FAILURE);
		return;
	}

	pResponse->set_result(CNodeModule::SendNodeMessage(rpcChannel, *pRequest, *pResponse));
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

	uint32_t serverId = static_cast<uint32_t>(pRequest->route());
	CAutoPointer<IChannelValue> rpcChannel(CNodeModule::GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid() serverId = %u", serverId);
		pResponse->set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		return;
	}

	if(CServantServiceImp::IsServerAlive(serverId)) {
		OutputError("IsServerAlive() serverId = %u", serverId);
		pResponse->set_result(SERVER_FAILURE);
		return;
	}

	const std::string& strProcessPath = rpcChannel->GetProcessPath();
	char szCurDir[MAX_PATH] = {'\0'};
	getcwd(szCurDir, sizeof(szCurDir));
	std::string::size_type nPos = strProcessPath.find_last_of('/');
	if(nPos == std::string::npos) {
		OutputError("nPos == std::string::npos serverId = %u", serverId);
		pResponse->set_result(SERVER_FAILURE);
		return;
	}
	std::string strProcessDir(strProcessPath.substr(0, nPos));
	chdir(strProcessDir.c_str());

	std::string strCmd;
#if defined( WIN32 ) || defined( _WIN32 ) || defined( __WIN32__ ) || defined( _WIN64 )
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
	pResponse->set_result(SERVER_SUCCESS);
}

void CServantModule::CaseShutdown(const util::CWeakPointer<mdl::INotification>& request,
	util::CWeakPointer<mdl::IResponse>& reply)
{
	util::CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if (pRequest.IsInvalid()) {
		return;
	}

	// response
	util::CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if (pResponse.IsInvalid()) {
		return;
	}

	uint32_t serverId = static_cast<uint32_t>(pRequest->route());
	CAutoPointer<IChannelValue> rpcChannel(CNodeModule::GetStaticChannel(serverId));
	if (rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid() serverId = %u", serverId);
		pResponse->set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		return;
	}

	pResponse->set_result(CNodeModule::SendNodeMessage(rpcChannel, *pRequest));
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

	uint32_t serverId = static_cast<uint32_t>(pRequest->route());
	if(CServantServiceImp::IsServerAlive(serverId)) {
		OutputError("IsServerAlive() serverId = %u", serverId);
		pResponse->set_result(SERVER_FAILURE);
		return;
	}

	CNodeModule::RemoveStaticChannel(serverId);
	pResponse->set_result(SERVER_SUCCESS);
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

	uint32_t serverId = static_cast<uint32_t>(pRequest->route());
	CAutoPointer<IChannelValue> rpcChannel(CNodeModule::GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid() serverId = %u", serverId);
		pResponse->set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		return;
	}

	if(rpcChannel->GetServerType() != REGISTER_TYPE_CACHE) {
		OutputError("GetServerType() != REGISTER_TYPE_CACHE GetServerType() = %d serverId = %u ",
			rpcChannel->GetServerType(), serverId);
		pResponse->set_result(SERVER_FAILURE);
		return;
	}

	::node::DataPacket cacheRequest;
	if(!ParseCacheData(cacheRequest, pRequest)) {
		OutputError("!ParseWorkerData serverId = %u", serverId);
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

	const uint32_t serverId = static_cast<uint32_t>(pRequest->route());
	if(serverId == m_serverId) {
		// This server
		atomic_xchg(&g_serverStatus, SERVER_STATUS_START);
		pResponse->set_result(SERVER_SUCCESS);
		return;
	}

	CAutoPointer<IChannelValue> rpcChannel(CNodeModule::GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid() serverId = %u", serverId);
		pResponse->set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
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

	const uint32_t serverId = static_cast<uint32_t>(pRequest->route());
	if(serverId == m_serverId) {
		// This server
		atomic_xchg(&g_serverStatus, SERVER_STATUS_STOP);
		pResponse->set_result(SERVER_SUCCESS);
		return;
	}

	CAutoPointer<IChannelValue> rpcChannel(CNodeModule::GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid() serverId = %u", serverId);
		pResponse->set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		return;
	}

	const int nServerType = rpcChannel->GetServerType();
	if(nServerType == REGISTER_TYPE_WORKER) {
		CNodeModule::SendNodeMessage(rpcChannel, *pRequest, *pResponse);
	} else if(nServerType == REGISTER_TYPE_CACHE) {
		SendCacheMessage(rpcChannel, *pRequest, *pResponse);
	}
}