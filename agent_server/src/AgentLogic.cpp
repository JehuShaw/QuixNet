/*
 * File:   AgentLogic.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_10_6 PM 2:45
 */

#include "AgentLogic.h"
#include "Log.h"
#include "Md5.h"
#include "CacheOperateHelper.h"
#include "WorkerOperateHelper.h"
#include "msg_agent_login.pb.h"
#include "msg_client_login.pb.h"
#include "msg_node_switch.pb.h"
#include "msg_game_map_switch.pb.h"
#include "TimerManager.h"
#include "LocalIDFactory.h"
#include "CallBack.h"
#include "ValueStream.h"
#include "ControlCentreStubImpEx.h"
#include "TimestampManager.h"
#include "ModuleOperateHelper.h"


using namespace ntwk;
using namespace evt;
using namespace thd;
using namespace util;


inline static bool CheckTargetMap(uint64_t userId, ::game::SwitchMapRequest& switchRequest) {
	::node::DataPacket mapIdRequest;
	mapIdRequest.set_cmd(N_CMD_PRESET_TARGET_MAP);
	mapIdRequest.set_route_type(ROUTE_BALANCE_USERID);
	mapIdRequest.set_route(userId);
	if (!SerializeWorkerData(mapIdRequest, switchRequest)) {
		return false;
	}
	::node::DataPacket mapIdResponse;
	SendWorkerNotification(mapIdRequest, mapIdResponse, switchRequest.mapid());
	return mapIdResponse.result() == SERVER_SUCCESS;
}

inline static bool CheckSwitchMapId(uint64_t userId, uint32_t oldMapId, ::game::SwitchMapRequest& switchRequest) {
	uint32_t newMapId = switchRequest.mapid();
	CBodyBitStream checkRequest;
	checkRequest.WriteUInt32(oldMapId);
	checkRequest.WriteUInt32(newMapId);
	CBodyBitStream checkResponse;
	checkResponse.WriteBool(FALSE);
	SendModuleNotification(N_CMD_CHECK_SWITCH_MAPID, checkRequest, checkResponse);
	bool bSwitchMap = checkResponse.ReadBool();
	if(bSwitchMap) {
		if (CheckTargetMap(userId, switchRequest)) {
			return true;
		}
	}
	return false;
}

CAgentLogic::CAgentLogic()
	: m_u16ServerRegoin(ID_NULL)
	, m_runningCount(0)
	, m_clientCount(0)
{
	m_pThis(this);
}

CAgentLogic::~CAgentLogic() {
}

bool CAgentLogic::Init(
	const std::string& strAddress, uint16_t usMaxLink,
	const std::string& strServantConnect, uint32_t u32MaxPacketSize,
	const std::string& endPoint, uint16_t u16ServerRegoin)
{
    m_strServantConnect = strServantConnect;
	m_endPoint = endPoint;
	m_u16ServerRegoin = u16ServerRegoin;
    m_tcpServer.SetLinkEvent(this);
	std::string strAcceptBind("0.0.0.0");
	std::string::size_type offset = strAddress.rfind(':');
	if(std::string::npos != offset) {
		strAcceptBind.append(strAddress, offset, strAddress.size() - offset);
	} else {
		assert(std::string::npos != offset);
	}
	m_queSocketIds.SetRawPointer(new SOCKETID_QUEUE_T());
	return m_tcpServer.Start(strAcceptBind.c_str(), usMaxLink, u32MaxPacketSize);
}

void CAgentLogic::Dispose() {
	// close tcp socket
	m_tcpServer.Stop();
    m_tcpServer.SetLinkEvent(NULL);
}

bool CAgentLogic::OnRun() {
	atomic_inc(&m_runningCount);
	//
	do {
		SocketID socketId;
		int nWhy = 0;
		if(m_tcpServer.HasLostConnection(socketId, nWhy)) {
			SocketID newSocketId;
			if(m_tcpServer.HasNewConnection(newSocketId)) {
				if(socketId == newSocketId) {
					break;
				}
				PrintBasic("lost ip address = %s  port = %d........\n",
					socketId.ToString(false), socketId.port);
				HandleLogout(socketId, nWhy);

				socketId = newSocketId;
				PrintBasic("recv ip address = %s  port = %d........\n",
					socketId.ToString(false), socketId.port);
				InsertSocketId(socketId);
			} else {
				PrintBasic("lost ip address = %s  port = %d........\n",
					socketId.ToString(false), socketId.port);
				HandleLogout(socketId, nWhy);
			}
		} else {
			if(m_tcpServer.HasNewConnection(socketId)) {
				PrintBasic("recv ip address = %s  port = %d........\n",
					socketId.ToString(false), socketId.port);
				InsertSocketId(socketId);
			}
		}
	} while(false);
	//
	ReceivePacket();
	atomic_dec(&m_runningCount);
	return false;
}

