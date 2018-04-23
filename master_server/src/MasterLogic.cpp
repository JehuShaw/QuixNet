/*
 * File:   MasterLogic.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#include "MasterLogic.h"
#include "Log.h"
#include "Md5.h"
#include "CacheOperateHelper.h"
#include "WorkerOperateHelper.h"
#include "msg_master_login.pb.h"
#include "msg_client_login.pb.h"
#include "TimerManager.h"
#include "GuidFactory.h"
#include "CallBack.h"
#include "MasterCmdManager.h"
#include "MasterWebRequest.h"
#include "HttpClientManager.h"
#include "SessionEventManager.h"
#include "msg_master_data.pb.h"

using namespace ntwk;
using namespace evt;
using namespace thd;
using namespace util;

CMasterLogic::CMasterLogic()
	: m_sktIdxCount(-1)
{
	m_pThis(this);
}

CMasterLogic::~CMasterLogic() {
}

bool CMasterLogic::Init(const std::string& strAddress,
	uint16_t usMaxLink, uint32_t u32MaxPacketSize)
{
    CAutoPointer<MemberMethodRIP1<CMasterLogic> > pMethod(new MemberMethodRIP1<CMasterLogic>(
        m_pThis(), &CMasterLogic::HandleSessionResult));
    CSessionEventManager::PTR_T pSessionEventMgr(CSessionEventManager::Pointer());
    pSessionEventMgr->AddEventListener(SESSION_EVENT_LOGIN_CHECK, pMethod);
    m_tcpServer.SetLinkEvent(this);
	return m_tcpServer.Start(strAddress.c_str(), usMaxLink, u32MaxPacketSize);
}

void CMasterLogic::Dispose() {
	// close tcp socket
	m_tcpServer.Stop();
    m_tcpServer.SetLinkEvent(NULL);
    CSessionEventManager::PTR_T pSessionEventMgr(CSessionEventManager::Pointer());
    pSessionEventMgr->RemoveEventListener(SESSION_EVENT_LOGIN_CHECK);
}

bool CMasterLogic::Run() {

	SocketID socketId;
	//
	if(m_tcpServer.HasNewConnection(socketId)){
		PrintBasic("CMasterLogic recv ip address = %s  port = %d........\n",
			socketId.ToString(false), socketId.port);
		InsertSocketId(socketId);
	}
	//
	if(m_tcpServer.HasLostConnection(socketId)){
		PrintBasic("CMasterLogic lost ip address = %s  port = %d........\n",
			socketId.ToString(false), socketId.port);
		HandleLogout(socketId);
	}
	//
    std::vector<::node::MasterDataPacket> outPackets;
    if(ReceivePacket(socketId, outPackets)) {
        for(int i = 0; i < (int)outPackets.size(); ++i) {
            ::node::MasterDataPacket& dataRequest = outPackets[i];

            switch(dataRequest.cmd()) {
            case C_CMD_CTM_LOGIN:
                HandleLogin(dataRequest, socketId);
                break;
            case C_CMD_CTM_LOGOUT:
                // Don't do anything here.
                break;
			case C_CMD_CTM_RESTART:
				HandleRestart(dataRequest, socketId);
				break;
			case C_CMD_CTM_AUTORESTART:
				HandleAutoRestart(dataRequest, socketId);
				break;
			case C_CMD_CTM_SHUTDOWN:
				HandleShutdown(dataRequest, socketId);
				break;
			case C_CMD_CTM_ERASE:
				HandleErase(dataRequest, socketId);
				break;
            default:
                HandleDefault(dataRequest, socketId);
                break;
            }
        }
    }
	return false;
}

void CMasterLogic::OnShutdown() {
}

bool CMasterLogic::SendToClient(uint32_t account, const ::node::MasterDataPacket& message)
{
	SocketID socketId;
	if(!FindSocketId(account, socketId)) {
		return false;
	}

	SmallBuffer smallbuf(message.ByteSize());
	if(!message.SerializeToArray((char*)smallbuf, message.ByteSize())) {
		OutputError("!pMessage->SerializeToArray");
		return false;
	}

	return m_tcpServer.Send((unsigned char*)(char*)smallbuf,
		message.ByteSize(), socketId);
}

void CMasterLogic::CloseClient(uint32_t account) {

    SocketID socketId;
    if(!FindSocketId(account, socketId)) {
        return;
    }

    m_tcpServer.CloseConnection(socketId);
}

void CMasterLogic::OnAccept() {
    ThreadPool.ExecuteTask(this);
}

void CMasterLogic::OnDisconnect() {
    ThreadPool.ExecuteTask(this);
}

void CMasterLogic::OnReceive() {
    ThreadPool.ExecuteTask(this);
}

long CMasterLogic::GetLinkCount() {
    CScopedReadLock rdlock(m_socketIdsLock);
    return (long)m_clients.size();
}

bool CMasterLogic::InsertSocketId(uint32_t account, const SocketID& socketId) {
	CScopedWriteLock wrlock(m_socketIdsLock);

	std::pair<ACCOUNT_TO_SOCKETID_T::iterator, bool> pairIBUS(
	m_socketIds.insert(ACCOUNT_TO_SOCKETID_T::value_type(
	account, socketId)));
    if(!pairIBUS.second) {
        if(pairIBUS.first->second == socketId) {
            return false;
        }
        pairIBUS.first->second = socketId;
    }

    struct ClientData socketData;
    socketData.account = account;
    socketData.index = (long)m_socketIdSet.size();
    std::pair<SOCKETID_TO_CLIENT_T::iterator, bool> pairIBSU(
    m_clients.insert(SOCKETID_TO_CLIENT_T::value_type(
    socketId,socketData)));
    if(pairIBSU.second) {
        m_socketIdSet.push_back(socketId);
    } else {
        pairIBSU.first->second.account = account;
        uint64_t timerId = pairIBSU.first->second.timerId;
        if(ID_NULL != timerId) {
            pairIBSU.first->second.timerId = ID_NULL;
            CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
            pTMgr->Remove(timerId);
        }
    }

	return true;
}

void CMasterLogic::InsertSocketId(const SocketID& socketId) {
	CScopedWriteLock wrlock(m_socketIdsLock);
    struct ClientData socketData;
    socketData.account = (uint32_t) -1;
    socketData.index = (long)m_socketIdSet.size();
    std::pair<SOCKETID_TO_CLIENT_T::iterator, bool> pairIBSU(
    m_clients.insert(SOCKETID_TO_CLIENT_T::value_type(
    socketId,socketData)));
    if(pairIBSU.second) {
        m_socketIdSet.push_back(socketId);
    } else {
        pairIBSU.first->second.account = (uint32_t) -1;
        uint64_t timerId = pairIBSU.first->second.timerId;
        if(ID_NULL != timerId) {
            pairIBSU.first->second.timerId = ID_NULL;
            CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
            pTMgr->Remove(timerId);
        }
    }
    pairIBSU.first->second.timerId = CGuidFactory::Pointer()->CreateGuid();
    CTimerManager::PTR_T pTMgr(CTimerManager::Pointer());
    CAutoPointer<CallbackMFnP1<CMasterLogic, unsigned int> >
        callback(new CallbackMFnP1<CMasterLogic, unsigned int>
        (m_pThis(), &CMasterLogic::LoginTimeout, socketId.index));
    pTMgr->SetTimeout(pairIBSU.first->second.timerId,
        LOGIN_EXPIRY_TIME, callback);
}

bool CMasterLogic::RemoveSocketId(const SocketID& socketId, uint32_t& outAccount) {
	CScopedWriteLock wrlock(m_socketIdsLock);
	SOCKETID_TO_CLIENT_T::iterator itSU = m_clients.find(socketId);
	if(m_clients.end() != itSU) {
		ACCOUNT_TO_SOCKETID_T::const_iterator itI = m_socketIds.find(itSU->second.account);
        if(m_socketIds.end() != itI && itI->second == socketId) {
            m_socketIds.erase(itI);
        }
		outAccount = itSU->second.account;
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

bool CMasterLogic::FindSocketId(uint32_t account, SocketID& outSocketId) {
	CScopedReadLock rdlock(m_socketIdsLock);
	ACCOUNT_TO_SOCKETID_T::iterator itUS = m_socketIds.find(account);
	if(m_socketIds.end() == itUS) {
		return false;
	}
	outSocketId = itUS->second;
	return true;
}

bool CMasterLogic::FindUserId(unsigned int socketIdx, uint32_t& outAccount)
{
	CScopedReadLock rdlock(m_socketIdsLock);
	SocketID socketId;
	socketId.index = socketIdx;
	SOCKETID_TO_CLIENT_T::iterator itSU = m_clients.find(socketId);
	if(m_clients.end() == itSU) {
		return false;
	}
	outAccount = itSU->second.account;
	return true;
}

bool CMasterLogic::SendToClient(const SocketID& socketId,
	const ::node::MasterDataPacket& message)
{
	SmallBuffer smallbuf(message.ByteSize());
	if(!message.SerializeToArray((char*)smallbuf, message.ByteSize())) {
		OutputError("!message.SerializeToArray");
		return false;
	}

	return m_tcpServer.Send((unsigned char*)(char*)smallbuf,
		message.ByteSize(), socketId);
}

bool CMasterLogic::ReceivePacket(SocketID& outSocketId, std::vector<::node::MasterDataPacket>& outPackets) {
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
			outPackets.push_back(node::MasterDataPacket());
			::node::MasterDataPacket& dataRequest = outPackets.back();
			if(!dataRequest.ParseFromArray((char*)pPacket->data, pPacket->length)) {
				OutputError("!dataRequest.ParseFromArray");
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

void CMasterLogic::HandleLogin(const ::node::MasterDataPacket& dataRequest,
	const SocketID& socketId)
{
	if(!dataRequest.has_data()) {
		OutputError("!dispatchRequest.has_data()");
		::node::MasterDataPacket dataResponse;
		dataResponse.set_cmd(dataRequest.cmd());
		dataResponse.set_result(FALSE);
		SendToClient(socketId, dataResponse);
		return;
	}

	master::LoginRequest loginRequest;
    const std::string& bytes = dataRequest.data();
	if(!loginRequest.ParseFromArray(bytes.data(), (int)bytes.length())) {
		OutputError("!loginRequest.ParseFromArray");
		::node::MasterDataPacket dataResponse;
		dataResponse.set_cmd(dataRequest.cmd());
		dataResponse.set_result(FALSE);
		SendToClient(socketId, dataResponse);
		return;
	}

    AppConfig::PTR_T pConfig(AppConfig::Pointer());
    char szBuffer[eBUF_SIZE_1024] = {'\0'};

	snprintf(szBuffer, sizeof(szBuffer), "%u%s%u%s", loginRequest.account(),
		loginRequest.sessionkey().c_str(), socketId.index, AUTH_KEY);
	szBuffer[sizeof(szBuffer) - 1] = '\0';

    MD5_CTX md5;
    std::string strAuthKey(md5.MakePassMD5(szBuffer));

    snprintf(szBuffer, sizeof(szBuffer), "%s?accountId=%u&sessionKey=%s&socketIdx=%u&authKey=%s",
        pConfig->GetString(APPCONFIG_LOGINCHECKWEB).c_str(),
        loginRequest.account(),
        loginRequest.sessionkey().c_str(),
        socketId.index,
        strAuthKey.c_str());
	szBuffer[sizeof(szBuffer) - 1] = '\0';

    PrintDebug("CMasterLogic URL = %s", szBuffer);
    CMasterWebRequest* pMasterRequest = CMasterWebRequest::Create(
        szBuffer, socketId.index, loginRequest.account());
    CHttpClientManager::Pointer()->AddRequest(pMasterRequest);
}

void CMasterLogic::HandleRestart(
	const ::node::MasterDataPacket& dataRequest,
	const ntwk::SocketID& socketId)
{
	uint32_t account(DEFAULT_USERID);
	if(!FindUserId(socketId.index, account)) {
		OutputError("!FindUserId");
		::node::MasterDataPacket dataResponse;
		dataResponse.set_cmd(dataRequest.cmd());
		dataResponse.set_result(FALSE);
		SendToClient(socketId, dataResponse);
		return;
	}
	CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
	int32_t nResult = pMasterCmdMgr->Restart(dataRequest.serverid());
	////////////////////////////////////////////////////////////////////
	::node::MasterDataPacket dataResponse;
	dataResponse.set_cmd(dataRequest.cmd());
	dataResponse.set_result(nResult);
	SendToClient(socketId, dataResponse);
}

void CMasterLogic::HandleAutoRestart(
	const ::node::MasterDataPacket& dataRequest,
	const ntwk::SocketID& socketId)
{
	uint32_t account(DEFAULT_USERID);
	if(!FindUserId(socketId.index, account)) {
		OutputError("!FindUserId");
		::node::MasterDataPacket dataResponse;
		dataResponse.set_cmd(dataRequest.cmd());
		dataResponse.set_result(FALSE);
		SendToClient(socketId, dataResponse);
		return;
	}

	if(!dataRequest.has_result()) {
		OutputError("!dataRequest.has_result()");
		::node::MasterDataPacket dataResponse;
		dataResponse.set_cmd(dataRequest.cmd());
		dataResponse.set_result(FALSE);
		SendToClient(socketId, dataResponse);
		return;
	}

	if(TRUE == dataRequest.result()) {
		atomic_xchg8(&g_bAutoRestart, true);
	} else if(FALSE == dataRequest.result()) {
		atomic_xchg8(&g_bAutoRestart, false);
	} else {
		OutputError("!TRUE AND !FALSE");
		::node::MasterDataPacket dataResponse;
		dataResponse.set_cmd(dataRequest.cmd());
		dataResponse.set_result(FALSE);
		SendToClient(socketId, dataResponse);
		return;
	}
	::node::MasterDataPacket dataResponse;
	dataResponse.set_cmd(dataRequest.cmd());
	dataResponse.set_result(TRUE);
	SendToClient(socketId, dataResponse);
}

void CMasterLogic::HandleShutdown(
	const ::node::MasterDataPacket& dataRequest,
	const SocketID& socketId)
{
	uint32_t account(DEFAULT_USERID);
	if(!FindUserId(socketId.index, account)) {
		OutputError("!FindUserId");
		::node::MasterDataPacket dataResponse;
		dataResponse.set_cmd(dataRequest.cmd());
		dataResponse.set_result(FALSE);
		SendToClient(socketId, dataResponse);
		return;
	}
	CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
	int32_t nResult = pMasterCmdMgr->Shutdown(dataRequest.serverid());
	////////////////////////////////////////////////////////////////////
	::node::MasterDataPacket dataResponse;
	dataResponse.set_cmd(dataRequest.cmd());
	dataResponse.set_result(nResult);
	SendToClient(socketId, dataResponse);
}

void CMasterLogic::HandleErase(
	const ::node::MasterDataPacket& dataRequest,
	const SocketID& socketId)
{
	uint32_t account(DEFAULT_USERID);
	if(!FindUserId(socketId.index, account)) {
		OutputError("!FindUserId");
		::node::MasterDataPacket dataResponse;
		dataResponse.set_cmd(dataRequest.cmd());
		dataResponse.set_result(FALSE);
		SendToClient(socketId, dataResponse);
		return;
	}
	CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
	int32_t nResult = pMasterCmdMgr->Erase(dataRequest.serverid());
	////////////////////////////////////////////////////////////////////
	::node::MasterDataPacket dataResponse;
	dataResponse.set_cmd(dataRequest.cmd());
	dataResponse.set_result(nResult);
	SendToClient(socketId, dataResponse);
}

void CMasterLogic::HandleDefault(
    const ::node::MasterDataPacket& dataRequest,
	const SocketID& socketId)
{
	uint32_t account(DEFAULT_USERID);
	if(!FindUserId(socketId.index, account)) {
		OutputError("!FindUserId");
		::node::MasterDataPacket dataResponse;
		dataResponse.set_cmd(dataRequest.cmd());
		dataResponse.set_result(FALSE);
		SendToClient(socketId, dataResponse);
		return;
	}
    OutputDebug("account = %u", account);

	int32_t nResult = FALSE;
	if(dataRequest.has_serverid()) {
		CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
		nResult = pMasterCmdMgr->SendByServId(dataRequest.serverid(), dataRequest.cmd(),
			dataRequest.data());
	} else if(dataRequest.has_servername() && dataRequest.has_userid()) {
		CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
		nResult = pMasterCmdMgr->SendByUserId(dataRequest.servername(),
			dataRequest.userid(), dataRequest.cmd(), dataRequest.data());
	}

	::node::MasterDataPacket dataResponse;
	dataResponse.set_cmd(dataRequest.cmd());
	dataResponse.set_result(nResult);
	SendToClient(socketId, dataResponse);
}

void CMasterLogic::HandleLogout(const SocketID& socketId)
{
	uint32_t account(DEFAULT_USERID);
	RemoveSocketId(socketId, account);
}

void CMasterLogic::LoginTimeout(unsigned int& socketIdx)
{
    SocketID socketId;
    socketId.index = socketIdx;
    m_tcpServer.CloseConnection(socketId);
}

int CMasterLogic::HandleSessionResult(const util::CWeakPointer<ArgumentBase>& arg)
{
    util::CWeakPointer<CArgBitStream> pArg(arg);
    if(pArg.IsInvalid()) {
        OutputError("pArg.IsInvalid()");
        return FALSE;
    }

    bool bSuccess = pArg->ReadBool();
    uint32_t uAccount = pArg->ReadUInt32();
    int32_t nSocketIdx = pArg->ReadInt32();
    SocketID socketId;
    socketId.index = nSocketIdx;
    if(!bSuccess) {
        OutputError("!bSuccess");

        ::node::MasterDataPacket dataResponse;
        dataResponse.set_cmd(C_CMD_CTM_LOGIN);
        dataResponse.set_result(FALSE);

        SendToClient(socketId, dataResponse);
        return FALSE;
    }

    SocketID oldSocketId;
    if(FindSocketId(uAccount, oldSocketId)) {
        if(nSocketIdx != oldSocketId.index) {
            HandleLogout(oldSocketId);
            m_tcpServer.CloseConnection(oldSocketId);
        }
    }

    if(!InsertSocketId(uAccount, socketId)) {
        OutputError("!InsertSocketId  account = %u ", uAccount);
        ::node::MasterDataPacket dataResponse;
        dataResponse.set_cmd(C_CMD_CTM_LOGIN);
        dataResponse.set_result(FALSE);
        SendToClient(socketId, dataResponse);
        return FALSE;
    }

    ::node::MasterDataPacket dataResponse;
	dataResponse.set_cmd(C_CMD_CTM_LOGIN);
    dataResponse.set_result(TRUE);
    SendToClient(socketId, dataResponse);
    return TRUE;
}



