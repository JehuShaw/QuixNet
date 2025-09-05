#include "CacheServiceImp.h"

#include "ModuleManager.h"
#include "BodyMessage.h"
#include "NodeDefines.h"
#include "CacheOperateHelper.h"
#include "CacheMemoryManager.h"
#include "CacheDBManager.h"
#include "WorkerOperateHelper.h"
#include "ControlCentreStubImpEx.h"
#include "TransferStream.h"
#include "CachePlayer.h"
#include "DBIDManager.h"

#include "msg_client_login.pb.h"

using namespace util;
using namespace db;


inline bool HandleLogin(
	::node::DataPacket& request,
	const std::string& strBind,
	const std::string& servantAddress,
	const std::string& serverName,
	uint32_t serverId)
{
	assert(HasDataPacketData(request));
	assert(!strBind.empty());

	uint64_t userId = request.route();

	::node::LoginRequest loginRequest;
	if(!ParseCacheData(loginRequest, request)) {
		OutputError("!ParseWorkerData userId = " I64FMTD, userId);
		return false;
	}
	uint32_t routeCount = loginRequest.routecount();
	std::string originIp(loginRequest.originip());

	loginRequest.set_originip(strBind);
	loginRequest.set_routecount(routeCount + 1);
	if(!SerializeCacheData(request, loginRequest)) {
		OutputError("!SerializeWorkerData userId = " I64FMTD, userId);
		return false;
	}

	CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
	pChlMgr->SetClientChannel(userId, originIp, routeCount);
	if(!servantAddress.empty()) {
		CControlCentreStubImpEx::PTR_T pCtrlCentreStubImpEx(CControlCentreStubImpEx::Pointer());
		pCtrlCentreStubImpEx->UserLogin(servantAddress, serverName, serverId, userId);
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
		CControlCentreStubImpEx::PTR_T pCtrlCentreStubImpEx(CControlCentreStubImpEx::Pointer());
		pCtrlCentreStubImpEx->UserLogout(servantAddress, serverName, userId);
	}
	return true;
}

inline static bool IncorrectKeyFormat(const std::string& strKey, const char* fun) {

	util::CTransferStream tranKey(strKey.data(), strKey.length(), false);
	uint8_t u8Type = STREAM_DATA_NIL;
	tranKey >> u8Type;

	if(!IsStringType(u8Type)) {
		PrintError("CCacheServiceImp @%s !IsStringType u8Type = %d ", fun, (int32_t)u8Type);
		return true;
	}

	uint16_t u16Length = 0;
	tranKey >> u16Length;
	if(u16Length > 0) {
		tranKey.IgnoreBits(TS_BYTES_TO_BITS(u16Length));
	} else {
		PrintError("CCacheServiceImp @%s strTableName.empty()", fun);
		return true;
	}

	if(tranKey.GetNumberOfUnreadBits() < 1) {
		PrintError("CCacheServiceImp @%s strKey.empty() ", fun);
		return true;
	}
	return false;
}

inline static bool EmptyTableFormat(const std::string& strKey, const char* fun) {

	util::CTransferStream tranKey(strKey.data(), strKey.length(), false);
	uint8_t u8Type = STREAM_DATA_NIL;
	tranKey >> u8Type;

	if(!IsStringType(u8Type)) {
		PrintError("CCacheServiceImp @%s !IsStringType u8Type = %d ", fun, (int32_t)u8Type);
		return true;
	}

	uint16_t u16Length = 0;
	tranKey >> u16Length;
	if(u16Length == 0) {
		PrintError("CCacheServiceImp @%s strTableName.empty()", fun);
		return true;
	}
	return false;
}

uint16_t CCacheServiceImp::GetDBIDFromDBByUserID(uint64_t userId) {
	CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
	uint16_t u16DBID = pCacheDBMgr->GetDBIdByUserId(userId);
	if(0 == u16DBID) {
		u16DBID = pCacheDBMgr->GetMinLoadDBIdByBalUserId(userId);
	}
	return u16DBID;
}

CCacheServiceImp::CCacheServiceImp(
	const std::string& endPoint,
	const std::string& servantAddress,
	const std::string& serverName,
	const std::string& agentName,
	uint32_t serverId,
	CreatePlayerMethod createPlayerMethod /*= NULL*/
	)
	: m_endPoint(endPoint)
	, m_servantAddress(servantAddress)
	, m_serverName(serverName)
	, m_agentName(agentName)
	, m_serverId(serverId)
	, m_createPlayerMethod(createPlayerMethod)
{
}


CCacheServiceImp::~CCacheServiceImp(void)
{
}

void CCacheServiceImp::ListNotificationInterests(
	const ::node::VoidPacket& request,
	::rpcz::reply<::node::InterestPacket> response)
{
	::node::InterestPacket cacheInterest;
	cacheInterest.add_interests(N_CMD_ADD);
    cacheInterest.add_interests(N_CMD_LOAD);
    cacheInterest.add_interests(N_CMD_STORE);
    cacheInterest.add_interests(N_CMD_GET);
	cacheInterest.add_interests(N_CMD_SET);
    cacheInterest.add_interests(N_CMD_GETS);
	cacheInterest.add_interests(N_CMD_CAS);
	cacheInterest.add_interests(N_CMD_DEL);
	cacheInterest.add_interests(N_CMD_LOADALL);
    cacheInterest.add_interests(N_CMD_DB_INSERT);
    cacheInterest.add_interests(N_CMD_DB_SELECT);
    cacheInterest.add_interests(N_CMD_DB_UPDATE);
    cacheInterest.add_interests(N_CMD_DB_DELETE);
    cacheInterest.add_interests(N_CMD_DB_SELECTALL);
    cacheInterest.add_interests(N_CMD_DB_ESCAPESTRING);
    cacheInterest.add_interests(N_CMD_DB_STOREDPROCEDURES);
	cacheInterest.add_interests(N_CMD_DB_ASYNCSTOREDPROCEDURES);
	cacheInterest.add_interests(N_CMD_DB_CHECK_GLOBAL_EXISTS);
	cacheInterest.add_interests(N_CMD_DB_CHKEXIST_ESCAPESTRING);
	cacheInterest.add_interests(N_CMD_SEND_TO_CLIENT);
	cacheInterest.add_interests(N_CMD_BROADCAST_TO_CLIENT);
	cacheInterest.add_interests(N_CMD_CLOSE_CLIENT);
	cacheInterest.add_interests(N_CMD_CLOSE_ALLCLIENTS);
	cacheInterest.add_interests(N_CMD_SEND_TO_WORKER);
	cacheInterest.add_interests(N_CMD_KICK_LOGGED);
	cacheInterest.add_interests(N_CMD_ALL_DB_STOREDPROCEDURES);
	cacheInterest.add_interests(N_CMD_SEND_TO_PLAYER);
	cacheInterest.add_interests(N_CMD_POST_TO_PLAYER);
	response.send(cacheInterest);
}

