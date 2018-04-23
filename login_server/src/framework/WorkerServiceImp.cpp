#include "WorkerServiceImp.h"
#include "ModuleManager.h"
#include "BodyMessage.h"
#include "ChannelManager.h"
#include "rpc_controller.hpp"
#include "WorkerOperateHelper.h"
#include "ControlCentreStubImpEx.h"
#include "Log.h"
#include "msg_client_login.pb.h"

using namespace mdl;
using namespace util;

inline static bool HandleLogin(
	::node::DataPacket& request,
	const std::string& strBind,
	const std::string& servantAddress,
	const std::string& serverName,
	uint16_t serverId)
{
	assert(request.has_data());
	assert(!strBind.empty());

	uint64_t userId = request.route();

	::node::LoginRequest loginRequest;
	if(!ParseWorkerData(loginRequest, request)) {
		OutputError("!ParseWorkerData userId = "I64FMTD, userId);
		return false;
	}
	uint32_t routeCount = loginRequest.routecount();
	std::string originIp(loginRequest.originip());

	loginRequest.set_originip(strBind);
	loginRequest.set_routecount(routeCount + 1);
	if(!SerializeWorkerData(request, loginRequest)) {
		OutputError("!SerializeWorkerData userId = "I64FMTD, userId);
		return false;
	}

	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	pChlMgr->SetClientChannel(userId, originIp, routeCount);
	if(!servantAddress.empty()) {
		CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
		pCtrlCenStubImpEx->UserLogin(servantAddress, serverName, serverId, userId);
	}
	return true;
}

inline static bool HandleLogout(
	::node::DataPacket& request,
	const std::string& servantAddress,
	const std::string& serverName)
{
	uint64_t userId = request.route();
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
    pChlMgr->RemoveClientChannel(userId);
	if(!servantAddress.empty()) {
		CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
		pCtrlCenStubImpEx->UserLogout(servantAddress, serverName, userId);
	}
    return true;
}

CWorkerServiceImp::CWorkerServiceImp(
	const std::string& serverBind, 
	const std::string& servantAddress,
	const std::string& serverName,
	uint16_t serverId,
	CreatePlayerMethod createPlayerMethod /*= NULL*/,
	ListInterestsMethod listProtoMethod /*= NULL*/,
	ListInterestsMethod listNotifMethod /*= NULL*/
	)
	: m_serverBind(serverBind)
	, m_servantAddress(servantAddress)
	, m_serverName(serverName)
	, m_serverId(serverId)
	, m_createPlayerMethod(createPlayerMethod)
	, m_listProtoMethod(listProtoMethod)
	, m_listNotifMethod(listNotifMethod)
{
}


CWorkerServiceImp::~CWorkerServiceImp(void)
{
}

void CWorkerServiceImp::HandleProtocol(const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket> response)
{
	if(request.route() == ID_NULL) {
		::node::DataPacket dspResponse;
		SendWorkerProtocol(request, dspResponse);
		dspResponse.set_cmd(request.cmd());
		response.send(dspResponse);
		return;
	}

	if(request.cmd() == P_CMD_C_LOGIN) {
		if(!HandleLogin(const_cast<::node::DataPacket&>(request),
			m_serverBind, m_servantAddress, m_serverName, m_serverId)) 
		{
			::node::DataPacket dspResponse;
			dspResponse.set_cmd(request.cmd());
			dspResponse.set_result(FALSE);
			response.send(dspResponse);
			return;
		}
		::node::DataPacket dspResponse;

		if(NULL == m_createPlayerMethod) {
			SendWorkerProtocol(request, dspResponse);
		} else {
			CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
			CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(request.route()));
			if(pPlayer.IsInvalid()) {
				CAutoPointer<CPlayerBase> pNewPlayer((*m_createPlayerMethod)(request.route()));
				assert(!pNewPlayer.IsInvalid());
				pChlMgr->SetPlayer(request.route(), pNewPlayer);
				CScopedPlayerMutex scopedPlayerMutex(pNewPlayer);
				SendWorkerProtocol(request, dspResponse, pNewPlayer);
			} else {
				CScopedPlayerMutex scopedPlayerMutex(pPlayer);
				SendWorkerProtocol(request, dspResponse, pPlayer);
			}
		}

		dspResponse.set_cmd(request.cmd());
		response.send(dspResponse);

	} else if(request.cmd() == P_CMD_S_LOGOUT) {

		::node::DataPacket dspResponse;

		if(NULL == m_createPlayerMethod) {
			SendWorkerProtocol(request, dspResponse);
		} else {
			CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
			CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(request.route()));
			if(pPlayer.IsInvalid()) {
				SendWorkerProtocol(request, dspResponse, pPlayer);
			} else {
				CScopedPlayerMutex scopedPlayerMutex(pPlayer);
				SendWorkerProtocol(request, dspResponse, pPlayer);
			}
		}

		if(!HandleLogout(const_cast<::node::DataPacket&>(request),
			m_servantAddress, m_serverName)) 
		{
			::node::DataPacket dspResponse;
			dspResponse.set_cmd(request.cmd());
			dspResponse.set_result(FALSE);
			response.send(dspResponse);
		} else {
			dspResponse.set_cmd(request.cmd());
			response.send(dspResponse);
		}

	} else {
		::node::DataPacket dspResponse;

		if(NULL == m_createPlayerMethod) {
			SendWorkerProtocol(request, dspResponse);
		} else {
			CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
			CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(request.route()));
			if(pPlayer.IsInvalid()) {
				SendWorkerProtocol(request, dspResponse, pPlayer);
			} else {
				CScopedPlayerMutex scopedPlayerMutex(pPlayer);
				SendWorkerProtocol(request, dspResponse, pPlayer);
			}
		}

		dspResponse.set_cmd(request.cmd());
		response.send(dspResponse);
	}
}