void CAgentLogic::OnShutdown() {
}

bool CAgentLogic::SendToClient(uint64_t userId, const ::node::DataPacket& message)
{
	SocketID socketId;
	if (!FindSocketId(userId, socketId)) {
		return false;
	}

	std::string bytes;
	if (!message.SerializeToString(&bytes)) {
		OutputError("!message.SerializeToString(byte)");
		return false;
	}

	return m_tcpServer.Send((unsigned char*)bytes.data(), bytes.size(), socketId);
}

bool CAgentLogic::SendToClient(uint64_t userId, const std::string& bytes)
{
	SocketID socketId;
	if(!FindSocketId(userId, socketId)) {
		return false;
	}

	return m_tcpServer.Send((unsigned char*)bytes.data(),
		bytes.length(), socketId);
}

void CAgentLogic::BroadcastToClient(const std::string& bytes, const std::set<uint64_t>& excludeId) {
	std::vector<SocketID> socketIds;
	IteratorSocketId(socketIds, excludeId);
	
	int nSize = (int)socketIds.size();
	for (int i = 0; i < nSize; ++i) {
		m_tcpServer.Send((unsigned char*)bytes.data(),
			bytes.length(), socketIds[i]);
	}
}

void CAgentLogic::CloseClient(uint64_t userId, int nWhy) {
	SocketID socketId;
	if (!FindSocketId(userId, socketId)) {
		return;
	}

    m_tcpServer.CloseConnection(socketId, nWhy);
}

void CAgentLogic::CloseAllClients(const std::set<uint64_t>& excludeId, int nWhy) {
	std::vector<SocketID> socketIds;
	IteratorSocketId(socketIds, excludeId);

	int nSize = (int)socketIds.size();
	for (int i = 0; i < nSize; ++i) {
		m_tcpServer.CloseConnection(socketIds[i], nWhy);
	}
}

void CAgentLogic::OnAccept() {
	if (m_runningCount > static_cast<long>(m_tcpServer.Size())) {
		return;
	}
    ThreadPool.ExecuteTask(this);
}

void CAgentLogic::OnDisconnect() {
	if (m_runningCount > static_cast<long>(m_tcpServer.Size())) {
		return;
	}
    ThreadPool.ExecuteTask(this);
}

void CAgentLogic::OnReceive() {
	if (m_runningCount > static_cast<long>(m_tcpServer.Size())) {
		return;
	}
    ThreadPool.ExecuteTask(this);
}

long CAgentLogic::GetClientCount() {
    return static_cast<long>(m_clientCount);
}

bool CAgentLogic::ReplaceSocketId(const SocketID& socketId, uint64_t userId, uint32_t mapId) {
	CScopedWriteLock wrlock(m_socketIdsLock);

	std::pair<USERID_TO_CLIENT_T::iterator, bool> pairIBUC(
	m_mapClients.insert(USERID_TO_CLIENT_T::value_type(
	userId, SClientData(socketId, mapId))));
	if(pairIBUC.second) {
		struct SPlayerData spData;
		spData.userId = userId;
		spData.mapId = mapId;
		std::pair<SOCKETID_TO_PLAYER_T::iterator, bool> pairIBSP(
			m_mapPlayers.insert(SOCKETID_TO_PLAYER_T::value_type(
				socketId, spData)));
		if(pairIBSP.second) {
			ntwk::SocketID* pSocketId = m_queSocketIds->WriteLock();
			*pSocketId = socketId;
			m_queSocketIds->WriteUnlock();
		} else {
			SPlayerData& spData = pairIBSP.first->second;
			spData.userId = userId;
			spData.mapId = mapId;
			uint64_t timerId = spData.timerId;
			if(ID_NULL != timerId) {
				spData.timerId = ID_NULL;
				CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
				pTMgr->Remove(timerId, true);
			}
		}
	} else {
		SClientData& scData = pairIBUC.first->second;
		uint64_t logoutTimerId = scData.logoutTimerId;
		if (ID_NULL != logoutTimerId) {
			scData.logoutTimerId = ID_NULL;
			CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
			pTMgr->Remove(logoutTimerId, true);
		}

		ntwk::SocketID oldSocketID(scData.socketId);
		if (oldSocketID == socketId) {
			scData.mapId = mapId;
			return false;
		}
		scData.socketId = socketId;
		scData.mapId = mapId;
		
		SOCKETID_TO_PLAYER_T::iterator itSP(m_mapPlayers.find(oldSocketID));
		if (m_mapPlayers.end() != itSP) {
			struct SPlayerData spData(itSP->second);
			m_mapPlayers.erase(itSP);

			uint64_t oldTimerId = spData.timerId;
			if (ID_NULL != oldTimerId) {
				spData.timerId = ID_NULL;
				CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
				pTMgr->Remove(oldTimerId, true);
			}
			std::pair<SOCKETID_TO_PLAYER_T::iterator, bool> pairIBSP(
				m_mapPlayers.insert(SOCKETID_TO_PLAYER_T::value_type(socketId, spData)));
			if (!pairIBSP.second) {
				uint64_t newTimerId = pairIBSP.first->second.timerId;
				pairIBSP.first->second = spData;
				if (ID_NULL != newTimerId) {
					CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
					pTMgr->Remove(newTimerId, true);
				}
			}
		} else {
			return false;
		}
    }
	return true;
}