void CCacheServiceImp::ListProtocolInterests(
	const ::node::VoidPacket& request,
	::rpcz::reply< ::node::InterestPacket> response)
{
	// attach protocols interest.
	::node::InterestPacket cacheInterest;
	cacheInterest.add_interests(P_CMD_C_LOGIN);
	cacheInterest.add_interests(P_CMD_S_LOGOUT);
	// send result
	response.send(cacheInterest);
}

void CCacheServiceImp::HandleNotification(const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket> response)
{
	int nOperate = request.cmd();
	switch(nOperate) {
    case N_CMD_GETS:
        HandleGets(request, response);
        break;
    case N_CMD_CAS:
        HandleCAS(request, response);
        break;
    case N_CMD_LOAD:
        HandleLoad(request, response);
        break;
    case N_CMD_STORE:
        HandleStore(request, response);
        break;
	case N_CMD_ADD:
		HandleAdd(request, response);
		break;
    case N_CMD_GET:
        HandleGet(request, response);
        break;
	case N_CMD_SET:
		HandleSet(request, response);
		break;
	case N_CMD_DEL:
		HandleDel(request, response);
		break;
	case N_CMD_LOADALL:
		HandleLoadAll(request, response);
		break;
    case N_CMD_DB_INSERT:
        HandleDBInsert(request, response);
        break;
    case N_CMD_DB_SELECT:
        HandleDBSelect(request, response);
        break;
    case N_CMD_DB_UPDATE:
        HandleDBUpdate(request, response);
        break;
    case N_CMD_DB_DELETE:
        HandleDBDelete(request, response);
        break;
    case N_CMD_DB_SELECTALL:
        HandleDBSelectAll(request, response);
        break;
    case N_CMD_DB_ESCAPESTRING:
        HandleDBEscapeString(request, response);
        break;
    case N_CMD_DB_STOREDPROCEDURES:
        HandleDBStoredProcedures(request, response);
        break;
	case N_CMD_DB_ASYNCSTOREDPROCEDURES:
		HandleDBAsyncStoredProcedures(request, response);
		break;
	case N_CMD_DB_CHECK_GLOBAL_EXISTS:
		HandleDBCheckGlobalExists(request, response);
		break;
	case N_CMD_DB_CHKEXIST_ESCAPESTRING:
		HandleDBCheckEscapeString(request, response);
		break;
	case N_CMD_SEND_TO_CLIENT:
		HandleSendToClient(request, response);
		break;
	case N_CMD_BROADCAST_TO_CLIENT:
		HandleBroadcastToClient(request, response);
		break;
	case N_CMD_CLOSE_CLIENT:
		HandleCloseClient(request, response);
		break;
	case N_CMD_CLOSE_ALLCLIENTS:
		HandleCloseAllClients(request, response);
		break;
	case N_CMD_SEND_TO_WORKER:
		HandleSendToWorker(request, response);
		break;
	case N_CMD_KICK_LOGGED:
		HandleKickLogged(request, response);
		break;
	case N_CMD_SEND_TO_PLAYER:
		HandleSendToPlayer(request, response);
		break;
	case N_CMD_POST_TO_PLAYER:
		HandlePostToPlayer(request, response);
		break;
	case N_CMD_SERVER_PLAY:
		HandlePlay(request, response);
		break;
	case N_CMD_SERVER_STOP:
		HandleStop(request, response);
		break;
	case N_CMD_ALL_DB_STOREDPROCEDURES:
		HandleAllDBStoredProcedures(request, response);
		break;
	default:
		HandleDefault(request, response);
		break;
	}

	if (request.route_type() == ROUTE_BALANCE_SERVERID) {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		pChlMgr->AddRouteServer(request.route());
	}
}

void CCacheServiceImp::HandleProtocol(const ::node::DataPacket& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	if(request.route() == ID_NULL) {
		::node::DataPacket dspResponse;
		SendCacheProtocol(request, dspResponse);
		dspResponse.set_cmd(request.cmd());
		dspResponse.set_result(ROUTE_ID_NULL);
		response.send(dspResponse);
		return;
	}

	if(request.cmd() == P_CMD_C_LOGIN) {
		if (request.sub_cmd() != LOGIN_RECOVER) {
			if (!HandleLogin(const_cast<::node::DataPacket&>(request),
				m_endPoint, m_servantAddress, m_serverName, m_serverId))
			{
				::node::DataPacket dspResponse;
				dspResponse.set_cmd(request.cmd());
				dspResponse.set_result(SERVER_FAILURE);
				response.send(dspResponse);
				return;
			}
		}
		::node::DataPacket dspResponse;

		if(NULL == m_createPlayerMethod) {
			SendCacheProtocol(request, dspResponse);
		} else {
			CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
			CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(request.route()));
			if(pPlayer.IsInvalid()) {
				CAutoPointer<CPlayerBase> pNewPlayer((*m_createPlayerMethod)(request.route()));
				assert(!pNewPlayer.IsInvalid());
				pChlMgr->SetPlayer(request.route(), pNewPlayer);
				CDBIDManager<uint64_t, BALUSERID_TAG>::PTR_T pBalUserIDMgr(
					CDBIDManager<uint64_t, BALUSERID_TAG>::Pointer());
				pBalUserIDMgr->Remove(request.route());
				CScopedPlayerMutex scopedPlayerMutex(pNewPlayer);
				SendCacheProtocol(request, dspResponse, pNewPlayer);
			} else {
				CScopedPlayerMutex scopedPlayerMutex(pPlayer);
				SendCacheProtocol(request, dspResponse, pPlayer);
			}
		}

		dspResponse.set_cmd(request.cmd());
		response.send(dspResponse);

	} else if(request.cmd() == P_CMD_S_LOGOUT) {

		::node::DataPacket dspResponse;

		if(NULL == m_createPlayerMethod) {
			SendCacheProtocol(request, dspResponse);
		} else {
			CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
			CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(request.route()));
			if(pPlayer.IsInvalid()) {
				SendCacheProtocol(request, dspResponse, pPlayer);
			} else {
				CScopedPlayerMutex scopedPlayerMutex(pPlayer);
				SendCacheProtocol(request, dspResponse, pPlayer);
			}
		}

		if(!HandleLogout(const_cast<::node::DataPacket&>(request),
			m_servantAddress, m_serverName))
		{
			::node::DataPacket dspResponse;
			dspResponse.set_cmd(request.cmd());
			dspResponse.set_result(SERVER_FAILURE);
			response.send(dspResponse);
		} else {
			dspResponse.set_cmd(request.cmd());
			response.send(dspResponse);
		}

	} else {
		::node::DataPacket dspResponse;

		if(NULL == m_createPlayerMethod) {
			SendCacheProtocol(request, dspResponse);
		} else {
			CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
			CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(request.route()));
			if(pPlayer.IsInvalid()) {
				SendCacheProtocol(request, dspResponse, pPlayer);
			} else {
				CScopedPlayerMutex scopedPlayerMutex(pPlayer);
				SendCacheProtocol(request, dspResponse, pPlayer);
			}
		}

		dspResponse.set_cmd(request.cmd());
		response.send(dspResponse);
	}
}

