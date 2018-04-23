#include "MasterCmdManager.h"
#include "NodeModule.h"
#include "CacheOperateHelper.h"
#include "AppConfig.h"
#include "Log.h"
#include "MasterServiceImp.h"

using namespace util;


int CMasterCmdManager::SendByUserId(
	const std::string& strServerName,
	uint64_t nUserId, int32_t nCmd)
{
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	CAutoPointer<CNodeModule> pNodeModule(pFacade->RetrieveModule(strServerName));
	if(pNodeModule.IsInvalid()) {
		return FALSE;
	}

	uint16_t serverId = CMasterServiceImp::RouteGetServerId(strServerName, nUserId);
	if(ID_NULL == serverId) {
		return FALSE;
	}

	return pNodeModule->SendNotification(serverId, nCmd);
}

int CMasterCmdManager::SendByUserId(
	const std::string& strServerName,
	uint64_t nUserId, int32_t nCmd,
	const ::google::protobuf::Message& message)
{
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	CAutoPointer<CNodeModule> pNodeModule(pFacade->RetrieveModule(strServerName));
	if(pNodeModule.IsInvalid()) {
		return FALSE;
	}

	uint16_t serverId = CMasterServiceImp::RouteGetServerId(strServerName, nUserId);
	if(ID_NULL == serverId) {
		return FALSE;
	}

	return pNodeModule->SendNotification(serverId, nCmd, nUserId, message);
}

int CMasterCmdManager::SendByUserId(
	const std::string& strServerName,
	uint64_t nUserId, int32_t nCmd,
	const std::string& data)
{
	mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
	CAutoPointer<CNodeModule> pNodeModule(pFacade->RetrieveModule(strServerName));
	if(pNodeModule.IsInvalid()) {
		return FALSE;
	}

	uint16_t serverId = CMasterServiceImp::RouteGetServerId(strServerName, nUserId);
	if(ID_NULL == serverId) {
		return FALSE;
	}

	return pNodeModule->SendNotification(serverId, nCmd, nUserId, data);
}

int CMasterCmdManager::Restart(uint16_t nServerId)
{
	CAutoPointer<IChannelValue> rpcChannel(
		CNodeModule::GetStaticChannel(nServerId));
	if(rpcChannel.IsInvalid()) {
		return FALSE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_RESTART);
	servantRequest.set_route(nServerId);

	return CNodeModule::SendNodeMessage(rpcChannel, servantRequest);
}

int CMasterCmdManager::Shutdown(uint16_t nServerId)
{
	CAutoPointer<IChannelValue> rpcChannel(
		CNodeModule::GetStaticChannel(nServerId));
	if(rpcChannel.IsInvalid()) {
		return FALSE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_SHUTDOWN);
	servantRequest.set_route(nServerId);

	return CNodeModule::SendNodeMessage(rpcChannel, servantRequest);
}

int CMasterCmdManager::Erase(uint16_t nServerId)
{
	CAutoPointer<IChannelValue> rpcChannel(
		CNodeModule::GetStaticChannel(nServerId));
	if(rpcChannel.IsInvalid()) {
		return FALSE;
	}

	if(REGISTER_TYPE_NODE != rpcChannel->GetServerType()) {
		return FALSE;
	}

	if(CMasterServiceImp::IsServerAlive(nServerId, REGISTER_TYPE_NODE)) {
		return FALSE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_ERASE);
	servantRequest.set_route(nServerId);

	if(CNodeModule::SendNodeMessage(rpcChannel, servantRequest) == TRUE) {
		CNodeModule::RemoveStaticChannel(nServerId);
		return TRUE;
	}
	return FALSE;
}