void CAgentLogic::InsertSocketId(const SocketID& socketId) {
	CScopedWriteLock wrlock(m_socketIdsLock);
    struct SPlayerData newSPData;
    newSPData.userId = DEFAULT_USERID;
	newSPData.mapId = DEFAULT_MAPID;
    std::pair<SOCKETID_TO_PLAYER_T::iterator, bool> pairIBSP(
    m_mapPlayers.insert(SOCKETID_TO_PLAYER_T::value_type(
    socketId, newSPData)));
    if(pairIBSP.second) {
		ntwk::SocketID* pSocketId = m_queSocketIds->WriteLock();
		*pSocketId = socketId;
		m_queSocketIds->WriteUnlock();
    } else {
		SPlayerData& oldSPData = pairIBSP.first->second;
		oldSPData.userId = DEFAULT_USERID;
		oldSPData.mapId = DEFAULT_MAPID;
        uint64_t timerId = oldSPData.timerId;
        if(ID_NULL != timerId) {
            oldSPData.timerId = ID_NULL;
            CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
            pTMgr->Remove(timerId);
        }
    }
	SPlayerData& spData = pairIBSP.first->second;
    spData.timerId = CLocalIDFactory::Pointer()->GenerateID();
    CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
    CAutoPointer<CallbackMFnP2<CAgentLogic, ntwk::SocketID, int> >
        callback(new CallbackMFnP2<CAgentLogic, ntwk::SocketID, int>
        (m_pThis(), &CAgentLogic::OnLoginTimeout, socketId, LOGOUT_LOGIN_TIMEOUT));
    pTMgr->SetTimeout(spData.timerId, LOGIN_EXPIRY_TIME, callback);
}

bool CAgentLogic::RemoveSocketId(const SocketID& socketId, uint64_t& outUserId, uint32_t& outMapId) {
	CScopedWriteLock wrlock(m_socketIdsLock);
	SOCKETID_TO_PLAYER_T::iterator itSP(m_mapPlayers.find(socketId));
	if(m_mapPlayers.end() != itSP) {
		USERID_TO_CLIENT_T::const_iterator itUC(m_mapClients.find(itSP->second.userId));
        if(m_mapClients.end() != itUC && itUC->second.socketId == socketId) {
            m_mapClients.erase(itUC);
			atomic_dec(&m_clientCount);
        }
		outUserId = itSP->second.userId;
		outMapId = itSP->second.mapId;

        uint64_t timerId = itSP->second.timerId;
        m_mapPlayers.erase(itSP);

        if(ID_NULL != timerId) {
            CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
            pTMgr->Remove(timerId);
        }
		return true;
	}
	return false;
}

bool CAgentLogic::FindSocketId(uint64_t userId, SocketID& outSocketId) {
	CScopedReadLock rdlock(m_socketIdsLock);
	USERID_TO_CLIENT_T::iterator itUC(m_mapClients.find(userId));
	if(m_mapClients.end() == itUC) {
		return false;
	}
	outSocketId = itUC->second.socketId;
	return true;
}

bool CAgentLogic::FindSocketId(uint64_t userId, ntwk::SocketID& outSocketId, uint32_t& outMapId) {
	CScopedReadLock rdlock(m_socketIdsLock);
	USERID_TO_CLIENT_T::iterator itUC(m_mapClients.find(userId));
	if(m_mapClients.end() == itUC) {
		return false;
	}
	outSocketId = itUC->second.socketId;
	outMapId = itUC->second.mapId;
	return true;
}