void CCacheServiceImp::HandleAdd(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    mc_response_t cacheResponse;
    CCacheMemoryManager::PTR_T pCacheMemMgr(CCacheMemoryManager::Pointer());

    for(int i = 0; i < nSize; ++i)
    {
        mc_record_t* pMcRecord = SetRecord(cacheResponse);
        if(NULL == pMcRecord) {
            OutputError("NULL == pMcRecord");
            continue;
        }

        const mc_record_t& mcRequest = GetRecord(cacheRequest, i);
		const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcRecord, strKey);

		if(IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcRecord, MCERR_NOTSTORED);
			continue;
		}

		const std::string& strValue = GetRecordValue(mcRequest);
		uint64_t outCas = 0;
        if(pCacheMemMgr->AddCacheRecord(u16DBID, strKey, strValue, outCas)) {
            SetRecordResult(pMcRecord, MCERR_OK);
        } else {
            SetRecordResult(pMcRecord, MCERR_NOTSTORED);
        }
		SetRecordCas(pMcRecord, outCas);
    }

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
    SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleLoad(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    mc_response_t cacheResponse;
    CCacheMemoryManager::PTR_T pCacheMemMgr(CCacheMemoryManager::Pointer());

	for (int i = 0; i < nSize; ++i)
	{
		mc_record_t* pMcResponse = SetRecord(cacheResponse);
		if (NULL == pMcResponse) {
			OutputError("NULL == pMcRecord");
			continue;
		}
		const mc_record_t& mcRequest = GetRecord(cacheRequest, i);

		const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcResponse, strKey);

		if (IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcResponse, MCERR_NOREPLY);
			continue;
		}

		bool bHasCas = HasRecordCas(mcRequest);
		CAutoPointer<ICacheMemory> pOutRecord;
		int nResult = pCacheMemMgr->LoadCacheRecord(pOutRecord,
			u16DBID, strKey, bHasCas);

		if (MCERR_OK != nResult) {
			SetRecordResult(pMcResponse, nResult);
		} else {
			CAutoPointer<CCacheMemory> pCacheMemory(pOutRecord);
			if (pCacheMemory.IsInvalid()) {
				SetRecordResult(pMcResponse, MCERR_NOTFOUND);
			} else {
				if (bHasCas) {
					uint64_t n64Cas;
					CTransferStream tranValue;
					SetRecordResult(pMcResponse, pCacheMemory->GetsValue(tranValue, n64Cas));
					SetRecordNValue(pMcResponse, tranValue.GetData(), tranValue.GetNumberOfBytesUsed());
					SetRecordCas(pMcResponse, n64Cas);
				} else {
					CTransferStream tranValue;
					pCacheMemory->GetValue(tranValue);
					SetRecordNValue(pMcResponse, tranValue.GetData(), tranValue.GetNumberOfBytesUsed());
					SetRecordResult(pMcResponse, MCERR_OK);
				}
			}
		}
	}

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleStore(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    mc_response_t cacheResponse;
    CCacheMemoryManager::PTR_T pCacheMemMgr(CCacheMemoryManager::Pointer());

    for(int i = 0; i < nSize; ++i)
    {
        mc_record_t* pMcRecord = SetRecord(cacheResponse);
        if(NULL == pMcRecord) {
            OutputError("NULL == pMcRecord");
            continue;
        }

        const mc_record_t& mcRequest = GetRecord(cacheRequest, i);
		const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcRecord, strKey);

		if(IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcRecord, MCERR_NOTSTORED);
			continue;
		}

        if(HasRecordValue(mcRequest)) {
			CTransferStream tranValue(GetRecordValue(mcRequest), false);
            CAutoPointer<CCacheMemory> pCacheMemory(pCacheMemMgr->RemoveMemoryRecord(strKey));
            if(pCacheMemory.IsInvalid()) {
				if(HasRecordCas(mcRequest)) {
					CCacheMemory cacheMemory(u16DBID, strKey, GetRecordCas(mcRequest));
					cacheMemory.SetValue(tranValue);
					SetRecordResult(pMcRecord, cacheMemory.UpdateToDB(true));
				} else {
					CCacheMemory cacheMemory(u16DBID, strKey);
					cacheMemory.SetValue(tranValue);
					SetRecordResult(pMcRecord, cacheMemory.UpdateToDB(false));
				}
            } else {
				if(HasRecordCas(mcRequest)) {
					SetRecordResult(pMcRecord, pCacheMemory->CheckAndSetValue(
						tranValue, GetRecordCas(mcRequest)));
				} else {
					pCacheMemory->SetValue(tranValue);
					SetRecordResult(pMcRecord, MCERR_OK);
				}
				pCacheMemMgr->CheckUpdateAndReloadRecord(pCacheMemory);
            }
        } else {
            pCacheMemMgr->RemoveCacheRecord(strKey, true);
            SetRecordResult(pMcRecord, MCERR_OK);
        }
    }
	
	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleGet(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    mc_response_t cacheResponse;
	CCacheMemoryManager::PTR_T pCacheMemMgr(CCacheMemoryManager::Pointer());

    for(int i = 0; i < nSize; ++i)
    {
        mc_record_t* pMcRecord = SetRecord(cacheResponse);
        if(NULL == pMcRecord) {
            OutputError("NULL == pMcRecord");
            continue;
        }

        const mc_record_t& mcRequest = GetRecord(cacheRequest, i);
		const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcRecord, strKey);

		if(IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcRecord, MCERR_NOREPLY);
			continue;
		}

        CAutoPointer<CCacheMemory> pCacheMemory(pCacheMemMgr->FindMemoryRecord(strKey));
        if(pCacheMemory.IsInvalid()) {
			CAutoPointer<ICacheMemory> pOutRecord;
            int nResult = pCacheMemMgr->LoadCacheRecord(pOutRecord, u16DBID, strKey, false);
            if(MCERR_OK != nResult) {
				if(MCERR_NOTFOUND != nResult) {
					OutputError("MCERR_OK != cacheMemoryManager.LoadCacheRecord "
                    "nResult = %d strKey = %s", nResult, strKey.c_str());
				}
                SetRecordResult(pMcRecord, nResult);
                continue;
            }
            pCacheMemory = pOutRecord;
        }

        if(pCacheMemory.IsInvalid()) {
            OutputError("pCacheMemory.IsInvalid() strKey = %s", strKey.c_str());
            SetRecordResult(pMcRecord, MCERR_NOREPLY);
        } else {
			CTransferStream tranValue;
			pCacheMemory->GetValue(tranValue);
            SetRecordNValue(pMcRecord, tranValue.GetData(), tranValue.GetNumberOfBytesUsed());
			SetRecordResult(pMcRecord, MCERR_OK);
        }
    }

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleSet(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    mc_response_t cacheResponse;
	CCacheMemoryManager::PTR_T pCacheMemMgr(CCacheMemoryManager::Pointer());

    for(int i = 0; i < nSize; ++i)
    {
        mc_record_t* pMcRecord = SetRecord(cacheResponse);
        if(NULL == pMcRecord) {
            OutputError("NULL == pMcRecord");
            continue;
        }

        const mc_record_t& mcRequest = GetRecord(cacheRequest, i);
		const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcRecord, strKey);

		if(IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcRecord, MCERR_NOTSTORED);
			continue;
		}

        CTransferStream tranValue(GetRecordValue(mcRequest), false);
        CAutoPointer<CCacheMemory> pCacheMemory(pCacheMemMgr->FindMemoryRecord(strKey));
        if(pCacheMemory.IsInvalid()) {
			CAutoPointer<ICacheMemory> pOutRecord;
            int nResult = pCacheMemMgr->LoadCacheRecord(pOutRecord, u16DBID, strKey, false);
            if(MCERR_OK != nResult) {
				OutputError("MCERR_OK != cacheMemoryManager.LoadCacheRecord "
                    "nResult = %d strKey = %s", nResult, strKey.c_str());
                SetRecordResult(pMcRecord, nResult);
                continue;
            }
            pCacheMemory = pOutRecord;
            if(pCacheMemory.IsInvalid()) {
                OutputError("pCacheMemory.IsInvalid() strKey = %s", strKey.c_str());
                SetRecordResult(pMcRecord, MCERR_NOTSTORED);
            } else {
                pCacheMemory->SetValue(tranValue);
                SetRecordResult(pMcRecord, MCERR_OK);
            }
        } else {
            pCacheMemory->SetValue(tranValue);
            SetRecordResult(pMcRecord, MCERR_OK);
        }
    }

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleGets(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    mc_response_t cacheResponse;
	CCacheMemoryManager::PTR_T pCacheMemMgr(CCacheMemoryManager::Pointer());

    for(int i = 0; i < nSize; ++i)
    {
        mc_record_t* pMcRecord = SetRecord(cacheResponse);
        if(NULL == pMcRecord) {
            OutputError("NULL == pMcRecord");
            continue;
        }

        const mc_record_t& mcRequest = GetRecord(cacheRequest, i);
		const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcRecord, strKey);

		if(IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcRecord, MCERR_NOREPLY);
			continue;
		}

        CAutoPointer<CCacheMemory> pCacheMemory(pCacheMemMgr->FindMemoryRecord(strKey));
        if(pCacheMemory.IsInvalid()) {
			CAutoPointer<ICacheMemory> pOutRecord;
            int nResult = pCacheMemMgr->LoadCacheRecord(pOutRecord, u16DBID, strKey, true);
            if(MCERR_OK != nResult) {
				if(MCERR_NOTFOUND != nResult) {
					OutputError("MCERR_OK != cacheMemoryManager.LoadCacheRecord "
                    "nResult = %d strKey = %s", nResult, strKey.c_str());
				}
                SetRecordResult(pMcRecord, nResult);
                continue;
            }
            pCacheMemory = pOutRecord;
        }

        if(pCacheMemory.IsInvalid()) {
            OutputError("pCacheMemory.IsInvalid() strKey = %s", strKey.c_str());
            SetRecordResult(pMcRecord, MCERR_NOREPLY);
        } else {
            uint64_t n64Cas;
			CTransferStream tranValue;
            SetRecordResult(pMcRecord, pCacheMemory->GetsValue(tranValue, n64Cas));
            SetRecordNValue(pMcRecord, tranValue.GetData(), tranValue.GetNumberOfBytesUsed());
            SetRecordCas(pMcRecord, n64Cas);
        }
    }

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleCAS(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    mc_response_t cacheResponse;
	CCacheMemoryManager::PTR_T pCacheMemMgr(CCacheMemoryManager::Pointer());

    for(int i = 0; i < nSize; ++i)
    {
        mc_record_t* pMcRecord = SetRecord(cacheResponse);
        if(NULL == pMcRecord) {
            OutputError("NULL == pMcRecord");
            continue;
        }

        const mc_record_t& mcRequest = GetRecord(cacheRequest, i);
		const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcRecord, strKey);

		if(IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcRecord, MCERR_NOTSTORED);
			continue;
		}

        CTransferStream tranValue(GetRecordValue(mcRequest), false);
        CAutoPointer<CCacheMemory> pCacheMemory(pCacheMemMgr->FindMemoryRecord(strKey));
        if(pCacheMemory.IsInvalid()) {
			CAutoPointer<ICacheMemory> pOutRecord;
            int nResult = pCacheMemMgr->LoadCacheRecord(pOutRecord,
				u16DBID, strKey, true, GetRecordCas(mcRequest));
            if(MCERR_OK != nResult) {
                OutputError("MCERR_OK != cacheMemoryManager.LoadCacheRecord "
                    "nResult = %d strKey = %s", nResult, strKey.c_str());
                SetRecordResult(pMcRecord, nResult);
                continue;
            }
            pCacheMemory = pOutRecord;
            if(pCacheMemory.IsInvalid()) {
                OutputError("pCacheMemory.IsInvalid() strKey = %s", strKey.c_str());
                SetRecordResult(pMcRecord, MCERR_NOTSTORED);
				continue;
            }
        }
		MCResult mcRet = pCacheMemory->CheckAndSetValue(
			tranValue, GetRecordCas(mcRequest));
		if(MCERR_EXISTS == mcRet) {
			CAutoPointer<ICacheMemory> pOutRecord;
			int nResult = pCacheMemMgr->LoadCacheRecord(pOutRecord,
				u16DBID, strKey, true, GetRecordCas(mcRequest));
			if(MCERR_OK == nResult) {
				if(pOutRecord.IsInvalid()) {
					OutputError("pOutRecord.IsInvalid() strKey = %s", strKey.c_str());
				} else {
					pCacheMemory = pOutRecord;
					if(pCacheMemory.IsInvalid()) {
						OutputError("pCacheMemory.IsInvalid() strKey = %s", strKey.c_str());
						SetRecordResult(pMcRecord, MCERR_NOTSTORED);
						continue;
					} else {
						mcRet = pCacheMemory->CheckAndSetValue(
							tranValue, GetRecordCas(mcRequest));
					}
				}
			} else {
				OutputError("MCERR_OK != pCacheMemMgr->LoadCacheRecord "
					"nResult = %d strKey = %s", nResult, strKey.c_str());
			}
		}
		SetRecordResult(pMcRecord, mcRet);
		SetRecordCas(pMcRecord, pCacheMemory->GetCas());
    }

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleDel(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    mc_response_t cacheResponse;
	CCacheMemoryManager::PTR_T pCacheMemMgr(CCacheMemoryManager::Pointer());

    for(int i = 0; i < nSize; ++i)
    {
        mc_record_t* pMcRecord = SetRecord(cacheResponse);
        if(NULL == pMcRecord) {
            OutputError("NULL == pMcRecord");
            continue;
        }

        const mc_record_t& mcRequest = GetRecord(cacheRequest, i);
		const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcRecord, strKey);

		if(IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcRecord, MCERR_NOTSTORED);
			continue;
		}

        if(pCacheMemMgr->RemoveAndDeleteFromDb(u16DBID, strKey)) {
            SetRecordResult(pMcRecord, MCERR_OK);
        } else {
            SetRecordResult(pMcRecord, MCERR_NOTSTORED);
        }
    }

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleLoadAll(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	if(!HasRecordKey(cacheRequest)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_KEY);
		response.send(outResponse);
		OutputError("!HasRecordKey");
		return;
	}

	const std::string& strKey = GetRecordKey(cacheRequest);

	if(EmptyTableFormat(strKey, __FUNCTION__)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_TABLE);
		response.send(outResponse);
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

	mc_response_t cacheResponse;
	cacheResponse.set_key(strKey);

	uint32_t nOffset = cacheRequest.offset();
	uint32_t nCount = cacheRequest.count();

	cacheResponse.set_result(CCacheMemory::SelectAllFromDB(u16DBID,
		strKey, nOffset, nCount, cacheResponse.mutable_values(), true));

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}


