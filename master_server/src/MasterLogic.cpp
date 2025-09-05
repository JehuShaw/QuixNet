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
#include "CallBack.h"
#include "MasterCmdManager.h"
#include "HttpClientManager.h"
#include "SessionEventManager.h"
#include "msg_master_data.pb.h"
#include "TimestampManager.h"
#include "Md5.h"
#include "MasterServer.h"

using namespace ntwk;
using namespace evt;
using namespace thd;
using namespace util;

CMasterLogic::CMasterLogic()
	: m_runningCount(0)
	, m_clientCount(0)
{
	m_pThis(this);
}

CMasterLogic::~CMasterLogic() {
}

bool CMasterLogic::Init(const std::string& strAddress,
	uint16_t usMaxLink, uint32_t u32MaxPacketSize)
{
    m_tcpServer.SetLinkEvent(this);
	m_queSocketIds.SetRawPointer(new SOCKETID_QUEUE_T());
	return m_tcpServer.Start(strAddress.c_str(), usMaxLink, u32MaxPacketSize);
}

void CMasterLogic::Dispose() {
	// close tcp socket
	m_tcpServer.Stop();
    m_tcpServer.SetLinkEvent(NULL);
	m_queSocketIds.SetRawPointer(NULL);
}

bool CMasterLogic::OnRun() {
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
				RemoveSocketId(socketId);

				socketId = newSocketId;
				PrintBasic("recv ip address = %s  port = %d........\n",
					socketId.ToString(false), socketId.port);
				InsertSocketId(socketId);
			} else {
				PrintBasic("lost ip address = %s  port = %d........\n",
					socketId.ToString(false), socketId.port);
				RemoveSocketId(socketId);
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

void CMasterLogic::OnShutdown() {
}

void CMasterLogic::OnAccept() {
	if (m_runningCount > static_cast<long>(m_tcpServer.Size())) {
		return;
	}
    ThreadPool.ExecuteTask(this);
}

void CMasterLogic::OnDisconnect() {
	if (m_runningCount > static_cast<long>(m_tcpServer.Size())) {
		return;
	}
    ThreadPool.ExecuteTask(this);
}

void CMasterLogic::OnReceive() {
	if (m_runningCount > static_cast<long>(m_tcpServer.Size())) {
		return;
	}
    ThreadPool.ExecuteTask(this);
}

long CMasterLogic::GetLinkCount() {
    return (long)m_clientCount;
}

void CMasterLogic::InsertSocketId(const SocketID& socketId) {
	CScopedWriteLock wrlock(m_socketIdsLock);

    std::pair<SOCKETIDS_T::iterator, bool> pairIB(m_clients.insert(socketId));
    if(pairIB.second) {
		atomic_inc(&m_clientCount);
		ntwk::SocketID* pSocketId = m_queSocketIds->WriteLock();
		*pSocketId = socketId;
		m_queSocketIds->WriteUnlock();
    }
}

bool CMasterLogic::RemoveSocketId(const SocketID& socketId) {
	CScopedWriteLock wrlock(m_socketIdsLock);
	SOCKETIDS_T::iterator itSU(m_clients.find(socketId));
	if(m_clients.end() != itSU) {
        m_clients.erase(itSU);
		atomic_dec(&m_clientCount);
		return true;
	}
	return false;
}

bool CMasterLogic::ExistSocketId(const ntwk::SocketID& socketId)
{
	CScopedReadLock rdlock(m_socketIdsLock);
	SOCKETIDS_T::iterator itSU(m_clients.find(socketId));
	if (m_clients.end() != itSU) {
		return true;
	}
	return false;
}

bool CMasterLogic::SendToClient(const SocketID& socketId,
	const ::node::MasterDataPacket& message)
{
	std::string bytes;
	if (!message.SerializeToString(&bytes)) {
		OutputError("!message.SerializeToString(byte)");
		return false;
	}

	return m_tcpServer.Send((unsigned char*)bytes.data(), bytes.size(), socketId);
}

void CMasterLogic::ReceivePacket() {
    
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

		::node::MasterDataPacket dataRequest;
		if (dataRequest.ParseFromArray((char*)pPacket->data, pPacket->length)) {
			if (!m_tcpServer.DeallocatePacket(pLinkData, pPacket)) {
				OutputError("!m_tcpServer.DeallocatePacket socketId = %s ", socketId.ToString());
			}
			if (HandlePacket(dataRequest, socketId)) {
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

bool CMasterLogic::CheckAdminAuth(const std::string& adminName, const std::string& authCode)
{
	util::CValueStream datas;
	datas.Serialize(NODE_ADMIN_CHECK);
	CRequestStoredProcs cacheRequest;
	cacheRequest.SetKey(datas);

	datas.Reset();

	datas.Serialize(adminName);
	cacheRequest.SetParams(datas);

	uint32_t u32Key = BKDRHash(adminName.c_str());

	CResponseRows cacheResponse;
	eServerError serError = McDBAsyncStoredProcHash32Key(
		u32Key, cacheRequest, cacheResponse);
	if (SERVER_SUCCESS != serError) {
		OutputError("SERVER_SUCCESS != McDBAsyncStoredProcHash32Key "
			"serError = %d adminName = %s", serError, adminName.c_str());
		return false;
	}

	int nSize = cacheResponse.GetSize();
	if (nSize > 0) {
		util::CValueStream tsValues(cacheResponse.GetValue(0));
		std::string password,webKey;
		tsValues.Parse(password);
		tsValues.Parse(webKey);
		if (!password.empty() && !webKey.empty()) {

			uint32_t timestamp = CTimestampManager::Pointer()->GetTimestamp() & ID_TYPE_32BIT_MASK;
			timestamp /= AUTH_TIMESTAMP_SPAN;

			//printf("adminName = %s password = %s webKey = %s timestamp = %u \n", adminName.c_str(), password.c_str(), webKey.c_str(), timestamp);

			char szMd5Buf[eBUF_SIZE_512] = { '\0' };
			int nOffset = snprintf(szMd5Buf, sizeof(szMd5Buf), "%s%s%s%u",
				adminName.c_str(), password.c_str(), webKey.c_str(), timestamp);
			szMd5Buf[sizeof(szMd5Buf) - 1] = '\0';
			std::string strSessionKey(util::md5(szMd5Buf, nOffset));
			if (strSessionKey == authCode) {
				return true;
			}
		}
	}
	return false;
}

bool CMasterLogic::HandlePacket(
	const ::node::MasterDataPacket& dataRequest,
	const ntwk::SocketID& socketId)
{
	if (!CheckAdminAuth(dataRequest.adminname(), dataRequest.authcode())) {
		OutputError("!CheckAdminAuth adminName = %s authCode = %s",
			dataRequest.adminname().c_str(), dataRequest.authcode().c_str());
		return true;
	}

	::node::MasterDataPacket dataResponse;

	switch (dataRequest.cmd()) {
	case C_CMD_CTM_BEGIN:
		HandleBegin(dataRequest, dataResponse, socketId);
		break;
	case C_CMD_CTM_STOP:
		HandleStop(dataRequest, dataResponse, socketId);
		break;
	case C_CMD_CTM_RESTART:
		HandleRestart(dataRequest, dataResponse, socketId);
		break;
	case C_CMD_CTM_AUTORESTART:
		HandleAutoRestart(dataRequest, dataResponse, socketId);
		break;
	case C_CMD_CTM_SHUTDOWN:
		HandleShutdown(dataRequest, dataResponse, socketId);
		break;
	case C_CMD_CTM_ERASE:
		HandleErase(dataRequest, dataResponse, socketId);
		break;
	default:
		HandleDefault(dataRequest, dataResponse, socketId);
		break;
	}
	////////////////////////////////////////////
	if (SERVER_NO_RESPOND != dataResponse.result()) {
		dataResponse.set_cmd(dataRequest.cmd());
		SendToClient(socketId, dataResponse);
	}
	return false;
}

void CMasterLogic::HandleBegin(
	const ::node::MasterDataPacket& dataRequest,
	::node::MasterDataPacket& dataResponse,
	const ntwk::SocketID& socketId)
{
	CMasterServer::PTR_T pMaster(CMasterServer::Pointer());
	pMaster->RemoveAutoPlayTimer();
	pMaster->OnServerPrepare();
	//////////////////////////////////////////////
	dataResponse.set_result(SERVER_SUCCESS);
}

void CMasterLogic::HandleStop(
	const ::node::MasterDataPacket& dataRequest,
	::node::MasterDataPacket& dataResponse,
	const ntwk::SocketID& socketId)
{
	CMasterServer::PTR_T pMaster(CMasterServer::Pointer());
	pMaster->RemoveAutoPlayTimer();

	atomic_xchg(&g_serverStatus, SERVER_STATUS_STOP);

	CNodeModule::BroadcastAllNodes(N_CMD_SERVER_STOP);
	//////////////////////////////////////////////////
	dataResponse.set_result(SERVER_SUCCESS);
}

void CMasterLogic::HandleRestart(
	const ::node::MasterDataPacket& dataRequest,
	::node::MasterDataPacket& dataResponse,
	const ntwk::SocketID& socketId)
{
	CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
	int32_t nResult = pMasterCmdMgr->Restart(dataRequest.serverid());
	////////////////////////////////////////////////////////////////////
	dataResponse.set_result(nResult);
}

void CMasterLogic::HandleAutoRestart(
	const ::node::MasterDataPacket& dataRequest,
	::node::MasterDataPacket& dataResponse,
	const ntwk::SocketID& socketId)
{
	if(TRUE == dataRequest.result()) {
		atomic_xchg8(&g_bAutoRestart, true);
	} else if(FALSE == dataRequest.result()) {
		atomic_xchg8(&g_bAutoRestart, false);
	} else {
		OutputError("!TRUE AND !FALSE");
		dataResponse.set_result(SERVER_FAILURE);
		return;
	}
	dataResponse.set_result(SERVER_SUCCESS);
}

void CMasterLogic::HandleShutdown(
	const ::node::MasterDataPacket& dataRequest,
	::node::MasterDataPacket& dataResponse,
	const SocketID& socketId)
{
	CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
	int32_t nResult = pMasterCmdMgr->Shutdown(dataRequest.serverid());
	////////////////////////////////////////////////////////////////////
	dataResponse.set_result(nResult);
}

void CMasterLogic::HandleErase(
	const ::node::MasterDataPacket& dataRequest,
	::node::MasterDataPacket& dataResponse,
	const SocketID& socketId)
{
	CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
	int32_t nResult = pMasterCmdMgr->Erase(dataRequest.serverid());
	////////////////////////////////////////////////////////////////////
	dataResponse.set_result(nResult);
}

void CMasterLogic::HandleDefault(
    const ::node::MasterDataPacket& dataRequest,
	::node::MasterDataPacket& dataResponse,
	const SocketID& socketId)
{
	if(dataRequest.userid() != ID_NULL) {
		CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
		dataResponse.set_result(pMasterCmdMgr->SendByUserId(dataRequest.userid(),
			dataRequest.cmd(), dataRequest.data(), *dataResponse.mutable_data()));
	} else if (dataRequest.serverid() != ID_NULL) {
		CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
		dataResponse.set_result(pMasterCmdMgr->SendByServId(dataRequest.serverid(),
			dataRequest.cmd(), dataRequest.data(), *dataResponse.mutable_data()));
	}
}




