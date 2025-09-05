/*
 * File:   NodeModule.cpp
 * Author: Jehu Shaw
 *
 * Created on 2014_4_28, 13:15
 */
#include "NodeModule.h"
#include "SpinRWLock.h"
#include "worker.rpcz.h"
#include "ChannelManager.h"
#include "rpc_controller.hpp"
#include "BodyMessage.h"
#include "Log.h"
#include "NodeChannel.h"

using namespace mdl;
using namespace rpcz;
using namespace util;
using namespace thd;


class CFindLowLoadNode : public IChannelValue {
public:
	CFindLowLoadNode(uint16_t serverRegion)
		: m_serverRegion(serverRegion)
	{}

	~CFindLowLoadNode() {}

	virtual uint32_t GetServerId() const {
		return 0;
	}

	virtual int32_t GetServerLoad() const {
		return 0;
	}

	virtual uint16_t GetServerRegion() const {
		return m_serverRegion;
	}

	virtual const std::string& GetEndPoint() const {
		throw std::runtime_error("Method const std::string& GetEndPoint() not implemented.");
	}

	virtual uint16_t GetServerType() const {
		throw std::runtime_error("Method uint16_t GetServerType() not implemented.");
	}

	virtual int GetIndex() const {
		throw std::runtime_error("Method int GetIndex() not implemented.");
	}

protected:
	virtual void SetServerId(uint32_t nValue) {
		throw std::runtime_error("Method SetServerId(uint32_t nValue) not implemented.");
	}

	virtual void SetEndPoint(const std::string& strValue) {
		throw std::runtime_error("Method SetEndPoint(const std::string& strValue) not implemented.");
	}

	virtual void SetServerType(uint16_t nValue) {
		throw std::runtime_error("Method SetServerType(uint16_t nValue) not implemented.");
	}

	virtual void SetIndex(int nIdx) {
		throw std::runtime_error("Method SetIndex(int nIdx) not implemented.");
	}

private:
	uint16_t m_serverRegion;
};

thd::CSpinRWLock CNodeModule::s_rwTicket;
CNodeModule::SERVERID_TO_CHANNEL_T CNodeModule::s_serIdChannels;

CNodeModule::CNodeModule(
	const std::string& moduleName,
	const std::string& endPoint,
	uint32_t serverId,
	uint16_t serverType,
	const std::string& acceptAddress,
	const std::string& processPath,
	const std::string& projectName,
	uint16_t serverRegion)

	: INodeControl(moduleName)
	, m_lessQueue(CAutoPointer<CNodeComparer>(new CNodeComparer))
{
	CreatChannel(serverId, endPoint, serverType, acceptAddress,
		processPath, projectName, serverRegion);
}

CNodeModule::~CNodeModule(void)
{
}

void CNodeModule::OnRegister()
{
}

void CNodeModule::OnRemove()
{
}

std::vector<int> CNodeModule::ListNotificationInterests()
{
    return std::vector<int>();
}

IModule::InterestList CNodeModule::ListProtocolInterests()
{
	return InterestList();
}

void CNodeModule::HandleNotification(const util::CWeakPointer<INotification>& request,
	util::CWeakPointer<IResponse>& reply)
{
}

bool CNodeModule::CreatChannel(uint32_t serverId, const std::string& endPoint, uint16_t serverType,
	const std::string& acceptAddress, const std::string& processPath, const std::string& projectName,
	uint16_t serverRegion)
{
	CAutoPointer<IChannelValue> rpcChannel;
	if(!InsertSerIdChannel(serverId, serverType, endPoint, acceptAddress, processPath, projectName,
		serverRegion, rpcChannel)) {
		return false;
	}
	AddStaticChannel(serverId, rpcChannel);
	return true;
}

CAutoPointer<IChannelValue> CNodeModule::GetChnlByDirServId(uint32_t serverId) const
{
	CScopedReadLock readLock(m_rwServer);

	SERVERID_TO_CHANNEL_T::const_iterator itU(
		m_serIdChannels.find(serverId));

	if(m_serIdChannels.end() == itU) {
		return CAutoPointer<IChannelValue>();
	}
	return itU->second;
}

bool CNodeModule::RemoveChannel(uint32_t serverId)
{
	ServantRemoveStaticChannel(serverId);
	if(!EraseSerIdChannel(serverId)) {
		return false;
	}
	return true;
}