void CCacheServiceImp::HandleDBInsert(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    mc_response_t cacheResponse;

    for(int i = 0; i < nSize; ++i)
    {
        mc_record_t* pMcRecord = SetRecord(cacheResponse);
        if(NULL == pMcRecord) {
            OutputError("NULL == pMcRecord");
            continue;
        }

        const mc_record_t& mcRequest = GetRecord(cacheRequest, i);
        const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcRecord, strKey);

		if(IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcRecord, MCERR_NOTSTORED);
			continue;
		}

		CTransferStream tranValue(GetRecordValue(mcRequest), false);

        CCacheMemory cacheMemory(u16DBID, strKey);
        cacheMemory.SetValue(tranValue);
        SetRecordResult(pMcRecord, cacheMemory.AddToDB());
    }

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleDBSelect(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    mc_response_t cacheResponse;

    for(int i = 0; i < nSize; ++i)
    {
        mc_record_t* pMcRecord = SetRecord(cacheResponse);
        if(NULL == pMcRecord) {
            OutputError("NULL == pMcRecord");
            continue;
        }

        const mc_record_t& mcRequest = GetRecord(cacheRequest, i);
		const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcRecord, strKey);

		if(IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcRecord, MCERR_NOREPLY);
			continue;
		}

		if(HasRecordCas(mcRequest)) {
			CCacheMemory cacheMemory(u16DBID, strKey);
			SetRecordResult(pMcRecord, cacheMemory.LoadFromDB(false));
			CTransferStream tranValue;
			cacheMemory.GetValue(tranValue);
			SetRecordNValue(pMcRecord, tranValue.GetData(), tranValue.GetNumberOfBytesUsed());
		} else {
			CCacheMemory cacheMemory(u16DBID, strKey);
			SetRecordResult(pMcRecord, cacheMemory.LoadFromDB(true));
			CTransferStream tranValue;
			cacheMemory.GetValue(tranValue);
			SetRecordNValue(pMcRecord, tranValue.GetData(), tranValue.GetNumberOfBytesUsed());
			SetRecordCas(pMcRecord, cacheMemory.GetCas());
		}
    }

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleDBUpdate(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    mc_response_t cacheResponse;

    for(int i = 0; i < nSize; ++i)
    {
        mc_record_t* pMcRecord = SetRecord(cacheResponse);
        if(NULL == pMcRecord) {
            OutputError("NULL == pMcRecord");
            continue;
        }

        const mc_record_t& mcRequest = GetRecord(cacheRequest, i);
		const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcRecord, strKey);

		if(IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcRecord, MCERR_NOTSTORED);
			continue;
		}

		CTransferStream tranValue(GetRecordValue(mcRequest), false);

		if(HasRecordCas(mcRequest)) {
			CCacheMemory cacheMemory(u16DBID, strKey, GetRecordCas(mcRequest));
			cacheMemory.SetValue(tranValue);
			SetRecordResult(pMcRecord, cacheMemory.UpdateToDB(true));
		} else {
			CCacheMemory cacheMemory(u16DBID, strKey);
			cacheMemory.SetValue(tranValue);
			SetRecordResult(pMcRecord, cacheMemory.UpdateToDB(false));
		}
    }

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleDBDelete(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    mc_response_t cacheResponse;

    for(int i = 0; i < nSize; ++i)
    {
        mc_record_t* pMcRecord = SetRecord(cacheResponse);
        if(NULL == pMcRecord) {
            OutputError("NULL == pMcRecord");
            continue;
        }

        const mc_record_t& mcRequest = GetRecord(cacheRequest, i);
		const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcRecord, strKey);

		if(IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcRecord, MCERR_NOTSTORED);
			continue;
		}

        CCacheMemory cacheMemory(u16DBID, strKey);
        SetRecordResult(pMcRecord, cacheMemory.DeleteFromDB());
    }

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleDBSelectAll(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	if(!HasRecordKey(cacheRequest)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_KEY);
		response.send(outResponse);
		OutputError("!HasRecordKey");
		return;
	}

	const std::string& strKey = GetRecordKey(cacheRequest);

	if(EmptyTableFormat(strKey, __FUNCTION__)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_TABLE);
		response.send(outResponse);
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

	mc_response_t cacheResponse;
	cacheResponse.set_key(strKey);

    uint32_t nOffset = cacheRequest.offset();
    uint32_t nCount = cacheRequest.count();

    cacheResponse.set_result(CCacheMemory::SelectAllFromDB(u16DBID,
		strKey, nOffset, nCount, cacheResponse.mutable_values(), false));

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleDBEscapeString(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
    if(!HasDataPacketData(request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_DATA);
		response.send(outResponse);
        OutputError("!HasDataPacketData");
        return;
    }

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
    CAutoPointer<Database> pDataBase(pCacheDBMgr->GetDatabase(u16DBID));
    if(pDataBase.IsInvalid()) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_DATABASE_INVALID);
		response.send(outResponse);
        OutputError("pDataBase.IsInvalid()");
        return;
    }

    std::stringstream outString;
    const std::string& strEscape = request.data();
    pDataBase->EscapeLongString(strEscape.c_str(), strEscape.length(), outString);

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	outResponse.set_data(outString.str());
	response.send(outResponse);
}

