#include "MasterCmdManager.h"
#include "NodeModule.h"
#include "CacheOperateHelper.h"
#include "AppConfig.h"
#include "Log.h"
#include "MasterServiceImp.h"

using namespace util;


int CMasterCmdManager::SendByUserId(
	uint64_t nUserId, int32_t nCmd)
{
	uint32_t serverId = ID_NULL;
	uint32_t mapId = ID_NULL;
	CMasterServiceImp::SeizeServerLocal(serverId, mapId, nUserId, m_agentServerName);
	if(ID_NULL == serverId || ID_NULL == mapId) {
		return SERVER_FAILURE;
	}

	return CNodeModule::SendNodeMessage(serverId, nCmd, nUserId, mapId);
}

int CMasterCmdManager::SendByUserId(
	uint64_t nUserId, int32_t nCmd,
	const ::google::protobuf::Message& request)
{
	uint32_t serverId = ID_NULL;
	uint32_t mapId = ID_NULL;
	CMasterServiceImp::SeizeServerLocal(serverId, mapId, nUserId, m_agentServerName);
	if (ID_NULL == serverId || ID_NULL == mapId) {
		return SERVER_FAILURE;
	}

	return CNodeModule::SendNodeMessage(serverId, nCmd, nUserId, request, mapId);
}

int CMasterCmdManager::SendByUserId(
	uint64_t nUserId, int32_t nCmd,
	const ::google::protobuf::Message& request,
	::google::protobuf::Message& response)
{
	uint32_t serverId = ID_NULL;
	uint32_t mapId = ID_NULL;
	CMasterServiceImp::SeizeServerLocal(serverId, mapId, nUserId, m_agentServerName);
	if (ID_NULL == serverId || ID_NULL == mapId) {
		return SERVER_FAILURE;
	}

	return CNodeModule::SendNodeMessage(serverId, nCmd, nUserId, request, response, mapId);
}

int CMasterCmdManager::SendByUserId(
	uint64_t nUserId, int32_t nCmd,
	const std::string& request,
	std::string &response)
{
	uint32_t serverId = ID_NULL;
	uint32_t mapId = ID_NULL;
	CMasterServiceImp::SeizeServerLocal(serverId, mapId, nUserId, m_agentServerName);
	if (ID_NULL == serverId || ID_NULL == mapId) {
		return SERVER_FAILURE;
	}

	return CNodeModule::SendNodeMessage(serverId, nCmd, nUserId, request, response, mapId);
}

int CMasterCmdManager::Restart(uint32_t nServerId)
{
	CAutoPointer<IChannelValue> rpcChannel(
		CNodeModule::GetStaticChannel(nServerId));
	if(rpcChannel.IsInvalid()) {
		return SERVER_FAILURE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_RESTART);
	servantRequest.set_route(nServerId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	return CNodeModule::SendNodeMessage(rpcChannel, servantRequest);
}

int CMasterCmdManager::Shutdown(uint32_t nServerId)
{
	CAutoPointer<IChannelValue> rpcChannel(
		CNodeModule::GetStaticChannel(nServerId));
	if(rpcChannel.IsInvalid()) {
		return SERVER_FAILURE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_SHUTDOWN);
	servantRequest.set_route(nServerId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	return CNodeModule::SendNodeMessage(rpcChannel, servantRequest);
}

int CMasterCmdManager::Erase(uint32_t nServerId)
{
	CAutoPointer<IChannelValue> rpcChannel(
		CNodeModule::GetStaticChannel(nServerId));
	if(rpcChannel.IsInvalid()) {
		return SERVER_FAILURE;
	}

	if(REGISTER_TYPE_NODE != rpcChannel->GetServerType()) {
		return SERVER_FAILURE;
	}

	if(CMasterServiceImp::IsServerAlive(nServerId, REGISTER_TYPE_NODE)) {
		return SERVER_FAILURE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_ERASE);
	servantRequest.set_route(nServerId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if(CNodeModule::SendNodeMessage(rpcChannel, servantRequest) == TRUE) {
		CNodeModule::RemoveStaticChannel(nServerId);
		return SERVER_SUCCESS;
	}
	return SERVER_FAILURE;
}




