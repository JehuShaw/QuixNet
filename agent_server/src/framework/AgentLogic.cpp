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
#include "TimerManager.h"
#include "GuidFactory.h"
#include "CallBack.h"
#include "ValueStream.h"
#include "ControlCentreStubImpEx.h"


using namespace ntwk;
using namespace evt;
using namespace thd;
using namespace util;

CAgentLogic::CAgentLogic()
	: m_sktIdxCount(-1)
	, m_u16ServerRegoin(ID_NULL)
{

}

CAgentLogic::~CAgentLogic() {
}

bool CAgentLogic::Init(
	const std::string& strAddress, uint16_t usMaxLink,
	const std::string& strServantConnect, uint32_t u32MaxPacketSize,
	const std::string& strBind, uint16_t u16ServerRegoin)
{
    m_strServantConnect = strServantConnect;
	m_strBind = strBind;
	m_u16ServerRegoin = u16ServerRegoin;
    m_tcpServer.SetLinkEvent(this);
	return m_tcpServer.Start(strAddress.c_str(), usMaxLink, u32MaxPacketSize);
}

void CAgentLogic::Dispose() {
	// close tcp socket
	m_tcpServer.Stop();
    m_tcpServer.SetLinkEvent(NULL);
}

bool CAgentLogic::Run() {

	SocketID socketId;
	//
	if(m_tcpServer.HasNewConnection(socketId)){
		PrintBasic("recv ip address = %s  port = %d........\n",
			socketId.ToString(false), socketId.port);
		InsertSocketId(socketId);
	}
	//
	if(m_tcpServer.HasLostConnection(socketId)){
		PrintBasic("lost ip address = %s  port = %d........\n",
			socketId.ToString(false), socketId.port);
		::node::DataPacket dispatchResponse;
		HandleLogout(socketId, dispatchResponse);
	}
	//
    std::vector<::node::DataPacket> outPackets;
    if(ReceivePacket(socketId, outPackets)) {
        for(int i = 0; i < (int)outPackets.size(); ++i) {
            ::node::DataPacket& dispatchRequest = outPackets[i];

            switch(dispatchRequest.cmd()) {
            case P_CMD_C_LOGIN:
                HandleLogin(dispatchRequest, socketId);
                break;
            case P_CMD_S_LOGOUT:
                // Don't do anything here.
                break;
            default:
                HandleDefault(dispatchRequest, socketId);
                break;
            }
        }
    }
	return false;
}

void CAgentLogic::OnShutdown() {
}

bool CAgentLogic::SendToClient(uint64_t userId, const ::node::DataPacket& message)
{
	SocketID socketId;
	if(!FindSocketId(userId, socketId)) {
		return false;
	}

	const_cast<::node::DataPacket&>(message).clear_route();
	const_cast<::node::DataPacket&>(message).clear_route_type();

	SmallBuffer smallbuf(message.ByteSize());
	if(!message.SerializeToArray((char*)smallbuf, message.ByteSize())) {
		OutputError("!pMessage->SerializeToArray");
		return false;
	}

	return m_tcpServer.Send((unsigned char*)(char*)smallbuf,
		message.ByteSize(), socketId);
}

void CAgentLogic::CloseClient(uint64_t userId) {

    SocketID socketId;
    if(!FindSocketId(userId, socketId)) {
        return;
    }

    m_tcpServer.CloseConnection(socketId);
}

void CAgentLogic::OnAccept() {
    ThreadPool.ExecuteTask(this);
}

void CAgentLogic::OnDisconnect() {
    ThreadPool.ExecuteTask(this);
}

void CAgentLogic::OnReceive() {
    ThreadPool.ExecuteTask(this);
}

long CAgentLogic::GetLinkCount() {
    CScopedReadLock rdlock(m_socketIdsLock);
    return (long)m_clients.size();
}