void CNodeModule::UpdateChannelLoad(uint32_t serverId, int32_t serverLoad)
{
    CAutoPointer<IChannelValue> pNodeChannel(GetChnlByDirServId(serverId));
    if(pNodeChannel.IsInvalid()) {
        return;
    }

	if(pNodeChannel->GetServerLoad() == serverLoad) {
		return;
	}

	CScopedWriteLock wRegLock(m_rwRegion);
	m_sortRegion.erase(pNodeChannel);
	pNodeChannel->XchgServerLoad(serverLoad);
	m_sortRegion.insert(pNodeChannel);
	wRegLock.Unlock();

	CScopedWriteLock wQueLock(m_rwQueue);
	CAutoPointer<CNodeChannel> pSubClass(pNodeChannel);
	if (pSubClass.IsInvalid()) {
		OutputError("pSubClass.IsInvalid()");
		assert(false);
	}
	m_lessQueue.Update(pSubClass);
	wQueLock.Unlock();	
}

int CNodeModule::ChannelCount() const
{
	CScopedReadLock readLock(m_rwServer);
	return (int)m_serIdChannels.size();
}

CAutoPointer<IChannelValue> CNodeModule::GetLowLoadUserChnl() const
{
	CScopedReadLock rQueLock(m_rwQueue);
	return CAutoPointer<CNodeChannel>(m_lessQueue.Peek());
}

CAutoPointer<IChannelValue> CNodeModule::GetLowLoadByRegion(uint16_t serverRegion) const
{
	CScopedReadLock readLock(m_rwRegion);
	if(m_sortRegion.empty()) {
		return CAutoPointer<IChannelValue>();
	}

	CFindLowLoadNode findSortNode(serverRegion);
	CAutoPointer<CFindLowLoadNode> pFindSortNode(
		&findSortNode, false);
	SORT_REGION_T::const_iterator it(
		m_sortRegion.upper_bound(pFindSortNode));

	if(m_sortRegion.end() == it) {
		 return CAutoPointer<IChannelValue>();
	}

	if((*it)->GetServerRegion() != serverRegion) {
		return CAutoPointer<IChannelValue>();
	}
	return *it;
}

void CNodeModule::IterateChannel(std::vector<CAutoPointer<IChannelValue> >& outChannels) const
{
	CScopedReadLock readLock(m_rwServer);
	SERVERID_TO_CHANNEL_T::const_iterator itU(m_serIdChannels.begin());
	for(; m_serIdChannels.end() != itU; ++itU) {
		outChannels.push_back(itU->second);
	}
}

bool CNodeModule::InsertSerIdChannel(
	uint32_t serverId,
	uint16_t serverType,
	const std::string& endPoint,
	const std::string& acceptAddress,
	const std::string& processPath,
	const std::string& projectName,
	uint16_t serverRegion,
	util::CAutoPointer<IChannelValue>& outRpcChannel)
{
	CAutoPointer<IChannelValue> rpcChannel;

	CScopedWriteLock wSerLock(m_rwServer);
	std::pair<SERVERID_TO_CHANNEL_T::iterator, bool> pairIBA(
		m_serIdChannels.insert(SERVERID_TO_CHANNEL_T::value_type(
			serverId, rpcChannel)));
	if (!pairIBA.second) {
		return false;
	}
	if ((uint16_t)REGISTER_TYPE_SERVANT == serverType) {
		pairIBA.first->second.SetRawPointer(new CServantChannel);
	}
	else {
		pairIBA.first->second.SetRawPointer(new CNodeChannel);
	}
	rpcChannel = pairIBA.first->second;
	if (rpcChannel.IsInvalid()) {
		return false;
	}
	rpcChannel->XchgServerLoad(0);
	rpcChannel->SetTimeoutCount(0);
	rpcChannel->SetAcceptAddress(acceptAddress);
	rpcChannel->SetEndPoint(endPoint);
	rpcChannel->SetServerId(serverId);
	rpcChannel->SetServerType(serverType);
	rpcChannel->SetProcessPath(processPath);
	rpcChannel->SetProjectName(projectName);
	rpcChannel->SetServerRegion(serverRegion);
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	rpcChannel->SetRpcChannel(pChlMgr->GetRpczChannel(endPoint));
	wSerLock.Unlock();

	CScopedWriteLock wRegionLock(m_rwRegion);
	m_sortRegion.insert(rpcChannel);
	wRegionLock.Unlock();

	CScopedWriteLock wQueLock(m_rwQueue);
	CAutoPointer<CNodeChannel> pSubClass(rpcChannel);
	if (pSubClass.IsInvalid()) {
		OutputError("pSubClass.IsInvalid()");
		assert(false);
	}
	m_lessQueue.Push(pSubClass);
	wQueLock.Unlock();

	outRpcChannel = rpcChannel;
	return true;
}