void CAgentLogic::IteratorSocketId(std::vector<SocketID>& outSocketIds, const std::set<uint64_t>& excludeId) {
	CScopedReadLock rdlock(m_socketIdsLock);
	if (excludeId.empty()) {
		USERID_TO_CLIENT_T::iterator itUC(m_mapClients.begin());
		for (; m_mapClients.end() != itUC; ++itUC) {
			outSocketIds.push_back(itUC->second.socketId);
		}
	} else {
		USERID_TO_CLIENT_T::iterator itUC(m_mapClients.begin());
		for (; m_mapClients.end() != itUC; ++itUC) {
			if (excludeId.end() != excludeId.find(itUC->first)) {
				continue;
			}
			outSocketIds.push_back(itUC->second.socketId);
		}
	}
}

bool CAgentLogic::FindUserId(const ntwk::SocketID& socketId, uint64_t& outUserId, uint32_t& outMapId)
{
	CScopedReadLock rdlock(m_socketIdsLock);

	SOCKETID_TO_PLAYER_T::iterator itSP(m_mapPlayers.find(socketId));
	if(m_mapPlayers.end() == itSP) {
		return false;
	}
	outUserId = itSP->second.userId;
	outMapId = itSP->second.mapId;
	return true;
}

bool CAgentLogic::SendToClient(const SocketID& socketId,
	const ::node::DataPacket& message)
{
	std::string bytes;
	if (!message.SerializeToString(&bytes)) {
		OutputError("!message.SerializeToString(byte)");
		return false;
	}

	return m_tcpServer.Send((unsigned char*)bytes.data(), bytes.size(), socketId);
}

void CAgentLogic::ReceivePacket() {
    
	ntwk::SocketID* pSocketId = m_queSocketIds->ReadLock();
    if(!pSocketId) {
		Sleep(3);
        return;
    }
	SocketID socketId(*pSocketId);
	m_queSocketIds->ReadUnlock();

	int Count = m_queSocketIds->Size();

	do {
		LinkData* pLinkData = m_tcpServer.Receive(socketId);
		if (NULL == pLinkData) {
			break;
		}

		Packet* pPacket = m_tcpServer.AllocatePacket(pLinkData);
		if (NULL == pPacket) {
			pSocketId = m_queSocketIds->WriteLock();
			*pSocketId = socketId;
			m_queSocketIds->WriteUnlock();

			Sleep(1);
			if (Count-- < 1) {
				break;
			}

			pSocketId = m_queSocketIds->ReadLock();
			if (pSocketId) {
				socketId = *pSocketId;
				m_queSocketIds->ReadUnlock();
			} else {
				break;
			}
			continue;
		}

		::node::DataPacket dispatchRequest;
		if (dispatchRequest.ParseFromArray((char*)pPacket->data, pPacket->length)) {
			if (!m_tcpServer.DeallocatePacket(pLinkData, pPacket)) {
				OutputError("!m_tcpServer.DeallocatePacket socketId = %s ", socketId.ToString());
			}
			if (HandlePacket(dispatchRequest, socketId)) {
				m_tcpServer.CloseConnection(socketId, LOGOUT_NOT_VALIDATED);
				break;
			}
		} else {
			if (!m_tcpServer.DeallocatePacket(pLinkData, pPacket)) {
				OutputError("!m_tcpServer.DeallocatePacket socketId = %s ", socketId.ToString());
			}
			OutputError("!dispatchRequest.ParseFromArray socketId = %s ", socketId.ToString());
		}
	} while (true);
}

bool CAgentLogic::ChangeMapId(const ntwk::SocketID& socketId, uint32_t mapId)
{
	uint64_t userId = ID_NULL;
	do {
		CScopedWriteLock wLock(m_socketIdsLock);

		SOCKETID_TO_PLAYER_T::iterator itSP(m_mapPlayers.find(socketId));
		if (m_mapPlayers.end() == itSP) {
			return false;
		}
		itSP->second.mapId = mapId;
		userId = itSP->second.userId;

		USERID_TO_CLIENT_T::iterator itUC(m_mapClients.find(userId));
		if(m_mapClients.end() != itUC) {
			itUC->second.mapId = mapId;
		}
	} while(false);
	CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
	if(SERVER_SUCCESS != pCtrlCenStubImpEx->UpdateUser(
		GetServantConnect(), userId, mapId))
	{
		OutputError("SERVER_SUCCESS != pCtrlCenStubImpEx->UpdateUser()");
	}
	return true;
}

