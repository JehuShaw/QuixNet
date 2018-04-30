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
#include "BalServIDManager.h"
#include "DirServIDManager.h"

#include "msg_client_login.pb.h"

using namespace util;
using namespace db;


inline bool HandleLogin(
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
	if(!ParseCacheData(loginRequest, request)) {
		OutputError("!ParseWorkerData userId = "I64FMTD, userId);
		return false;
	}
	uint32_t routeCount = loginRequest.routecount();
	std::string originIp(loginRequest.originip());

	loginRequest.set_originip(strBind);
	loginRequest.set_routecount(routeCount + 1);
	if(!SerializeCacheData(request, loginRequest)) {
		OutputError("!SerializeWorkerData userId = "I64FMTD, userId);
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

inline static bool HandleLoadRecord(
	int32_t& nOutFlag,
	CCacheMemoryManager::PTR_T pCacheMemMgr,
	uint16_t u16DBID,
	uint16_t u16ServerId,
	const mc_record_t& mcRequest,
	mc_record_t* pMcResponse)
{
	const std::string& strKey = GetRecordKey(mcRequest);

	bool bHasCas = (!HasRecordCas(mcRequest) || GetRecordCas(mcRequest));
	CAutoPointer<ICacheMemory> pOutRecord;
	int32_t nInFlag = u16ServerId;
	int nResult = pCacheMemMgr->LoadCacheRecord(pOutRecord,
		u16DBID, strKey, bHasCas, 0, &nInFlag, &nOutFlag);

	if(MCERR_OK != nResult) {
		SetRecordResult(pMcResponse, nResult);
	} else {
		if(0 != nOutFlag && nInFlag != nOutFlag) {
			// Need to store
			return false;
		}

		CAutoPointer<CCacheMemory> pCacheMemory(pOutRecord);
		if(pCacheMemory.IsInvalid()) {
			SetRecordResult(pMcResponse, MCERR_NOTFOUND);
		} else {
			if(bHasCas) {
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
	return true;
}


CCacheServiceImp::CCacheServiceImp(
	const std::string& serverBind,
	const std::string& servantAddress,
	const std::string& serverName,
	uint16_t serverId,
	CreatePlayerMethod createPlayerMethod /*= NULL*/
	)
	: m_serverBind(serverBind)
	, m_servantAddress(servantAddress)
	, m_serverName(serverName)
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
	cacheInterest.add_interests(N_CMD_SEND_TO_CLIENT);
	cacheInterest.add_interests(N_CMD_BROADCAST_TO_CLIENT);
	cacheInterest.add_interests(N_CMD_CLOSE_CLIENT);
	cacheInterest.add_interests(N_CMD_CLOSE_ALLCLIENT);
	cacheInterest.add_interests(N_CMD_SEND_TO_WORKER);
	cacheInterest.add_interests(N_CMD_KICK_LOGGED);
	cacheInterest.add_interests(N_CMD_SERVER_PLAY);
	cacheInterest.add_interests(N_CMD_SERVER_STOP);
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
	::node::DataPacket dataResponse;
	int nOperate = request.cmd();
	switch(nOperate) {
	case N_CMD_GETS:
		HandleGets(request, dataResponse);
		break;
	case N_CMD_CAS:
		HandleCAS(request, dataResponse);
		break;
	case N_CMD_LOAD:
		HandleLoad(request, dataResponse);
		break;
	case N_CMD_STORE:
		HandleStore(request, dataResponse);
		break;
	case N_CMD_ADD:
		HandleAdd(request, dataResponse);
		break;
	case N_CMD_GET:
		HandleGet(request, dataResponse);
		break;
	case N_CMD_SET:
		HandleSet(request, dataResponse);
		break;
	case N_CMD_DEL:
		HandleDel(request, dataResponse);
		break;
	case N_CMD_LOADALL:
		HandleLoadAll(request, dataResponse);
		break;
	case N_CMD_DB_INSERT:
		HandleDBInsert(request, dataResponse);
		break;
	case N_CMD_DB_SELECT:
		HandleDBSelect(request, dataResponse);
		break;
	case N_CMD_DB_UPDATE:
		HandleDBUpdate(request, dataResponse);
		break;
	case N_CMD_DB_DELETE:
		HandleDBDelete(request, dataResponse);
		break;
	case N_CMD_DB_SELECTALL:
		HandleDBSelectAll(request, dataResponse);
		break;
	case N_CMD_DB_ESCAPESTRING:
		HandleDBEscapeString(request, dataResponse);
		break;
	case N_CMD_DB_STOREDPROCEDURES:
		HandleDBStoredProcedures(request, dataResponse);
		break;
	case N_CMD_DB_ASYNCSTOREDPROCEDURES:
		HandleDBAsyncStoredProcedures(request, dataResponse);
		break;
	case N_CMD_SEND_TO_CLIENT:
		HandleSendToClient(request, dataResponse);
		break;
	case N_CMD_BROADCAST_TO_CLIENT:
		HandleBroadcastToClient(request, dataResponse);
		break;
	case N_CMD_CLOSE_CLIENT:
		HandleCloseClient(request, dataResponse);
		break;
	case N_CMD_CLOSE_ALLCLIENT:
		HandleCloseAllClient(request, dataResponse);
		break;
	case N_CMD_SEND_TO_WORKER:
		HandleSendToWorker(request, dataResponse);
		break;
	case N_CMD_KICK_LOGGED:
		HandleKickLogged(request, dataResponse);
		break;
	case N_CMD_SERVER_PLAY:
		HandlePlay(request, dataResponse);
		break;
	case N_CMD_SERVER_STOP:
		HandleStop(request, dataResponse);
		break;
	default:
		HandleDefault(request, dataResponse);
		break;
	}

	if(dataResponse.has_result() || dataResponse.has_data()) {
		dataResponse.set_cmd(request.cmd());
		response.send(dataResponse);
	}
}

void CCacheServiceImp::HandleProtocol(const ::node::DataPacket& request,
	::rpcz::reply< ::node::DataPacket> response)
{
	if(request.route() == ID_NULL) {
		::node::DataPacket dspResponse;
		SendCacheProtocol(request, dspResponse);
		dspResponse.set_cmd(request.cmd());
		dspResponse.set_result(FALSE);
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
			SendCacheProtocol(request, dspResponse);
		} else {
			CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
			CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(request.route()));
			if(pPlayer.IsInvalid()) {
				CAutoPointer<CPlayerBase> pNewPlayer((*m_createPlayerMethod)(request.route()));
				assert(!pNewPlayer.IsInvalid());
				pChlMgr->SetPlayer(request.route(), pNewPlayer);
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
			dspResponse.set_result(FALSE);
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
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

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

        if(pCacheMemMgr->AddCacheRecord(u16DBID, strKey, strValue)) {
            SetRecordResult(pMcRecord, MCERR_OK);
        } else {
            SetRecordResult(pMcRecord, MCERR_NOTSTORED);
        }
    }

	outResponse.set_result(SERVER_SUCCESS);
    SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleLoad(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

    mc_response_t cacheResponse;
    CCacheMemoryManager::PTR_T pCacheMemMgr(CCacheMemoryManager::Pointer());
	std::vector<std::pair<const mc_record_t*, mc_record_t*> > vecLoadCaches;
	std::map<int32_t, ::node::CacheStoreRequest> mapStoreRequests;

    for(int i = 0; i < nSize; ++i)
    {
		mc_record_t* pMcResponse = SetRecord(cacheResponse);
		if(NULL == pMcResponse) {
			OutputError("NULL == pMcRecord");
			continue;
		}
        const mc_record_t& mcRequest = GetRecord(cacheRequest, i);

		const std::string& strKey = GetRecordKey(mcRequest);
		SetRecordKey(pMcResponse, strKey);

		if(IncorrectKeyFormat(strKey, __FUNCTION__)) {
			SetRecordResult(pMcResponse, MCERR_NOREPLY);
			continue;
		}

		int32_t nOutFlag = 0;
		if(!HandleLoadRecord(nOutFlag, pCacheMemMgr,
			u16DBID, m_serverId, mcRequest, pMcResponse))
		{
			// Need to store
			mapStoreRequests[nOutFlag].add_keys(strKey);
			vecLoadCaches.push_back(std::pair<const mc_record_t*,
				mc_record_t*>(&mcRequest, pMcResponse));
			continue;
		}
    }
	// Store the keys
	if(!mapStoreRequests.empty()) {
		CControlCentreStubImpEx::PTR_T pCtrlCentreStub
			= CControlCentreStubImpEx::Pointer();

		std::map<int32_t, ::node::CacheStoreRequest>::
			iterator it(mapStoreRequests.begin());
		for(; mapStoreRequests.end() != it; ++it) {

			::node::CacheStoreRequest& storeRequest = it->second;
			storeRequest.set_serverid(it->first);
			storeRequest.set_routetype(inRequest.route_type());
			storeRequest.set_route(inRequest.route());

			eServerError nResult = pCtrlCentreStub->CacheServerStore(
				m_servantAddress, storeRequest);
			if(SERVER_SUCCESS != nResult) {
				OutputError("pCtrlCentreStub->CacheServerStore()"
					" return %d != SERVER_SUCCESS ", nResult);
			}
		}
	}

	// reload
	if(!vecLoadCaches.empty()) {
		int nCacheSize = (int)vecLoadCaches.size();
		for(int i = 0; i < nCacheSize; ++i) {
			const mc_record_t& mcRequest = *vecLoadCaches[i].first;
			mc_record_t* pMcResponse = vecLoadCaches[i].second;
			int32_t nOutFlag = 0;
			if(!HandleLoadRecord(nOutFlag, pCacheMemMgr,
				u16DBID, m_serverId, mcRequest, pMcResponse))
			{
				OutputError("0 != nOutFlag && nInFlag{%d} != "
					"nOutFlag{%d}", (int)m_serverId, nOutFlag);
				assert(false);
			}
		}
	}

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleStore(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

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
					SetRecordResult(pMcRecord, cacheMemory.UpdateToDB(true, true));
				} else {
					CCacheMemory cacheMemory(u16DBID, strKey);
					cacheMemory.SetValue(tranValue);
					SetRecordResult(pMcRecord, cacheMemory.UpdateToDB(false, true));
				}
            } else {
				if(HasRecordCas(mcRequest)) {
					SetRecordResult(pMcRecord, pCacheMemory->CheckAndSetValue(
						tranValue, GetRecordCas(mcRequest)));
				} else {
					pCacheMemory->SetValue(tranValue);
					SetRecordResult(pMcRecord, MCERR_OK);
				}
				if(!pCacheMemMgr->CheckUpdateAndReloadRecord(pCacheMemory, true)) {
					CCacheMemory::ChangeFlag(u16DBID, strKey, 0);
				}
            }
        } else {
            if(!pCacheMemMgr->RemoveCacheRecord(strKey, true, true)) {
				CCacheMemory::ChangeFlag(u16DBID, strKey, 0);
			}
            SetRecordResult(pMcRecord, MCERR_OK);
        }
    }

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleGet(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

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

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleSet(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

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

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleGets(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

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

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleCAS(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

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
            } else {
                SetRecordResult(pMcRecord, pCacheMemory->CheckAndSetValue(
                    tranValue, GetRecordCas(mcRequest)));
				SetRecordCas(pMcRecord, pCacheMemory->GetCas());
            }
        } else {
            SetRecordResult(pMcRecord, pCacheMemory->CheckAndSetValue(
                tranValue, GetRecordCas(mcRequest)));
			SetRecordCas(pMcRecord, pCacheMemory->GetCas());
        }
    }

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleDel(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

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

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleLoadAll(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	if(!HasRecordKey(cacheRequest) || GetRecordKey(cacheRequest).empty()) {
		outResponse.set_result(CACHE_ERROR_EMPTY_KEY);

		OutputError("!request.has_strkey() || request.strkey().empty()");
		return;
	}

	const std::string& strKey = GetRecordKey(cacheRequest);

	if(EmptyTableFormat(strKey, __FUNCTION__)) {
		outResponse.set_result(CACHE_ERROR_EMPTY_TABLE);
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

	mc_response_t cacheResponse;
	cacheResponse.set_key(strKey);

	uint32_t nOffset = 0;
	if(cacheRequest.has_offset()) {
		nOffset = cacheRequest.offset();
	}
	uint32_t nCount = 0;
	if(cacheRequest.has_count()) {
		nCount = cacheRequest.count();
	}

	cacheResponse.set_result(CCacheMemory::SelectAllFromDB(u16DBID,
		strKey, nOffset, nCount, cacheResponse.mutable_values(), true));

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}


void CCacheServiceImp::HandleDBInsert(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

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

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleDBSelect(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

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

		if(HasRecordCas(mcRequest) && !GetRecordCas(mcRequest)) {
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

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleDBUpdate(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

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

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleDBDelete(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	int nSize = GetRecordSize(cacheRequest);
	if(nSize < 1) {
		outResponse.set_result(CACHE_ERROR_EMPTY_VALUES);
		OutputError("values_size() < 1 ");
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

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

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleDBSelectAll(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	if(!HasRecordKey(cacheRequest) || GetRecordKey(cacheRequest).empty()) {
		outResponse.set_result(CACHE_ERROR_EMPTY_KEY);

		OutputError("!request.has_strkey() || request.strkey().empty()");
		return;
	}

	const std::string& strKey = GetRecordKey(cacheRequest);

	if(EmptyTableFormat(strKey, __FUNCTION__)) {
		outResponse.set_result(CACHE_ERROR_EMPTY_TABLE);
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

	mc_response_t cacheResponse;
	cacheResponse.set_key(strKey);

    uint32_t nOffset = 0;
    if(cacheRequest.has_offset()) {
        nOffset = cacheRequest.offset();
    }
    uint32_t nCount = 0;
    if(cacheRequest.has_count()) {
        nCount = cacheRequest.count();
    }

    cacheResponse.set_result(CCacheMemory::SelectAllFromDB(u16DBID,
		strKey, nOffset, nCount, cacheResponse.mutable_values(), false));

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleDBEscapeString(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
    if(!inRequest.has_data() || inRequest.data().empty()) {
        outResponse.set_result(CACHE_ERROR_EMPTY_DATA);

        OutputError("!inRequest.has_data() || inRequest.data().empty()");
        return;
    }

	uint16_t u16DBId = GetDBID(inRequest.route_type(), inRequest.route());

    CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
    CAutoPointer<Database> pDataBase(pCacheDBMgr->GetDatabase(u16DBId));
    if(pDataBase.IsInvalid()) {
        outResponse.set_result(CACHE_ERROR_DATABASE_INVALID);

        OutputError("pDataBase.IsInvalid()");
        return;
    }

    std::stringstream outString;
    const std::string& strEscape = inRequest.data();
    pDataBase->EscapeLongString(strEscape.c_str(), strEscape.length(), outString);

	outResponse.set_result(SERVER_SUCCESS);
	outResponse.set_data(outString.str());
}

void CCacheServiceImp::HandleDBStoredProcedures(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

    if(!HasRecordKey(cacheRequest) || GetRecordKey(cacheRequest).empty()) {
        outResponse.set_result(CACHE_ERROR_EMPTY_KEY);

        OutputError("!request.has_key() || request.key().empty()");
        return;
    }

    const std::string& strKey = GetRecordKey(cacheRequest);

	if(EmptyTableFormat(strKey, __FUNCTION__)) {
		outResponse.set_result(CACHE_ERROR_EMPTY_TABLE);
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

    mc_response_t cacheResponse;
    cacheResponse.set_key(strKey);

	cacheResponse.set_result(CCacheMemory::StoredProcedures(u16DBID,
		strKey, cacheRequest.data(), cacheResponse.mutable_values()));

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleDBAsyncStoredProcedures(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	mc_request_t cacheRequest;
	if(!ParseCacheData(cacheRequest, inRequest)) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);
		return;
	}

	if(!HasRecordKey(cacheRequest) || GetRecordKey(cacheRequest).empty()) {
		outResponse.set_result(CACHE_ERROR_EMPTY_KEY);

		OutputError("!request.has_key() || request.key().empty()");
		return;
	}

	const std::string& strKey = GetRecordKey(cacheRequest);

	if(EmptyTableFormat(strKey, __FUNCTION__)) {
		outResponse.set_result(CACHE_ERROR_EMPTY_TABLE);
		return;
	}

	uint16_t u16DBID = GetDBID(inRequest.route_type(), inRequest.route());

	mc_response_t cacheResponse;
	cacheResponse.set_key(strKey);

	if(cacheRequest.result() == MCERR_NOREPLY) {
		cacheResponse.set_result(CCacheMemory::AsyncStoredProcedures(
			u16DBID, strKey, cacheRequest.data(), NULL));
	} else {
		cacheResponse.set_result(CCacheMemory::AsyncStoredProcedures(
			u16DBID, strKey, cacheRequest.data(), cacheResponse.mutable_values()));
	}

	outResponse.set_result(SERVER_SUCCESS);
	SerializeCacheData(outResponse, cacheResponse);
}

void CCacheServiceImp::HandleSendToClient(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	if(!inRequest.has_route()) {
		outResponse.set_result(CACHE_ERROR_EMPTY_ROUTE);

		OutputError("!request.has_route()");
		return;
	}

	if(!inRequest.has_data() || inRequest.data().empty()) {
		outResponse.set_result(CACHE_ERROR_EMPTY_DATA);

		OutputError("!request.has_data() || request.data().empty() userId = "I64FMTD, inRequest.route());
		return;
	}

	const std::string& bytes = inRequest.data();
	::node::DataPacket dataPacket;
	if(!dataPacket.ParseFromArray(bytes.data(), bytes.length())) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);

		OutputError("!dataPacket.ParseFromArray userId = "I64FMTD, inRequest.route());
		return;
	}

	eWSResult wsRet = SendWorkerPacketToClient(dataPacket);
	if(WS_SUCCESS == wsRet) {
		outResponse.set_result(SERVER_SUCCESS);
	} else if(WS_NOTFOUND == wsRet) {
		outResponse.set_result(CACHE_ERROR_NOTFOUND);
	} else {
		outResponse.set_result(SERVER_ERROR_UNKNOW);

		OutputError("!SendWorkerToClient userId = "I64FMTD, inRequest.route());
	}
}

void CCacheServiceImp::HandleBroadcastToClient(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	if(!inRequest.has_data() || inRequest.data().empty()) {
		outResponse.set_result(CACHE_ERROR_EMPTY_DATA);

		OutputError("!request.has_data() || request.data().empty()");
		return;
	}

	const std::string& bytes = inRequest.data();
	::node::DataPacket dataPacket;
	if(!dataPacket.ParseFromArray(bytes.data(), bytes.length())) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);

		OutputError("!dataPacket.ParseFromArray");
		return;
	}

	uint64_t excludeId = ID_NULL;
	if(inRequest.has_route()) {
		excludeId = inRequest.route();
	}

	BroadcastWorkerPacketToClient(dataPacket, excludeId);

	outResponse.set_result(SERVER_SUCCESS);
}

void CCacheServiceImp::HandleCloseClient(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	if(!inRequest.has_route()) {
		outResponse.set_result(CACHE_ERROR_EMPTY_ROUTE);

		OutputError("!request.has_route()");
		return;
	}

	eWSResult wsRet = CloseWorkerClient(inRequest.route());
	if(WS_SUCCESS == wsRet) {
		outResponse.set_result(SERVER_SUCCESS);
	} else if(WS_NOTFOUND == wsRet) {
		outResponse.set_result(CACHE_ERROR_NOTFOUND);
	} else {
		outResponse.set_result(SERVER_ERROR_UNKNOW);

		OutputError("!CloseWorkerClient userId = "I64FMTD, inRequest.route());
	}
}

void CCacheServiceImp::HandleCloseAllClient(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	CloseWorkerAllClient();

	outResponse.set_result(SERVER_SUCCESS);
}

void CCacheServiceImp::HandleSendToWorker(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	if(!inRequest.has_route()) {
		outResponse.set_result(CACHE_ERROR_EMPTY_ROUTE);

		OutputError("!request.has_route()");
		return;
	}

	if(!inRequest.has_data() || inRequest.data().empty()) {
		outResponse.set_result(CACHE_ERROR_EMPTY_DATA);

		OutputError("!request.has_data() || request.data().empty() userId = "I64FMTD, inRequest.route());
		return;
	}

	const std::string& bytes = inRequest.data();
	::node::DataPacket dataPacket;
	if(!dataPacket.ParseFromArray(bytes.data(), bytes.length())) {
		outResponse.set_result(CACHE_ERROR_PARSE_REQUEST);

		OutputError("!dataPacket.ParseFromArray userId = "I64FMTD, inRequest.route());
		return;
	}

	eWSResult wsRet = SendWorkerPacketToWorker(dataPacket);
	if(WS_SUCCESS == wsRet) {
		outResponse.set_result(SERVER_SUCCESS);
	} else if(WS_NOTFOUND == wsRet) {
		outResponse.set_result(CACHE_ERROR_NOTFOUND);
	} else {
		outResponse.set_result(SERVER_ERROR_UNKNOW);

		OutputError("!SendWorkerToClient userId = "I64FMTD, inRequest.route());
	}
}

void CCacheServiceImp::HandleKickLogged(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	if(!inRequest.has_route()) {
		outResponse.set_result(CACHE_ERROR_EMPTY_ROUTE);

		OutputError("!request.has_route()");
		return;
	}

	eWSResult wsRet = KickWorkerLogged(inRequest.route());
	if(WS_SUCCESS == wsRet) {
		outResponse.set_result(SERVER_SUCCESS);
	} else if(WS_NOTFOUND == wsRet) {
		outResponse.set_result(CACHE_ERROR_NOTFOUND);
	} else {
		outResponse.set_result(SERVER_ERROR_UNKNOW);

		OutputError("!KickWorkerLogged userId = "I64FMTD, inRequest.route());
	}
}

void CCacheServiceImp::HandlePlay(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	atomic_xchg(&g_serverStatus, SERVER_STATUS_START);
}

void CCacheServiceImp::HandleStop(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	atomic_xchg(&g_serverStatus, SERVER_STATUS_STOP);
}

void CCacheServiceImp::HandleDefault(
	const ::node::DataPacket& inRequest,
	::node::DataPacket& outResponse)
{
	outResponse.set_result(CACHE_ERROR_CMD_UNKNOWN);
}

uint16_t CCacheServiceImp::GetDBID(int32_t nRouteType, uint64_t u64Route)
{
	uint16_t u16DBId = 0;
	if(ROUTE_BALANCE_USERID == nRouteType) {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<CCachePlayer> pCachePlayer(
			pChlMgr->GetPlayer(u64Route));
		if(!pCachePlayer.IsInvalid()) {
			u16DBId = pCachePlayer->GetDBID();
		}
	} else if(ROUTE_BALANCE_SERVERID == nRouteType) {
		uint16_t u16ServId = (uint16_t)u64Route;
		CBalServIDManager::PTR_T pBalServIDMgr(
			CBalServIDManager::Pointer());
		if(!pBalServIDMgr->Find(u16ServId, u16DBId)) {
			CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
			u16DBId = pCacheDBMgr->GetDBIdByBalServId(u16ServId);
			if(0 == u16DBId) {
				u16DBId = pCacheDBMgr->GetMinLoadDBIdByBalServId(u16ServId);
				if(0 != u16DBId) {
					pBalServIDMgr->Add(u16ServId, u16DBId);
				} else {
					OutputError("u16DBId == 0");
					assert(false);
				}
			} else {
				pBalServIDMgr->Add(u16ServId, u16DBId);
			}
		}
	} else if(ROUTE_DIRECT_SERVERID == nRouteType) {
		uint16_t u16ServId = (uint16_t)u64Route;
		CDirServIDManager::PTR_T pDirServIDMgr(
			CDirServIDManager::Pointer());
		if(!pDirServIDMgr->Find(u16ServId, u16DBId)) {
			CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
			u16DBId = pCacheDBMgr->GetDBIdByDirServId(u16ServId);
			if(0 == u16DBId) {
				u16DBId = pCacheDBMgr->GetMinLoadDBIdByDirServId(u16ServId);
				if(0 != u16DBId) {
					pDirServIDMgr->Add(u16ServId, u16DBId);
				} else {
					OutputError("u16DBId == 0");
					assert(false);
				}
			} else {
				pDirServIDMgr->Add(u16ServId, u16DBId);
			}
		}
	}

	return u16DBId;
}