bool CNodeModule::EraseSerIdChannel(uint32_t serverId)
{
	util::CAutoPointer<IChannelValue> pChannelValue;

	CScopedWriteLock wSerLock(m_rwServer);
	SERVERID_TO_CHANNEL_T::const_iterator
		itU(m_serIdChannels.find(serverId));

	if (m_serIdChannels.end() == itU) {
		return false;
	}
	pChannelValue = itU->second;
	m_serIdChannels.erase(itU);
	wSerLock.Unlock();

	CScopedWriteLock wQueLock(m_rwQueue);
	CAutoPointer<CNodeChannel> pSubClass(pChannelValue);
	if (pSubClass.IsInvalid()) {
		OutputError("pSubClass.IsInvalid()");
		assert(false);
	}
	m_lessQueue.Remove(pSubClass);
	wQueLock.Unlock();

	CScopedWriteLock wRegLock(m_rwRegion);
	m_sortRegion.erase(pChannelValue);
	wRegLock.Unlock();
	return true;
}

int CNodeModule::SendNotification(uint32_t serverId, int32_t cmd) {
	CAutoPointer<IChannelValue> rpcChannel(GetChnlByDirServId(serverId));
	if(rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route_type(ROUTE_DIRECT_SERVERID);
	workerRequest.set_route(ID_NULL);

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	return SendNotification(rpcChannel, servantRequest);
}

int CNodeModule::SendNotification(uint32_t serverId, int32_t cmd, const ::google::protobuf::Message& request)
{
	CAutoPointer<IChannelValue> rpcChannel(GetChnlByDirServId(serverId));
	if(rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route_type(ROUTE_DIRECT_SERVERID);
	workerRequest.set_route(ID_NULL);

	if (!request.SerializeToString(workerRequest.mutable_data())) {
		OutputError("!request.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	return SendNotification(rpcChannel, servantRequest);
}

int CNodeModule::SendNotification(uint32_t serverId, int32_t cmd, const ::google::protobuf::Message& request, ::google::protobuf::Message& response)
{
	CAutoPointer<IChannelValue> rpcChannel(GetChnlByDirServId(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route_type(ROUTE_DIRECT_SERVERID);
	workerRequest.set_route(ID_NULL);

	if (!request.SerializeToString(workerRequest.mutable_data())) {
		OutputError("!request.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantResponse;
	SendNotification(rpcChannel, servantRequest, servantResponse);

	if (!response.ParseFromString(servantResponse.data())) {
		OutputError("!response.ParseFromString");
		return PARSE_PACKAGE_FAIL;
	}
	return servantResponse.result();
}

int CNodeModule::SendNotification(uint32_t serverId, int32_t cmd, const std::string& request, std::string& response)
{
	CAutoPointer<IChannelValue> rpcChannel(GetChnlByDirServId(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route_type(ROUTE_DIRECT_SERVERID);
	workerRequest.set_route(ID_NULL);
	if (!request.empty()) {
		workerRequest.set_data(request);
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantResponse;
	SendNotification(rpcChannel, servantRequest, servantResponse);

	response = servantResponse.data();

	return servantResponse.result();
}

int CNodeModule::SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId)
{
	CAutoPointer<IChannelValue> rpcChannel(GetChnlByDirServId(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	return SendNotification(rpcChannel, servantRequest);
}

int CNodeModule::SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request)
{
	CAutoPointer<IChannelValue> rpcChannel(GetChnlByDirServId(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	if (!request.SerializeToString(workerRequest.mutable_data())) {
		OutputError("!request.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	return SendNotification(rpcChannel, servantRequest);
}

int CNodeModule::SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request, ::google::protobuf::Message& response)
{
	CAutoPointer<IChannelValue> rpcChannel(GetChnlByDirServId(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	if (!request.SerializeToString(workerRequest.mutable_data())) {
		OutputError("!request.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantResponse;
	SendNotification(rpcChannel, servantRequest, servantResponse);

	if (!response.ParseFromString(servantResponse.data())) {
		OutputError("!response.ParseFromString");
		return PARSE_PACKAGE_FAIL;
	}
	return servantResponse.result();
}


int CNodeModule::SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, const std::string& request, std::string& response)
{
	CAutoPointer<IChannelValue> rpcChannel(GetChnlByDirServId(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	if (!request.empty()) {
		workerRequest.set_data(request);
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantResponse;
	SendNotification(rpcChannel, servantRequest, servantResponse);

	response = servantResponse.data();

	return servantResponse.result();
}

int CNodeModule::SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, int32_t subcmd)
{
	CAutoPointer<IChannelValue> rpcChannel(GetChnlByDirServId(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);
	servantRequest.set_sub_cmd(subcmd);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	return SendNotification(rpcChannel, servantRequest);
}

int CNodeModule::SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request, int32_t subcmd)
{
	CAutoPointer<IChannelValue> rpcChannel(GetChnlByDirServId(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	if (!request.SerializeToString(workerRequest.mutable_data())) {
		OutputError("!request.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);
	servantRequest.set_sub_cmd(subcmd);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	return SendNotification(rpcChannel, servantRequest);
}

int CNodeModule::SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request, ::google::protobuf::Message& response, int32_t subcmd)
{
	CAutoPointer<IChannelValue> rpcChannel(GetChnlByDirServId(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	if (!request.SerializeToString(workerRequest.mutable_data())) {
		OutputError("!request.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);
	servantRequest.set_sub_cmd(subcmd);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantResponse;
	SendNotification(rpcChannel, servantRequest, servantResponse);

	if (!response.ParseFromString(servantResponse.data())) {
		OutputError("!response.ParseFromString");
		return PARSE_PACKAGE_FAIL;
	}
	return servantResponse.result();
}

int CNodeModule::SendNotification(uint32_t serverId, int32_t cmd, uint64_t userId, const std::string& request, std::string& response, int32_t subcmd)
{
	CAutoPointer<IChannelValue> rpcChannel(GetChnlByDirServId(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	if (!request.empty()) {
		workerRequest.set_data(request);
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);
	servantRequest.set_sub_cmd(subcmd);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantResponse;
	SendNotification(rpcChannel, servantRequest, servantResponse);

	response = servantResponse.data();

	return servantResponse.result();
}

int CNodeModule::SendNotification(CAutoPointer<IChannelValue>& rpcChannel, const ::node::DataPacket& request)
{
	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid()");
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::WorkerService_Stub workerService_stub(&*rpcChannel->GetRpcChannel());

	rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);
	::node::DataPacket dpResponse;
	workerService_stub.HandleNotification(request, &dpResponse, &controller, NULL);
	controller.wait();

	const rpcz::status_code curStatus = controller.get_status();

	if (curStatus == rpcz::status::DEADLINE_EXCEEDED) {
		return SERVER_CALL_DEADLINE;
	}

	if(curStatus != rpcz::status::OK) {
		if(rpcChannel->IncTimeoutCount()
			>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
		{
			EraseSerIdChannel(rpcChannel->GetServerId());
		}
	} else {
		rpcChannel->SetTimeoutCount(0);
	}

	return dpResponse.result();
}

int CNodeModule::SendNotification(CAutoPointer<IChannelValue>& rpcChannel, const ::node::DataPacket& request, ::node::DataPacket& response)
{
	if (rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid()");
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::WorkerService_Stub workerService_stub(&*rpcChannel->GetRpcChannel());

	rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);

	workerService_stub.HandleNotification(request, &response, &controller, NULL);
	controller.wait();

	const rpcz::status_code curStatus = controller.get_status();

	if (curStatus == rpcz::status::DEADLINE_EXCEEDED) {
		return SERVER_CALL_DEADLINE;
	}

	if (curStatus != rpcz::status::OK) {
		if (rpcChannel->IncTimeoutCount()
			>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
		{
			EraseSerIdChannel(rpcChannel->GetServerId());
		}
	}
	else {
		rpcChannel->SetTimeoutCount(0);
	}

	return response.result();
}

int CNodeModule::SendNodeMessage(uint32_t serverId, int32_t cmd)
{
	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route_type(ROUTE_DIRECT_SERVERID);
	workerRequest.set_route(ID_NULL);

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	return SendNodeMessage(rpcChannel, servantRequest);
}

int CNodeModule::SendNodeMessage(uint32_t serverId, int32_t cmd, const ::google::protobuf::Message& request)
{
	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route_type(ROUTE_DIRECT_SERVERID);
	workerRequest.set_route(ID_NULL);

	if (!request.SerializeToString(workerRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	return SendNodeMessage(rpcChannel, servantRequest);
}

int CNodeModule::SendNodeMessage(uint32_t serverId, int32_t cmd, const ::google::protobuf::Message& request, ::google::protobuf::Message& response)
{
	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route_type(ROUTE_DIRECT_SERVERID);
	workerRequest.set_route(ID_NULL);

	if (!request.SerializeToString(workerRequest.mutable_data())) {
		OutputError("!request.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantResponse;
	SendNodeMessage(rpcChannel, servantRequest, servantResponse);

	if (!response.ParseFromString(servantResponse.data())) {
		OutputError("!response.ParseFromString");
		return PARSE_PACKAGE_FAIL;
	}
	return servantResponse.result();
}

int CNodeModule::SendNodeMessage(uint32_t serverId, int32_t cmd, const std::string& request, std::string& response)
{
	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route_type(ROUTE_DIRECT_SERVERID);
	workerRequest.set_route(ID_NULL);

	if (!request.empty()) {
		workerRequest.set_data(request);
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantResponse;
	SendNodeMessage(rpcChannel, servantRequest, servantResponse);

	response = servantResponse.data();

	return servantResponse.result();
}

int CNodeModule::SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId)
{
	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	return SendNodeMessage(rpcChannel, servantRequest);
}

int CNodeModule::SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request)
{
	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	if (!request.SerializeToString(workerRequest.mutable_data())) {
		OutputError("!request.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	return SendNodeMessage(rpcChannel, servantRequest);
}

int CNodeModule::SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request, ::google::protobuf::Message& response)
{
	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	if (!request.SerializeToString(workerRequest.mutable_data())) {
		OutputError("!request.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantResponse;
	SendNodeMessage(rpcChannel, servantRequest, servantResponse);

	if (!response.ParseFromString(servantResponse.data())) {
		OutputError("!response.ParseFromString");
		return PARSE_PACKAGE_FAIL;
	}
	return servantResponse.result();
}

int CNodeModule::SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, const std::string& request, std::string& response)
{
	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	if (!request.empty()) {
		workerRequest.set_data(request);
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantResponse;
	SendNodeMessage(rpcChannel, servantRequest, servantResponse);

	response = servantResponse.data();

	return servantResponse.result();
}

int CNodeModule::SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, int32_t subcmd)
{
	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);
	servantRequest.set_sub_cmd(subcmd);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	return SendNodeMessage(rpcChannel, servantRequest);
}

int CNodeModule::SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request, int32_t subcmd)
{
	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	if (!request.SerializeToString(workerRequest.mutable_data())) {
		OutputError("!request.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);
	servantRequest.set_sub_cmd(subcmd);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	return SendNodeMessage(rpcChannel, servantRequest);
}

int CNodeModule::SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, const ::google::protobuf::Message& request, ::google::protobuf::Message& response, int32_t subcmd)
{
	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	if (!request.SerializeToString(workerRequest.mutable_data())) {
		OutputError("!request.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);
	servantRequest.set_sub_cmd(subcmd);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantResponse;
	SendNodeMessage(rpcChannel, servantRequest, servantResponse);

	if (!response.ParseFromString(servantResponse.data())) {
		OutputError("!response.ParseFromString");
		return PARSE_PACKAGE_FAIL;
	}
	return servantResponse.result();
}

int CNodeModule::SendNodeMessage(uint32_t serverId, int32_t cmd, uint64_t userId, const std::string& request, std::string& response, int32_t subcmd)
{
	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(serverId));
	if (rpcChannel.IsInvalid()) {
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::DataPacket workerRequest;
	workerRequest.set_cmd(N_CMD_MASTER_TO_USER);
	workerRequest.set_sub_cmd(cmd);
	workerRequest.set_route(userId);
	workerRequest.set_route_type(ROUTE_BALANCE_USERID);

	if (!request.empty()) {
		workerRequest.set_data(request);
	}

	::node::DataPacket servantRequest;
	servantRequest.set_cmd(N_CMD_MASTER_TRANSPORT);
	servantRequest.set_route(serverId);
	servantRequest.set_route_type(ROUTE_BALANCE_SERVERID);
	servantRequest.set_sub_cmd(subcmd);

	if (!workerRequest.SerializeToString(servantRequest.mutable_data())) {
		OutputError("!workerRequest.SerializeToString");
		return SERVER_ERROR_SERIALIZE;
	}

	::node::DataPacket servantResponse;
	SendNodeMessage(rpcChannel, servantRequest, servantResponse);

	response = servantResponse.data();

	return servantResponse.result();
}

int CNodeModule::SendNodeMessage(CAutoPointer<IChannelValue>& rpcChannel, const ::node::DataPacket& request) {

	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid()");
		return SERVER_ERROR_NOTFOUND_CHANNEL;
	}

	::node::WorkerService_Stub workerService_stub(&*rpcChannel->GetRpcChannel());

	rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);
	::node::DataPacket dpResponse;
	workerService_stub.HandleNotification(request, &dpResponse, &controller, NULL);
	controller.wait();

	const rpcz::status_code curStatus = controller.get_status();

	if (curStatus == rpcz::status::DEADLINE_EXCEEDED) {
		return SERVER_CALL_DEADLINE;
	}

	if (curStatus != rpcz::status::OK) {
		OutputError("controller.get_status() = %d", controller.get_status());
	}

	return dpResponse.result();
}

int CNodeModule::SendNodeMessage(CAutoPointer<IChannelValue>& rpcChannel, const ::node::DataPacket& request, ::node::DataPacket& response) {

	if(rpcChannel.IsInvalid()) {
		OutputError("rpcChannel.IsInvalid()");
		return SERVER_FAILURE;
	}

	::node::WorkerService_Stub workerService_stub(&*rpcChannel->GetRpcChannel());

	rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);
	workerService_stub.HandleNotification(request, &response, &controller, NULL);
	controller.wait();

	if(!controller.ok()) {
		OutputError("controller.get_status() = %d", controller.get_status());
	}

	return response.result();
}

void CNodeModule::BroadcastAllNodes(int32_t cmd)
{
	::node::DataPacket servantRequest;
	servantRequest.set_cmd(cmd);
	servantRequest.set_route_type(ROUTE_DIRECT_SERVERID);
	servantRequest.set_route(ID_NULL);

	std::vector<util::CAutoPointer<IChannelValue> > channels;
	IterateStaticChannel(channels);

	int nSize = (int)channels.size();
	for(int i = 0; i < nSize; ++i) {
		util::CAutoPointer<IChannelValue> pChannelValue(channels[i]);
		servantRequest.set_route(pChannelValue->GetServerId());
		SendNodeMessage(channels[i], servantRequest);
	}
}

int CNodeModule::AddServerId(uint32_t servantId, uint32_t serverId)
{
	if(servantId == serverId) {
		return TRUE;
	}

	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(servantId));
	if(rpcChannel.IsInvalid()) {
		return FALSE;
	}

	if((uint16_t)REGISTER_TYPE_SERVANT != rpcChannel->GetServerType()) {
		return FALSE;
	}

	rpcChannel->AddServerId(serverId);
	return TRUE;
}

void CNodeModule::ServantRemoveStaticChannel(uint32_t serverId)
{
	CAutoPointer<IChannelValue> rpcChannel(GetStaticChannel(serverId));
	if(rpcChannel.IsInvalid()) {
		return;
	}

	if(static_cast<uint16_t>(REGISTER_TYPE_SERVANT) != rpcChannel->GetServerType()) {
		return;
	}

	std::vector<uint32_t> serverIds;
	rpcChannel->IterateServerIds(serverIds);
	int nSize = (int)serverIds.size();
	for(int i = 0; i < nSize; ++i) {
		RemoveStaticChannel(serverIds[i]);
	}

	RemoveStaticChannel(serverId);
}