bool CAgentLogic::HandlePacket(const ::node::DataPacket& dispatchRequest,
	const SocketID& socketId)
{
	switch (dispatchRequest.cmd()) {
	case P_CMD_C_KEEPALIVE:
		return HandleKeepAlive(dispatchRequest, socketId);
	case P_CMD_C_LOGIN:
		return HandleLogin(dispatchRequest, socketId);
	case P_CMD_C_SWITCH_NODE:
		return HandleSwitchNode(dispatchRequest, socketId);
	case P_CMD_C_SWITCH_MAP:
		return HandleSwitchMap(dispatchRequest, socketId);
	case P_CMD_S_LOGOUT:
		return HandleLogout(dispatchRequest, socketId);
	default:
		return HandleDefault(dispatchRequest, socketId);
	}
}

bool CAgentLogic::HandleKeepAlive(const ::node::DataPacket& dispatchRequest,
	const ntwk::SocketID& socketId)
{
	::node::DataPacket dispatchResponse;
	dispatchResponse.set_cmd(dispatchRequest.cmd());
	dispatchResponse.set_route(dispatchRequest.route());
	dispatchResponse.set_route_type(dispatchRequest.route_type());
	dispatchResponse.set_result(SERVER_SUCCESS);
	SendToClient(socketId, dispatchResponse);
	return false;
}

bool CAgentLogic::HandleLogin(const ::node::DataPacket& dispatchRequest,
	const SocketID& socketId)
{
	if(SERVER_STATUS_STOP == g_serverStatus) {
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(SERVER_ERROR_SERVER_STOP);
		SendToClient(socketId, dispatchResponse);
		return false;
	}

	if(!HasDataPacketData(dispatchRequest)) {
		OutputError("!dispatchRequest.has_data() ip = %s",socketId.ToString());
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(SERVER_FAILURE);
		SendToClient(socketId, dispatchResponse);
		return false;
	}

	const std::string& bytes = dispatchRequest.data();
	agent::LoginRequest loginRequest;
	if(!loginRequest.ParseFromArray(bytes.data(), (int)bytes.length())) {
		OutputError("!loginRequest.ParseFromArray ip = %s", socketId.ToString());
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(SERVER_FAILURE);
		SendToClient(socketId, dispatchResponse);
		return false;
	}

	//OutputDebug("0 CAgentLogic::HandleLogin Begin userId = " I64FMTD, loginRequest.userid());
	
	SocketID oldSocketId;
	uint32_t oldMapId;
	if (FindSocketId(loginRequest.userid(), oldSocketId, oldMapId)) {
		// 如果是断线重连
		if (!ReplaceSocketId(socketId, loginRequest.userid(), oldMapId)) {
			OutputError("The same socket id login! ip = %s", socketId.ToString());
			::node::DataPacket dispatchResponse;
			dispatchResponse.set_cmd(dispatchRequest.cmd());
			dispatchResponse.set_result(SERVER_FAILURE);
			SendToClient(socketId, dispatchResponse);
			return false;
		}
		::node::DataPacket dispatchAll;
		dispatchAll.set_cmd(dispatchRequest.cmd());
		dispatchAll.set_route(loginRequest.userid());
		dispatchAll.set_sub_cmd(LOGIN_RECOVER);
		::node::DataPacket dispatchResponse;
		SendWorkerProtocol(dispatchAll, dispatchResponse, oldMapId);
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		SendToClient(socketId, dispatchResponse);
		return false;
	}

	//OutputDebug("1 CAgentLogic::HandleLogin userId = " I64FMTD, loginRequest.userid());
	// 通过登录服登录
	::node::CheckUserResponse checkResponse;
	CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
	if(SERVER_SUCCESS != pCtrlCenStubImpEx->CheckUser(GetServantConnect(),
		loginRequest.userid(), checkResponse))
	{
		OutputError("TRUE != pCtrlCenStubImpEx->CheckUser userId = " I64FMTD, loginRequest.userid());
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(SERVER_FAILURE);
		SendToClient(socketId, dispatchResponse);
		return false;
	}
	//OutputDebug("2 CAgentLogic::HandleLogin userId = " I64FMTD, loginRequest.userid());
	bool bCheckResult = false;
	if (!loginRequest.sessionkey().empty()) {
		char szMd5Buf[eBUF_SIZE_128] = { '\0' };
		int nOffset = snprintf(szMd5Buf, sizeof(szMd5Buf), I64FMTD "" I64FMTD "%s%u",
			loginRequest.account(), loginRequest.userid(),
			checkResponse.createtime().c_str(), checkResponse.logincount());
		szMd5Buf[sizeof(szMd5Buf) - 1] = '\0';
		bCheckResult = (loginRequest.sessionkey() == util::md5(szMd5Buf, nOffset));
	}
	if (!bCheckResult) {
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(AGENT_MD5_CHECK_FAIL);
		SendToClient(socketId, dispatchResponse);
		return false;
	}

	uint32_t mapId = checkResponse.mapid();
	//OutputDebug("3 CAgentLogic::HandleLogin userId = " I64FMTD, loginRequest.userid());
	uint32_t u32NewRegion = GetServerRegoin();
	if(SERVER_SUCCESS != pCtrlCenStubImpEx->UpdateUser(
		GetServantConnect(), loginRequest.userid(), mapId, u32NewRegion, true))
	{
		OutputError("SERVER_SUCCESS != pCtrlCenStubImpEx->UpdateUser()");
	}

	//OutputDebug("4 CAgentLogic::HandleLogin userId = " I64FMTD, loginRequest.userid());

	KickCacheLogged(loginRequest.userid());
	//OutputDebug("6 CAgentLogic::HandleLogin userId = " I64FMTD, loginRequest.userid());
	
	OutputDebug("7 CAgentLogic::HandleLogin  userId =" I64FMTD ", index = %d, binaryAddress = %u, port = %d mapId = %u ",
		loginRequest.userid(), socketId.index, socketId.binaryAddress, (int)socketId.port, mapId);
	if(!ReplaceSocketId(socketId, loginRequest.userid(), mapId)) {
        OutputError("!InsertSocketId  userId = " I64FMTD, loginRequest.userid());
        ::node::DataPacket dispatchResponse;
        dispatchResponse.set_cmd(dispatchRequest.cmd());
        dispatchResponse.set_result(SERVER_FAILURE);
        SendToClient(socketId, dispatchResponse);
        return false;
    }
	//OutputDebug("8 CAgentLogic::HandleLogin userId = " I64FMTD, loginRequest.userid());
	::node::DataPacket dispatchAll;
	dispatchAll.set_cmd(dispatchRequest.cmd());
	dispatchAll.set_route(loginRequest.userid());
	dispatchAll.set_sub_cmd(LOGIN_NORMAL);
   // OutputDebug("userId = " I64FMTD, loginRequest.userid());
	::node::LoginRequest loginAllRequest;
	assert(!m_endPoint.empty());
	loginAllRequest.set_originip(m_endPoint);
	loginAllRequest.set_routecount(1);
	loginAllRequest.set_account(loginRequest.account());
    loginAllRequest.set_version(loginRequest.version());
	loginAllRequest.set_remoteip(socketId.ToString(true));
	loginAllRequest.set_mapid(mapId);

	if(!SerializeWorkerData(dispatchAll, loginAllRequest)){
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(SERVER_ERROR_SERIALIZE);
		SendToClient(socketId, dispatchResponse);
		return false;
	}
	
	atomic_inc(&m_clientCount);
	//OutputDebug("9 CAgentLogic::HandleLogin userId = " I64FMTD, loginRequest.userid());
	::node::DataPacket dispatchResponse;
	SendWorkerProtocol(dispatchAll, dispatchResponse, mapId);

	dispatchResponse.set_cmd(dispatchRequest.cmd());
	SendToClient(socketId, dispatchResponse);

	//OutputDebug("10 CAgentLogic::HandleLogin End userId = " I64FMTD, loginRequest.userid());
	return false;
}