void CCacheServiceImp::HandleDBStoredProcedures(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

    if(!HasRecordKey(cacheRequest)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_KEY);
		response.send(outResponse);
        OutputError("!HasRecordKey");
        return;
    }

    const std::string& strKey = GetRecordKey(cacheRequest);

	if(EmptyTableFormat(strKey, __FUNCTION__)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_TABLE);
		response.send(outResponse);
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

    mc_response_t cacheResponse;
    cacheResponse.set_key(strKey);

	int nResultFlag = cacheRequest.result();

	cacheResponse.set_result(CCacheMemory::StoredProcedures(u16DBID,
		strKey, cacheRequest.data(), nResultFlag & MCERR_ESCAPE_STRING,
		cacheResponse.mutable_values()));

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleDBAsyncStoredProcedures(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	if(!HasRecordKey(cacheRequest)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_KEY);
		response.send(outResponse);
		OutputError("!HasRecordKey");
		return;
	}

	const std::string& strKey = GetRecordKey(cacheRequest);

	if(EmptyTableFormat(strKey, __FUNCTION__)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_TABLE);
		response.send(outResponse);
		return;
	}

	uint16_t u16DBID = GetDBID(request.route_type(), request.route());

	mc_response_t cacheResponse;
	cacheResponse.set_key(strKey);

	int nResultFlag = cacheRequest.result();
	if(nResultFlag & MCERR_NOREPLY) {
		cacheResponse.set_result(CCacheMemory::AsyncStoredProcedures(
			u16DBID, strKey, cacheRequest.data(), nResultFlag & MCERR_ESCAPE_STRING, NULL));
	} else {
		cacheResponse.set_result(CCacheMemory::AsyncStoredProcedures(
			u16DBID, strKey, cacheRequest.data(), nResultFlag & MCERR_ESCAPE_STRING,
			cacheResponse.mutable_values()));
	}

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleDBCheckGlobalExists(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if (!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if (nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("request size() < 1 ");
		return;
	}

	mc_response_t cacheResponse;

	for (int i = 0; i < nSize; ++i)
	{
		mc_record_t* pMcRecord = SetRecord(cacheResponse);
		if (NULL == pMcRecord) {
			OutputError("NULL == pMcRecord");
			continue;
		}

		const mc_record_t& mcRequest = GetRecord(cacheRequest, i);
		const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcRecord, strKey);

		if (IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcRecord, MCERR_NOREPLY);
			continue;
		}

		SetRecordResult(pMcRecord, CCacheMemory::CheckGlobalExists(strKey));
	}

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleDBCheckEscapeString(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if (!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if (nSize < 1) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		response.send(outResponse);
		OutputError("request size() < 1 ");
		return;
	}

	mc_response_t cacheResponse;

	for (int i = 0; i < nSize; ++i)
	{
		mc_record_t* pMcRecord = SetRecord(cacheResponse);
		if (NULL == pMcRecord) {
			OutputError("NULL == pMcRecord");
			continue;
		}

		const mc_record_t& mcRequest = GetRecord(cacheRequest, i);
		const std::string& strKey = GetRecordKey(mcRequest);
		

		if (IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcRecord, MCERR_NOREPLY);
			SetRecordKey(pMcRecord, strKey);
			continue;
		}

		util::CTransferStream tsNewKey;
		SetRecordResult(pMcRecord, CCacheMemory::CheckExistEscapeString(strKey, tsNewKey));
		uint32_t nNewKeySize = tsNewKey.GetNumberOfBytesUsed();
		if (nNewKeySize > 0) {
			SetRecordNKey(pMcRecord, tsNewKey.GetData(), nNewKeySize);
		} else {
			SetRecordKey(pMcRecord, strKey);
		}
	}

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleSendToClient(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	if(!HasDataPacketRoute(request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_ROUTE);
		response.send(outResponse);
		OutputError("!request.has_route()");
		return;
	}

	if(!HasDataPacketData(request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_DATA);
		response.send(outResponse);
		OutputError("!request.has_data() || request.data().empty() userId = " I64FMTD, request.route());
		return;
	}

	const std::string& bytes = request.data();
	::node::DataPacket dataPacket;
	if(!dataPacket.ParseFromArray(bytes.data(), bytes.length())) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		OutputError("!dataPacket.ParseFromArray userId = " I64FMTD, request.route());
		return;
	}

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	eServerError serRet = SendWorkerPacketToClient(dataPacket);
	outResponse.set_result(serRet);
	response.send(outResponse);

	if(SERVER_SUCCESS != serRet) {
		OutputError("SERVER_SUCCESS != SendWorkerPacketToClient userId = " I64FMTD " serRet = %d", request.route(), serRet);
	}
}