bool CAgentLogic::InsertSocketId(uint64_t userId, const SocketID& socketId) {
	CScopedWriteLock wrlock(m_socketIdsLock);

	std::pair<USERID_TO_SOCKETID_T::iterator, bool> pairIBUS(
	m_socketIds.insert(USERID_TO_SOCKETID_T::value_type(
	userId, socketId)));
    if(!pairIBUS.second) {
        if(pairIBUS.first->second == socketId) {
            return false;
        }
        pairIBUS.first->second = socketId;
    }

    struct ClientData socketData;
    socketData.userId = userId;
    socketData.index = (long)m_socketIdSet.size();
    std::pair<SOCKETID_TO_CLIENT_T::iterator, bool> pairIBSU(
    m_clients.insert(SOCKETID_TO_CLIENT_T::value_type(
    socketId,socketData)));
    if(pairIBSU.second) {
        m_socketIdSet.push_back(socketId);
    } else {
        pairIBSU.first->second.userId = userId;
        uint64_t timerId = pairIBSU.first->second.timerId;
        if(ID_NULL != timerId) {
            pairIBSU.first->second.timerId = ID_NULL;
            CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
            pTMgr->Remove(timerId);
        }
    }

	return true;
}

void CAgentLogic::InsertSocketId(const SocketID& socketId) {
	CScopedWriteLock wrlock(m_socketIdsLock);
    struct ClientData socketData;
    socketData.userId = (uint32_t) -1;
    socketData.index = (long)m_socketIdSet.size();
    std::pair<SOCKETID_TO_CLIENT_T::iterator, bool> pairIBSU(
    m_clients.insert(SOCKETID_TO_CLIENT_T::value_type(
    socketId,socketData)));
    if(pairIBSU.second) {
        m_socketIdSet.push_back(socketId);
    } else {
        pairIBSU.first->second.userId = (uint32_t) -1;
        uint64_t timerId = pairIBSU.first->second.timerId;
        if(ID_NULL != timerId) {
            pairIBSU.first->second.timerId = ID_NULL;
            CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
            pTMgr->Remove(timerId);
        }
    }
    pairIBSU.first->second.timerId = CGuidFactory::Pointer()->CreateGuid();
    CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
    CAutoPointer<CallbackMFnP1<CAgentLogic, unsigned int> >
        callback(new CallbackMFnP1<CAgentLogic, unsigned int>
        (m_pThis(), &CAgentLogic::LoginTimeout, socketId.index));
    pTMgr->SetTimeout(pairIBSU.first->second.timerId,
        LOGIN_EXPIRY_TIME, callback);
}