bool CAgentLogic::HandleLogout(const ::node::DataPacket& dispatchRequest, const ntwk::SocketID& socketId)
{
	uint64_t userId(DEFAULT_USERID);
	uint32_t mapId(DEFAULT_MAPID);
	if (!FindUserId(socketId, userId, mapId)) {
		OutputError("!FindUserId");
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(SERVER_ERROR_NOTFOUND_USER);
		SendToClient(socketId, dispatchResponse);
		return true;
	}
	return false;
}

bool CAgentLogic::HandleSwitchNode(const ::node::DataPacket& dispatchRequest,
	const ntwk::SocketID& socketId)
{
	uint64_t userId(DEFAULT_USERID);
	uint32_t mapId(DEFAULT_MAPID);
	if (!FindUserId(socketId, userId, mapId)) {
		OutputError("!FindUserId");
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(SERVER_ERROR_NOTFOUND_USER);
		SendToClient(socketId, dispatchResponse);
		return true;
	}
	OutputDebug("CAgentLogic::HandleSwitchNode  userId =" I64FMTD ", index = %d, binaryAddress = %u, port = %d ",
		userId, socketId.index, socketId.binaryAddress, socketId.port);
	
	::node::CheckUserResponse checkResponse;
	CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
	if (SERVER_SUCCESS != pCtrlCenStubImpEx->CheckUser(GetServantConnect(),
		userId, checkResponse))
	{
		OutputError("TRUE != controlCentreStubEx.CheckUserId");
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(SERVER_FAILURE);
		SendToClient(socketId, dispatchResponse);
		return false;
	}

	std::string strSessionKey;
	uint64_t timestamp = CTimestampManager::Pointer()->GetTimestamp();
	do {
		char szMd5Buf[eBUF_SIZE_128] = { '\0' };
		int nOffset = snprintf(szMd5Buf, sizeof(szMd5Buf), I64FMTD "" I64FMTD "%s%u" I64FMTD,
			checkResponse.account(), userId, checkResponse.createtime().c_str(),
			checkResponse.logincount(), timestamp);
		szMd5Buf[sizeof(szMd5Buf) - 1] = '\0';
		strSessionKey = util::md5(szMd5Buf, nOffset);
	} while (false);

	::node::SwitchData switchResponse;
	switchResponse.set_account(checkResponse.account());
	switchResponse.set_userid(userId);
	switchResponse.set_timestamp(timestamp);
	switchResponse.set_sessionkey(strSessionKey);

	::node::DataPacket dispatchResponse;
	if (!SerializeWorkerData(dispatchResponse, switchResponse)) {
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(SERVER_FAILURE);
		SendToClient(socketId, dispatchResponse);
		return false;
	}

	dispatchResponse.set_cmd(dispatchRequest.cmd());
	dispatchResponse.set_result(SERVER_SUCCESS);
	SendToClient(socketId, dispatchResponse);
	return false;
}