void CCacheServiceImp::HandleBroadcastToClient(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	if(!HasDataPacketData(request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_DATA);
		response.send(outResponse);
		OutputError("!request.has_data() || request.data().empty()");
		return;
	}

	const std::string& bytes = request.data();
	::node::BroadcastRequest broadcastPacket;
	if(!broadcastPacket.ParseFromArray(bytes.data(), bytes.length())) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		OutputError("!broadcastPacket.ParseFromArray(inRequest)");
		return;
	}
	
	BroadcastPacketToWorker(broadcastPacket);

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	response.send(outResponse);
}

void CCacheServiceImp::HandleCloseClient(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	if(!HasDataPacketRoute(request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_ROUTE);
		response.send(outResponse);
		OutputError("!HasDataPacketRoute");
		return;
	}

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	eServerError serRet = CloseWorkerClient(request.route());
	outResponse.set_result(serRet);
	response.send(outResponse);

	if(SERVER_SUCCESS != serRet) {
		OutputError("SERVER_SUCCESS != CloseWorkerClient userId = " I64FMTD " serRet = %d", request.route(), serRet);
	}
}

void CCacheServiceImp::HandleCloseAllClients(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	if (!HasDataPacketData(request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_DATA);
		response.send(outResponse);
		OutputError("!request.has_data() || request.data().empty()");
		return;
	}

	const std::string& bytes = request.data();
	::node::BroadcastRequest broadcastPacket;
	if (!broadcastPacket.ParseFromArray(bytes.data(), bytes.length())) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		OutputError("!broadcastPacket.ParseFromArray(inRequest)");
		return;
	}

	CloseWorkerAllClients(broadcastPacket);

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	response.send(outResponse);
}