bool CAgentLogic::RemoveSocketId(const SocketID& socketId, uint64_t& outUserId) {
	CScopedWriteLock wrlock(m_socketIdsLock);
	SOCKETID_TO_CLIENT_T::iterator itSU = m_clients.find(socketId);
	if(m_clients.end() != itSU) {
		USERID_TO_SOCKETID_T::const_iterator itI = m_socketIds.find(itSU->second.userId);
        if(m_socketIds.end() != itI && itI->second == socketId) {
            m_socketIds.erase(itI);
        }
		outUserId = itSU->second.userId;
        uint64_t timerId = itSU->second.timerId;
        const SocketID& socketIdLast = m_socketIdSet[m_socketIdSet.size() - 1];
        m_socketIdSet[itSU->second.index] = socketIdLast;
        SOCKETID_TO_CLIENT_T::iterator itSULast = m_clients.find(socketIdLast);
        if(m_clients.end() != itSULast) {
            itSULast->second.index = itSU->second.index;
        }
        m_socketIdSet.pop_back();
        m_clients.erase(itSU);
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
	USERID_TO_SOCKETID_T::iterator itUS = m_socketIds.find(userId);
	if(m_socketIds.end() == itUS) {
		return false;
	}
	outSocketId = itUS->second;
	return true;
}

bool CAgentLogic::FindUserId(unsigned int socketIdx, uint64_t& outUserId)
{
	CScopedReadLock rdlock(m_socketIdsLock);
	SocketID socketId;
	socketId.index = socketIdx;
	SOCKETID_TO_CLIENT_T::iterator itSU = m_clients.find(socketId);
	if(m_clients.end() == itSU) {
		return false;
	}
	outUserId = itSU->second.userId;
	return true;
}

bool CAgentLogic::SendToClient(const SocketID& socketId,
	const ::node::DataPacket& message)
{
	const_cast<::node::DataPacket&>(message).clear_route();
	const_cast<::node::DataPacket&>(message).clear_route_type();

	SmallBuffer smallbuf(message.ByteSize());
	if(!message.SerializeToArray((char*)smallbuf, message.ByteSize())) {
		OutputError("!message.SerializeToArray");
		return false;
	}

	return m_tcpServer.Send((unsigned char*)(char*)smallbuf,
		message.ByteSize(), socketId);
}

bool CAgentLogic::ReceivePacket(SocketID& outSocketId, std::vector<::node::DataPacket>& outPackets) {
    long socketCount = GetArrSocketIdSize();
    if(socketCount < 1) {
        return false;
    }
    unsigned long sktIdxStart = m_sktIdxCount;
    unsigned long sktIdxEnd = m_sktIdxCount + socketCount;
    while(--socketCount > -1) {
        if(sktIdxStart < sktIdxEnd) {
            if(m_sktIdxCount < sktIdxStart
                || m_sktIdxCount >= sktIdxEnd)
            {
                break;
            }
        } else if(sktIdxStart > sktIdxEnd) {
            if(m_sktIdxCount < sktIdxStart
                && m_sktIdxCount >= sktIdxEnd)
            {
                break;
            }
        }
        outSocketId = GetArrSocketId();
		int nSize = m_tcpServer.ReceiveSize(outSocketId);
		for(int i = 0; i < nSize; ++i) {
			Packet* pPacket = m_tcpServer.Receive(outSocketId);
			if(NULL == pPacket) {
				break;
			}
			outPackets.push_back(node::DataPacket());
			::node::DataPacket& dispatchRequest = outPackets.back();
			if(!dispatchRequest.ParseFromArray((char*)pPacket->data, pPacket->length)) {
				OutputError("!dispatchRequest.ParseFromArray");
				outPackets.pop_back();
			}
			m_tcpServer.DeallocatePacket(pPacket, outSocketId);
		}
		if(!outPackets.empty()) {
			return true;
		}
    }
    return false;
}

void CAgentLogic::HandleLogin(const ::node::DataPacket& dispatchRequest,
	const SocketID& socketId)
{
	if(SERVER_STATUS_STOP == g_serverStatus) {
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(SERVER_ERROR_SERVER_STOP);
		SendToClient(socketId, dispatchResponse);
		return;
	}

	if(!dispatchRequest.has_data()) {
		OutputError("!dispatchRequest.has_data()");
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(FALSE);
		SendToClient(socketId, dispatchResponse);
		return;
	}

	const std::string& bytes = dispatchRequest.data();
	agent::LoginRequest loginRequest;
	if(!loginRequest.ParseFromArray(bytes.data(), (int)bytes.length())) {
		OutputError("!loginRequest.ParseFromArray");
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(FALSE);
		SendToClient(socketId, dispatchResponse);
		return;
	}

	uint64_t u64UserId = ID_NULL;
	std::string strCreationTime;
	uint32_t u32ServerRegion = ID_NULL;
	uint64_t u64Cas = 0;
	CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
	if(SERVER_SUCCESS != pCtrlCenStubImpEx->CheckUserId(GetServantConnect(),
		loginRequest.account(), u64UserId, strCreationTime, u32ServerRegion, u64Cas))
	{
		OutputError("TRUE != controlCentreStubEx.CheckUserId");
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(FALSE);
		SendToClient(socketId, dispatchResponse);
		return;
	}

	if(loginRequest.userid() != u64UserId) {
        OutputError("loginRequest.userid()["I64FMTD"] != u64UserId["I64FMTD"]"
			, loginRequest.userid(), u64UserId);
        ::node::DataPacket dispatchResponse;
        dispatchResponse.set_cmd(dispatchRequest.cmd());
        dispatchResponse.set_result(FALSE);
        SendToClient(socketId, dispatchResponse);
        return;
    }

	if(true) {
		char szMd5Buf[eBUF_SIZE_128] = {'\0'};
		snprintf(szMd5Buf, sizeof(szMd5Buf), I64FMTD""I64FMTD"%s",
			loginRequest.account(), u64UserId, strCreationTime.c_str());
		szMd5Buf[sizeof(szMd5Buf) - 1] = '\0';
		MD5_CTX md5;
		if(loginRequest.sessionkey() != md5.MakePassMD5(szMd5Buf)) {
			OutputError("loginRequest.sessionkey() != md5.MakePassMD5(szMd5Buf)");
			::node::DataPacket dispatchResponse;
			dispatchResponse.set_cmd(dispatchRequest.cmd());
			dispatchResponse.set_result(FALSE);
			SendToClient(socketId, dispatchResponse);
			return;
		}
	}

	uint32_t u32NewRegion = GetServerRegoin();
    if(u32ServerRegion != u32NewRegion) {
		if(SERVER_SUCCESS != pCtrlCenStubImpEx->UpdateUserRegion(
			GetServantConnect(), loginRequest.userid(), u32NewRegion, u64Cas))
		{
			OutputError("TRUE != controlCentreStubEx.UpdateUserRegion");
		}
	}

	KickCacheLogged(loginRequest.userid());

	if(!InsertSocketId(loginRequest.userid(), socketId)) {
        OutputError("!InsertSocketId  userId = "I64FMTD, loginRequest.userid());
        ::node::DataPacket dispatchResponse;
        dispatchResponse.set_cmd(dispatchRequest.cmd());
        dispatchResponse.set_result(FALSE);
        SendToClient(socketId, dispatchResponse);
        return;
    }

	::node::DataPacket dispatchAll;
	dispatchAll.set_cmd(dispatchRequest.cmd());
	dispatchAll.set_route(loginRequest.userid());
    OutputDebug("userId = "I64FMTD, loginRequest.userid());
	::node::LoginRequest loginAllRequest;
	assert(!m_strBind.empty());
	loginAllRequest.set_originip(m_strBind);
	loginAllRequest.set_routecount(1);
	loginAllRequest.set_account(loginRequest.account());
    loginAllRequest.set_version(loginRequest.version());
	loginAllRequest.set_remoteip(socketId.ToString(true));

	if(!SerializeWorkerData(dispatchAll, loginAllRequest)){
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		dispatchResponse.set_result(FALSE);
		SendToClient(socketId, dispatchResponse);
		return;
	}

	::node::DataPacket dispatchResponse;
	SendWorkerProtocol(dispatchAll, dispatchResponse);

	dispatchResponse.set_cmd(dispatchRequest.cmd());
	SendToClient(socketId, dispatchResponse);
}

void CAgentLogic::HandleDefault(const ::node::DataPacket& dispatchRequest,
	const SocketID& socketId)
{
	uint64_t userId(DEFAULT_USERID);
	if(!FindUserId(socketId.index, userId)) {
		OutputError("!FindUserId");
		return;
	}
    OutputDebug("userId = "I64FMTD, userId);
	const_cast<::node::DataPacket&>(dispatchRequest).set_route(userId);
	::node::DataPacket dispatchResponse;
	SendWorkerProtocol(dispatchRequest, dispatchResponse);
	if(dispatchResponse.has_result() || dispatchResponse.has_data()) {
		dispatchResponse.set_cmd(dispatchRequest.cmd());
		SendToClient(socketId, dispatchResponse);
	}
}

void CAgentLogic::HandleLogout(const SocketID& socketId, ::node::DataPacket& dispatchResponse, int32_t nWhy)
{
	uint64_t userId(DEFAULT_USERID);
	if(RemoveSocketId(socketId, userId)) {
		::node::DataPacket dispatchAll;
		dispatchAll.set_cmd(P_CMD_S_LOGOUT);
		dispatchAll.set_route(userId);
		if(0 != nWhy) {
			dispatchAll.set_result(nWhy);
		}
		SendWorkerProtocol(dispatchAll, dispatchResponse);
	}
}

void CAgentLogic::LoginTimeout(unsigned int& socketIdx)
{
    SocketID socketId;
    socketId.index = socketIdx;
    m_tcpServer.CloseConnection(socketId);
}

bool CAgentLogic::KickLogged(uint64_t userId)
{
	SocketID oldSocketId;
	if(FindSocketId(userId, oldSocketId)) {
		// tell client
		::node::DataPacket dispatchResponse;
		dispatchResponse.set_cmd(P_CMD_S_LOGOUT);
		dispatchResponse.set_result(SERVER_ERROR_ALREADY_EXIST);
		SendToClient(oldSocketId, dispatchResponse);
		// set close
		uint64_t timerId = CGuidFactory::Pointer()->CreateGuid();
		CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
		CAutoPointer<CallbackMFnP1<CAgentLogic, unsigned int> >
			callback(new CallbackMFnP1<CAgentLogic, unsigned int>
			(m_pThis(), &CAgentLogic::LoginTimeout, oldSocketId.index));
		pTMgr->SetTimeout(timerId, KEEP_FOR_CLOSE_CONNECTION, callback);
		return true;
	}
	return false;
}