bool CAgentLogic::HandleSwitchMap(const ::node::DataPacket& dispatchRequest,
	const ntwk::SocketID& socketId)
{
	uint64_t userId(DEFAULT_USERID);
	uint32_t mapId(DEFAULT_MAPID);
	if (!FindUserId(socketId, userId, mapId)) {
		OutputError("!FindUserId");
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(SERVER_ERROR_NOTFOUND_USER);
		SendToClient(socketId, dispatchResponse);
		return true;
	}
	::game::SwitchMapRequest switchRequest;
	if (!ParseWorkerData(switchRequest, dispatchRequest)) {
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(PARSE_PACKAGE_FAIL);
		SendToClient(socketId, dispatchResponse);
		return false;
	}

	::node::DataPacket dispatchResponse;
	//////////////////////////////////////////////////////////////////////////
	if (CheckSwitchMapId(userId, mapId, switchRequest)) {
		SwitchMapThroughNodes(socketId, userId, mapId, switchRequest.mapid(), dispatchResponse);
	} else {
		const_cast<::node::DataPacket&>(dispatchRequest).set_route(userId);
		dispatchResponse.set_result(CANNT_FIND_MAP);
		SendWorkerProtocol(dispatchRequest, dispatchResponse, switchRequest.mapid());
	}
	if (switchRequest.mapid() != mapId && SERVER_SUCCESS == dispatchResponse.result()) {
		ChangeMapId(socketId, switchRequest.mapid());
	} else {
		OutputError("mapid() == mapId && SERVER_SUCCESS != result() mapId() = %d mapId =%d result = %d",
			switchRequest.mapid(), mapId, dispatchResponse.result());
	}

	dispatchResponse.set_cmd(dispatchRequest.cmd());
	SendToClient(socketId, dispatchResponse);
	return false;
}

bool CAgentLogic::HandleDefault(const ::node::DataPacket& dispatchRequest,
	const SocketID& socketId)
{
	uint64_t userId(DEFAULT_USERID);
	uint32_t mapId(DEFAULT_MAPID);
	if(!FindUserId(socketId, userId, mapId)) {
		OutputError("!FindUserId");
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(SERVER_ERROR_NOTFOUND_USER);
		SendToClient(socketId, dispatchResponse);
		return true;
	}
	OutputDebug("userId = " I64FMTD, userId);
	const_cast<::node::DataPacket&>(dispatchRequest).set_route(userId);
	::node::DataPacket dispatchResponse;
	SendWorkerProtocol(dispatchRequest, dispatchResponse, mapId);
	if(dispatchResponse.result() != SERVER_NO_RESPOND) {
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		SendToClient(socketId, dispatchResponse);
		OutputDebug("Protocol Reponse userId = " I64FMTD " cmd = %u result = %d ",
			userId, dispatchRequest.cmd(), dispatchResponse.result());
	}
	return false;
}

void CAgentLogic::OnLoginTimeout(ntwk::SocketID& socketId, int& nWhy)
{
    m_tcpServer.CloseConnection(socketId, nWhy);
}