void CCacheServiceImp::HandleSendToWorker(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	if(!HasDataPacketRoute(request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_ROUTE);
		response.send(outResponse);
		OutputError("!HasDataPacketRoute");
		return;
	}

	if(!HasDataPacketData(request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_DATA);
		response.send(outResponse);
		OutputError("!HasDataPacketData userId = " I64FMTD, request.route());
		return;
	}

	const std::string& bytes = request.data();
	::node::DataPacket dataPacket;
	if(!dataPacket.ParseFromArray(bytes.data(), bytes.length())) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		OutputError("!dataPacket.ParseFromArray userId = " I64FMTD, request.route());
		return;
	}

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	eServerError serRet = SendWorkerPacketToWorker(dataPacket);
	outResponse.set_result(serRet);
	response.send(outResponse);

	if(SERVER_SUCCESS != serRet) {
		OutputError("SERVER_SUCCESS != serRet userId = " I64FMTD " serRet = %d", request.route(), serRet);
	}
}

void CCacheServiceImp::HandleKickLogged(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	if(!HasDataPacketRoute(request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_ROUTE);
		response.send(outResponse);
		OutputError("!HasDataPacketRoute");
		return;
	}

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	eServerError serRet = KickWorkerLogged(request.route());
	outResponse.set_result(serRet);
	response.send(outResponse);

	if(SERVER_SUCCESS != serRet) {
		OutputError("SERVER_SUCCESS != KickWorkerLogged userId = " I64FMTD " serRet = %d", request.route(), serRet);
	}
}

void CCacheServiceImp::HandleSendToPlayer(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	if (!HasDataPacketRoute(request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_ROUTE);
		response.send(outResponse);
		OutputError("!HasDataPacketRoute");
		return;
	}

	if (!HasDataPacketData(request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_DATA);
		response.send(outResponse);
		OutputError("!HasDataPacketData userId = " I64FMTD, request.route());
		return;
	}

	const std::string& bytes = request.data();
	::node::ForwardRequest dataRequest;
	if (!dataRequest.ParseFromArray(bytes.data(), bytes.length())) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		OutputError("!dataPacket.ParseFromArray userId = " I64FMTD, request.route());
		return;
	}

	const ::node::DataPacket& dataPacket = dataRequest.data();

	if(m_servantAddress.empty()) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		response.send(outResponse);
		OutputError("m_servantAddress.empty() userId = " I64FMTD, dataPacket.route());
		return;
	}

	if(m_agentName.empty()) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		response.send(outResponse);
		OutputError("m_gentName.empty() userId = " I64FMTD, dataPacket.route());
		return;
	}

	uint32_t agentId = ID_NULL;
	uint32_t mapId = ID_NULL;
	CControlCentreStubImpEx::PTR_T pCtrlStubEx(CControlCentreStubImpEx::Pointer());
	pCtrlStubEx->SeizeServerReturnID(agentId, mapId,
		m_servantAddress, m_agentName, dataPacket.route(), false);
	if (ID_NULL == mapId) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(SERVER_ERROR_NOTFOUND_CHARACTER);
		response.send(outResponse);
		OutputError("ID_NULL == mapId userId = " I64FMTD, dataPacket.route());
		return;
	}
	if (ID_NULL == agentId) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		response.send(outResponse);
		OutputError("ID_NULL == agentId userId = " I64FMTD, dataPacket.route());
		return;
	}
	dataRequest.add_agentids(agentId);
	dataRequest.set_mapid(mapId);

	::node::DataPacket outResponse;
	SendWorkerPacketToPlayer(dataRequest, outResponse);
	response.send(outResponse);

	eServerError servRet = pCtrlStubEx->FreeServer(m_servantAddress, dataPacket.route(), false);
	if(SERVER_SUCCESS != servRet) {
		OutputError("SERVER_SUCCESS != pCtrlStubEx->FreeServer() userId = " I64FMTD, dataPacket.route());
	}
}