void CWorkerServiceImp::HandleNotification(const ::node::DataPacket& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	if(request.route() == ID_NULL) {
		::node::DataPacket dspResponse;
		SendWorkerNotification(request, dspResponse);
		dspResponse.set_cmd(request.cmd());
		response.send(dspResponse);
		return;
	}

	::node::DataPacket dspResponse;

	if(NULL == m_createPlayerMethod || N_CMD_KICK_LOGGED == request.cmd()) {
		SendWorkerNotification(request, dspResponse);
	} else {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(request.route()));
		if(pPlayer.IsInvalid()) {
			SendWorkerNotification(request, dspResponse, pPlayer);
		} else {
			CScopedPlayerMutex scopedPlayerMutex(pPlayer);
			SendWorkerNotification(request, dspResponse, pPlayer);
		}
	}

	dspResponse.set_cmd(request.cmd());
	response.send(dspResponse);
}

void CWorkerServiceImp::ListProtocolInterests(const ::node::VoidPacket& request,
	::rpcz::reply< ::node::InterestPacket> response)
{
	// attach protocols interest.
	::node::InterestPacket workerInterest;
	if(NULL != m_listProtoMethod) {
		(*m_listProtoMethod)(workerInterest);
	}
	// send result
	response.send(workerInterest);
}

void CWorkerServiceImp::ListNotificationInterests(const ::node::VoidPacket& request,
	::rpcz::reply<::node::InterestPacket> response)
{
	// attach notification interest.
	::node::InterestPacket workerInterest;
	if(NULL != m_listNotifMethod) {
		(*m_listNotifMethod)(workerInterest);
	}
	// send result
	response.send(workerInterest);
}

void CWorkerServiceImp::SendToClient(const ::node::DataPacket& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	::node::DataPacket dspResponse;
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	CAutoPointer<rpcz::rpc_channel> channel(
		pChlMgr->GetRpczChannel(request.route()));

	if(channel.IsInvalid()) {
		dspResponse.set_cmd(request.cmd());
		dspResponse.set_result(FALSE);
		response.send(dspResponse);
		return;
	}

	::node::WorkerService_Stub workerService_stub(&*channel, false);
	rpcz::rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);

	workerService_stub.SendToClient(request, &dspResponse, &controller, NULL);
	controller.wait();

	pChlMgr->CheckClientChannel(request.route(), !controller.ok());

	dspResponse.set_cmd(request.cmd());
	response.send(dspResponse);
}

void CWorkerServiceImp::CloseClient(const ::node::DataPacket& request,
    ::rpcz::reply< ::node::DataPacket> response)
{
	::node::DataPacket dspResponse;
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

	CAutoPointer<rpcz::rpc_channel> channel(
		pChlMgr->GetRpczChannel(request.route()));

	if(channel.IsInvalid()) {
		dspResponse.set_cmd(request.cmd());
		dspResponse.set_result(FALSE);
		response.send(dspResponse);
		return;
	}

	::node::WorkerService_Stub workerService_stub(&*channel, false);
	rpcz::rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);

	workerService_stub.CloseClient(request, &dspResponse, &controller, NULL);
	controller.wait();

	pChlMgr->CheckClientChannel(request.route(), !controller.ok());

	dspResponse.set_cmd(request.cmd());
	response.send(dspResponse);
}

void CWorkerServiceImp::SendToWorker(const ::node::DataPacket& request,
    ::rpcz::reply< ::node::DataPacket> response)
{
	::node::DataPacket dspResponse;
    CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

    CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(request.route()));
    if(pPlayer.IsInvalid()) {
		SendWorkerNotification(request, dspResponse, pPlayer);
    } else {
		CScopedPlayerMutex scopedPlayerMutex(pPlayer);
		SendWorkerNotification(request, dspResponse, pPlayer);
    }

	CAutoPointer<rpcz::rpc_channel> channel(
		pChlMgr->GetRpczChannel(request.route()));
	if(!channel.IsInvalid()) {
		::node::WorkerService_Stub workerService_stub(&*channel, false);
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		workerService_stub.SendToWorker(request, &dspResponse, &controller, NULL);
		controller.wait();
		pChlMgr->CheckClientChannel(request.route(), !controller.ok());
	}

	dspResponse.set_cmd(request.cmd());
	response.send(dspResponse);
}

void CWorkerServiceImp::KickLogged(const ::node::DataPacket& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	::node::DataPacket dspResponse;
	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	CAutoPointer<rpcz::rpc_channel> channel(
		pChlMgr->GetRpczChannel(request.route()));

	if(channel.IsInvalid()) {
		dspResponse.set_cmd(request.cmd());
		dspResponse.set_result(FALSE);
		response.send(dspResponse);
		return;
	}

	::node::WorkerService_Stub workerService_stub(&*channel, false);
	rpcz::rpc_controller controller;
	controller.set_deadline_ms(CALL_DEADLINE_MS);

	workerService_stub.KickLogged(request, &dspResponse, &controller, NULL);
	controller.wait();

	pChlMgr->CheckClientChannel(request.route(), !controller.ok());

	dspResponse.set_cmd(request.cmd());
	response.send(dspResponse);
}