bool CAgentLogic::KickLogged(uint64_t userId)
{
	SocketID curSocketId;
	if(FindSocketId(userId, curSocketId)) {
		if (curSocketId == SocketID()) {
			return false;
		}
		// tell client
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(P_CMD_S_LOGOUT);
		dispatchResponse.set_result(SERVER_ERROR_ALREADY_EXIST);
		SendToClient(curSocketId, dispatchResponse);
		// set close
		uint64_t timerId = CLocalIDFactory::Pointer()->GenerateID();
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		CAutoPointer<CallbackMFnP2<CAgentLogic, ntwk::SocketID, int> >
			callback(new CallbackMFnP2<CAgentLogic, ntwk::SocketID, int>
			(m_pThis(), &CAgentLogic::OnLoginTimeout, curSocketId, LOGOUT_KICK));
		pTMgr->SetTimeout(timerId, KEEP_FOR_CLOSE_CONNECTION, callback);
		return true;
	}
	return false;
}

void CAgentLogic::SwitchMapThroughNodes(
	const ntwk::SocketID& socketId,
	uint64_t userId,
	uint32_t oldMapId,
	uint32_t newMapId,
	::node::DataPacket& outResponse)
{
	::node::DataPacket loginAllRequest;
	loginAllRequest.set_cmd(P_CMD_C_LOGIN);
	loginAllRequest.set_route(userId);
	loginAllRequest.set_sub_cmd(LOGIN_SWITCH_MAP);

	::node::LoginRequest loginRequest;
	loginRequest.set_originip(m_endPoint);
	loginRequest.set_routecount(1);
	loginRequest.set_remoteip(socketId.ToString(true));
	loginRequest.set_mapid(newMapId);

	if(!SerializeWorkerData(loginAllRequest, loginRequest)) {
		return;
	}

	// logout
	::node::DataPacket logoutAllRequest;
	logoutAllRequest.set_cmd(P_CMD_S_LOGOUT);
	logoutAllRequest.set_route(userId);
	logoutAllRequest.set_result(LOGOUT_SWITCH_MAP);
	::node::DataPacket logoutResponse;
	SendWorkerProtocol(logoutAllRequest, logoutResponse, oldMapId);
	// login
	SendWorkerProtocol(loginAllRequest, outResponse, newMapId);
}

void CAgentLogic::SetDelayLogout(const SocketID& socketId, int nWhy)
{
	CScopedWriteLock wrlock(m_socketIdsLock);

	SOCKETID_TO_PLAYER_T::iterator itSP(m_mapPlayers.find(socketId));
	if (m_mapPlayers.end() == itSP) {
		OutputError("m_mapPlayers.end() == itSP socketId = %s", socketId.ToString(true));
		return;
	}
	USERID_TO_CLIENT_T::iterator itUC(m_mapClients.find(itSP->second.userId));
	if (m_mapClients.end() == itUC) {
		OutputError("m_mapClients.end() == itUC socketId = %s userId = " I64FMTD, socketId.ToString(true), itSP->second.userId);
		return;
	}
	uint64_t logoutTimerId = itUC->second.logoutTimerId;
	if (ID_NULL != logoutTimerId) {
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		pTMgr->Modify(logoutTimerId, TIMER_OPERATER_RESET);
	} else {
		logoutTimerId = CLocalIDFactory::Pointer()->GenerateID();
		itUC->second.logoutTimerId = logoutTimerId;
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		CAutoPointer<CallbackMFnP2<CAgentLogic, ntwk::SocketID, int> >
			callback(new CallbackMFnP2<CAgentLogic, ntwk::SocketID, int>
			(m_pThis(), &CAgentLogic::OnLogoutCallback, socketId, nWhy));
		pTMgr->SetTimeout(logoutTimerId, LOGOUT_DELAY, callback);
	}
}

void CAgentLogic::OnLogoutCallback(ntwk::SocketID& socketId, int& nWhy)
{
	uint64_t userId(DEFAULT_USERID);
	uint32_t mapId(DEFAULT_MAPID);
	if (RemoveSocketId(socketId, userId, mapId)) {
		::node::DataPacket dispatchAll;
		dispatchAll.set_cmd(P_CMD_S_LOGOUT);
		dispatchAll.set_route(userId);
		dispatchAll.set_result(nWhy);
		::node::DataPacket dispatchResponse;
		SendWorkerProtocol(dispatchAll, dispatchResponse, mapId);
		CControlCentreStubImpEx::PTR_T pCtrlStubEx(CControlCentreStubImpEx::Pointer());
		pCtrlStubEx->FreeServer(m_strServantConnect, userId, true);
	}
}