void CCacheServiceImp::HandlePostToPlayer(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	if (!HasDataPacketRoute(request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_ROUTE);
		response.send(outResponse);
		OutputError("!HasDataPacketRoute");
		return;
	}

	if (!HasDataPacketData(request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_DATA);
		response.send(outResponse);
		OutputError("!HasDataPacketData userId = " I64FMTD, request.route());
		return;
	}

	const std::string& bytes = request.data();
	::node::ForwardRequest dataRequest;
	if (!dataRequest.ParseFromArray(bytes.data(), bytes.length())) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		OutputError("!dataPacket.ParseFromArray userId = " I64FMTD, request.route());
		return;
	}

	const ::node::DataPacket& dataPacket = dataRequest.data();

	if (m_servantAddress.empty()) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		response.send(outResponse);
		OutputError("m_servantAddress.empty() userId = " I64FMTD, dataPacket.route());
		return;
	}

	if (m_agentName.empty()) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		response.send(outResponse);
		OutputError("m_gentName.empty() userId = " I64FMTD, dataPacket.route());
		return;
	}

	uint32_t agentId = ID_NULL;
	uint32_t mapId = ID_NULL;
	CControlCentreStubImpEx::PTR_T pCtrlStubEx(CControlCentreStubImpEx::Pointer());
	pCtrlStubEx->SeizeServerReturnID(agentId, mapId,
		m_servantAddress, m_agentName, dataPacket.route(), false);
	if (ID_NULL == mapId) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(SERVER_ERROR_NOTFOUND_CHARACTER);
		response.send(outResponse);
		OutputError("ID_NULL == mapId userId = " I64FMTD, dataPacket.route());
		return;
	}
	if (ID_NULL == agentId) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		response.send(outResponse);
		OutputError("ID_NULL == agentId userId = " I64FMTD, dataPacket.route());
		return;
	}
	dataRequest.add_agentids(agentId);
	dataRequest.set_mapid(mapId);
	// 返回结果 （这边先返回结果，在发送到远端）
	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	response.send(outResponse);
	// 发送到远端
	PostWorkerPacketToPlayer(dataRequest);

	eServerError servRet = pCtrlStubEx->FreeServer(m_servantAddress, dataPacket.route(), false);
	if (SERVER_SUCCESS != servRet) {
		OutputError("SERVER_SUCCESS != pCtrlStubEx->FreeServer() userId = " I64FMTD, dataPacket.route());
	}
}

void CCacheServiceImp::HandlePlay(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	atomic_xchg(&g_serverStatus, SERVER_STATUS_START);

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	response.send(outResponse);
}

void CCacheServiceImp::HandleStop(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	atomic_xchg(&g_serverStatus, SERVER_STATUS_STOP);

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	response.send(outResponse);
}

void CCacheServiceImp::HandleAllDBStoredProcedures(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	mc_request_t cacheRequest;
	if (!ParseCacheData(cacheRequest, request)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		response.send(outResponse);
		return;
	}

	if (!HasRecordKey(cacheRequest)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_KEY);
		response.send(outResponse);
		OutputError("!HasRecordKey");
		return;
	}

	const std::string& strKey = GetRecordKey(cacheRequest);

	if (EmptyTableFormat(strKey, __FUNCTION__)) {
		::node::DataPacket outResponse;
		outResponse.set_cmd(request.cmd());
		outResponse.set_result(CACHE_ERROR_EMPTY_TABLE);
		response.send(outResponse);
		return;
	}

	mc_response_t cacheResponse;
	cacheResponse.set_key(strKey);

	int nResultFlag = cacheRequest.result();
	if (nResultFlag & MCERR_NOREPLY) {
		cacheResponse.set_result(CCacheMemory::AllDBStoredProcedures(
			strKey, cacheRequest.data(), nResultFlag & MCERR_ESCAPE_STRING, NULL));
	} else {
		cacheResponse.set_result(CCacheMemory::AllDBStoredProcedures(
			strKey, cacheRequest.data(), nResultFlag & MCERR_ESCAPE_STRING, 
			cacheResponse.mutable_values()));
	}

	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
	response.send(outResponse);
}

void CCacheServiceImp::HandleDefault(
	const ::node::DataPacket& request,
	::rpcz::reply<::node::DataPacket>& response)
{
	::node::DataPacket outResponse;
	outResponse.set_cmd(request.cmd());
	outResponse.set_result(CACHE_ERROR_CMD_UNKNOWN);
	response.send(outResponse);
}

uint16_t CCacheServiceImp::GetDBID(int32_t nRouteType, uint64_t u64Route)
{
	uint16_t u16DBID = 0;
	if(ROUTE_BALANCE_USERID == nRouteType) {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<CCachePlayer> pCachePlayer(
			pChlMgr->GetPlayer(u64Route));
		if(pCachePlayer.IsInvalid()) {
			CDBIDManager<uint64_t, BALUSERID_TAG>::PTR_T pBalUserIDMgr(
				CDBIDManager<uint64_t, BALUSERID_TAG>::Pointer());
			if(!pBalUserIDMgr->Find(u64Route, u16DBID)) {
				u16DBID = GetDBIDFromDBByUserID(u64Route);
			}
		} else {
			u16DBID = pCachePlayer->GetDBID();
		}
	} else if(ROUTE_BALANCE_SERVERID == nRouteType || ROUTE_HASH_32KEY == nRouteType) {
		uint32_t uServerId = static_cast<uint32_t>(u64Route);
		CDBIDManager<uint32_t, BALSERVID_TAG>::PTR_T pBalServIDMgr(
			CDBIDManager<uint32_t, BALSERVID_TAG>::Pointer());
		if(!pBalServIDMgr->Find(uServerId, u16DBID)) {
			CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
			u16DBID = pCacheDBMgr->GetDBIdByBalServId(uServerId);
			if(0 == u16DBID) {
				u16DBID = pCacheDBMgr->GetMinLoadDBIdByBalServId(uServerId);
				if(0 != u16DBID) {
					pBalServIDMgr->Add(uServerId, u16DBID);
				} else {
					OutputError("u16DBId == 0");
					assert(false);
				}
			} else {
				pBalServIDMgr->Add(uServerId, u16DBID);
			}
		}
	} else if(ROUTE_DIRECT_SERVERID == nRouteType) {
		uint32_t uServerId = static_cast<uint32_t>(u64Route);
		CDBIDManager<uint32_t, DIRSERVID_TAG>::PTR_T pDirServIDMgr(
			CDBIDManager<uint32_t, DIRSERVID_TAG>::Pointer());
		if(!pDirServIDMgr->Find(uServerId, u16DBID)) {
			CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
			u16DBID = pCacheDBMgr->GetDBIdByDirServId(uServerId);
			if(0 == u16DBID) {
				u16DBID = pCacheDBMgr->GetMinLoadDBIdByDirServId(uServerId);
				if(0 != u16DBID) {
					pDirServIDMgr->Add(uServerId, u16DBID);
				} else {
					OutputError("u16DBId == 0");
					assert(false);
				}
			} else {
				pDirServIDMgr->Add(uServerId, u16DBID);
			}
		}
	}

	return u16DBID;
}





