/*
 * File:   PlayerModule.cpp
 * Author: Jehu Shaw
 *
 * Created on 2014_7_9, 16:00
 */
#include "PlayerModule.h"
#include "NodeDefines.h"
#include "Log.h"
#include "BodyMessage.h"
#include "ChannelManager.h"
#include "WorkerOperateHelper.h"
#include "CacheOperateHelper.h"
#include "msg_client_login.pb.h"
#include "TimestampManager.h"

#include "PlayerData.h"
#include "msg_game_login.pb.h"
#include "Player.h"
#include "FillPacketHelper.h"

#include "CacheUserManager.h"
#include "LoggingOperateHelper.h"
#include "EnvLockHelper.h"
#include "msg_node_create_character.pb.h"
#include "msg_node_get_character.pb.h"
#include "msg_game_time.pb.h"
#include "PlayerPosData.h"
#include "PlayerBasic.h"
#include "CharacterConfig.h"
#include "GlobalConfig.h"
#include "MapConfig.h"
#include "Utf8.h"
#include "msg_game_map.pb.h"
#include "PlayerMap.h"


using namespace mdl;
using namespace evt;
using namespace util;

CPlayerModule::CPlayerModule(const char* name)
	: CModule(name)
{
	m_pThis(this);
}

CPlayerModule::~CPlayerModule() {
}

void CPlayerModule::OnRegister(){
    OutputBasic("OnRegister");
}

void CPlayerModule::OnRemove(){
    OutputBasic("OnRemove");
}

std::vector<int> CPlayerModule::ListNotificationInterests()
{
	return std::vector<int>({ 
		N_CMD_CHECK_CREATE_CHARACTER,
		N_CMD_CREATE_CHARACTER,
		N_CMD_GET_CHARACTER
	});
}

IModule::InterestList CPlayerModule::ListProtocolInterests()
{
	return InterestList({
		BindMethod<CPlayerModule>(
		P_CMD_C_LOGIN, &CPlayerModule::HandleLogin),

		BindMethod<CPlayerModule>(
		P_CMD_S_LOGOUT, &CPlayerModule::HandleLogout),

		BindMethod<CPlayerModule>(
		P_CMD_C_TIME, &CPlayerModule::HandleTime)
	});
}

void CPlayerModule::HandleNotification(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	int32_t nCmd = request->GetName();
	switch (nCmd) {
	case N_CMD_CHECK_CREATE_CHARACTER:
		CaseCheckCreateCharacter(request, reply);
		break;
	case N_CMD_CREATE_CHARACTER:
		CaseCreateCharacter(request, reply);
		break;
	case N_CMD_GET_CHARACTER:
		CaseGetCharacter(request, reply);
		break;
	default:
		break;
	}
}

void CPlayerModule::CaseCheckCreateCharacter(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if (pRequest.IsInvalid()) {
		return;
	}

	// response
	CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if (pResponse.IsInvalid()) {
		return;
	}

	uint64_t userId = pRequest->route();

	::node::CheckCreateCharRequest checkRequest;
	if (!ParseWorkerData(checkRequest, pRequest)) {
		OutputError("!ParseWorkerData userId = " I64FMTD, userId);
		pResponse->set_result(PARSE_PACKAGE_FAIL);
		return;
	}

	if (CCharacterConfig::Pointer()->GetRow(checkRequest.cfgid()) == NULL) {
		OutputError("!CCharacterConfig::Pointer()->GetRow cfgid = %u", checkRequest.cfgid());
		pResponse->set_result(CONFIG_NOT_FOUND);
		return;
	}

	const std::string& strName = checkRequest.name();
	if (!strName.empty()) {
		wchar_t swzBuf[eBUF_SIZE_512] = { 0 };
		UTF8ToUNICODE((util::custr)strName.data(), strName.length(), swzBuf, eBUF_SIZE_512-1);

        //int nLength = CWordFilter::UnicodeCalcLen(swzBuf, eBUF_SIZE_512);
	
		CGlobalConfig::PTR_T pGlobalConfig = CGlobalConfig::Pointer();
		//if (pGlobalConfig->GetCharacterNameMaxSize() < nLength) {
		//	OutputError("GetCharacterNameMaxSize() < nLength cfgid = %u cfgMaxSize = %u nLength = %u",
		//		checkRequest.cfgid(), pGlobalConfig->GetCharacterNameMaxSize(), nLength);
		//	pResponse->set_result(CHARACTER_NAME_LIMIT);
		//	return;
		//}

        //// Filter invalid character
        //if (!CWordFilter::UnicodeCheckNameValid(swzBuf, eBUF_SIZE_512)) {
        //    pResponse->set_result(CHARACTER_NAME_INVALID);
        //    return;
        //}

		util::CTransferStream strKeys;
		strKeys.Serialize(CHECK_EXIST_ESCAPE_TABLE_NAME, true);
		strKeys.Serialize(strName, true);
		util::CTransferStream outNewKeys;
		MCResult mcResult = McCheckExistEscapeStringBalUserId(userId, strKeys, outNewKeys);
		if(MCERR_NOTFOUND == mcResult) {
			pResponse->set_result(SERVER_SUCCESS);
			std::string strTableKey;
			outNewKeys.Parse(strTableKey);
			std::string strNewName;
			outNewKeys.Parse(strNewName);
			::node::CheckCreateCharResponse checkResponse;
			checkResponse.set_name(strNewName);
			uint32_t nMapId = pGlobalConfig->GetInitMapID();
			checkResponse.set_mapid(nMapId);
			SerializeWorkerData(pResponse, checkResponse);
		} else if(MCERR_OK == mcResult) {
			pResponse->set_result(PLAYER_NAME_ALREADY_EXIST);
		}
		return;
	}
	pResponse->set_result(SERVER_FAILURE);
}

void CPlayerModule::CaseCreateCharacter(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if (pRequest.IsInvalid()) {
		return;
	}

	// response
	CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if (pResponse.IsInvalid()) {
		return;
	}

	uint64_t userId = pRequest->route();

	::node::CreateCharacterRequest createRequest;
	if (!ParseWorkerData(createRequest, pRequest)) {
		OutputError("!ParseWorkerData userId = " I64FMTD, userId);
		pResponse->set_result(PARSE_PACKAGE_FAIL);
		return;
	}

	if (CCharacterConfig::Pointer()->GetRow(createRequest.cfgid()) == NULL) {
		OutputError("!CCharacterConfig::Pointer()->GetRow cfgid = %u", createRequest.cfgid());
		pResponse->set_result(CONFIG_NOT_FOUND);
		return;
	}

	// 调用创建角色接口
	int nResult = CPlayer::OnCreate(userId, createRequest);
	if (SERVER_SUCCESS != nResult) {
		OutputError("CPlayer::OnCreate nResult = %d userId = " I64FMTD, nResult, userId);
	}

	pResponse->set_result(nResult);
}

void CPlayerModule::CaseGetCharacter(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if (pRequest.IsInvalid()) {
		return;
	}

	// response
	CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if (pResponse.IsInvalid()) {
		return;
	}

	uint64_t userId = pRequest->route();

	::node::GetCharacterResponse getResponse;
	int nResult = CPlayer::OnCharacterInfo(getResponse, userId);

	// send to client
	pResponse->set_result(nResult);
	SerializeWorkerData(pResponse, getResponse);
}


class CLoginMessage : public thd::CWriteData<CPlayer> {
public:
	CLoginMessage(CWeakPointer<::node::DataPacket>& pRequest, CWeakPointer<::node::DataPacket>& pResponse)
		: m_pRequest(pRequest), m_pResponse(pResponse) {}
private:
	CWeakPointer<::node::DataPacket> m_pRequest;
	// response
	CWeakPointer<::node::DataPacket> m_pResponse;
private:
	virtual void Process(util::CWeakPointer<CPlayer> pPlayer) final {
		if (pPlayer.IsInvalid()) {
			OutputError("pPlayer.IsInvalid() userId = " I64FMTD, m_pRequest->route());
			return;
		}

		uint64_t userId = pPlayer->GetUserID();
		int nSubCmd = m_pRequest->sub_cmd();
		if (LOGIN_RECOVER != nSubCmd) {
			::node::LoginRequest loginRequest;
			if (!ParseWorkerData(loginRequest, m_pRequest)) {
				OutputError("!ParseWorkerData userId = " I64FMTD, userId);
				m_pResponse->set_result(PARSE_PACKAGE_FAIL);
				return;
			}

			//   CConfigTemplate::PTR_T pConfigTemp(CConfigTemplate::Pointer());

			   //if(pRequest->result() != LOGIN_SWITCH_MAP) {
			   //	uint32_t nSerVersion = (uint32_t)pConfigTemp->GetValue(CONFIG_VERSION);
			   //	if(loginRequest.version() != nSerVersion) {
			   //		OutputError("CONFIG_VERSION_NOT_CONSISTENT userId = "
			   //			I64FMTD " clientVersion = %d serverVersion = %d", userId
			   //			, loginRequest.version(), nSerVersion);
			   //		pResponse->set_result(CONFIG_VERSION_NOT_CONSISTENT);
			   //		return;
			   //	}
			   //}

			// Set map id;
			pPlayer->SetMapID(loginRequest.mapid());

			CAutoPointer<CPlayerBasic> pPlayerBasic(pPlayer->GetUnit(PLAYER_UNIT_BASIC));
			CAutoPointer<CPlayerData> pPlayerData(pPlayerBasic->GetPlayerData());
			util::CValueStream strKeys;
			strKeys.Serialize(userId);
			MCResult nResult = pPlayerData->LoadFromCache(userId, strKeys);
			if (MCERR_OK != nResult) {
				OutputError("MCERR_OK != pPlayerData->LoadFromCache nResult = %d"
					" userId = " I64FMTD, nResult, userId);
				// response
				m_pResponse->set_result(SERVER_FAILURE);
				return;
			}
			//OutputDebug("3 CPlayerModule::HandleLogin load player data end userId = " I64FMTD, userId);

			if (LOGIN_SWITCH_MAP != nSubCmd) {
				if (pPlayerData->GetAccount() != loginRequest.account()) {
					OutputError("pPlayerData->GetAccount() = " I64FMTD " loginRequest.account() = " I64FMTD
						" userId = " I64FMTD, pPlayerData->GetAccount(), loginRequest.account(), userId);
					m_pResponse->set_result(SERVER_FAILURE);
					return;
				}
			}
			// 调用角色登录接口
			pPlayer->OnLogin();
		}

		//OutputDebug("4 CPlayerModule::HandleLogin load player data end userId = " I64FMTD, userId);
		if (LOGIN_SWITCH_MAP != nSubCmd) {
			game::LoginResponse loginResponse;
			// 获取需要下发客户端的数据
			pPlayer->OnInitClient(loginResponse);

			//OutputDebug("5 CPlayerModule::HandleLogin userId = " I64FMTD, userId);
			SerializeWorkerData(m_pResponse, loginResponse);
		}
		else {
			CAutoPointer<CPlayerMap> pPlayerMap(pPlayer->GetUnit(PLAYER_UNIT_MAP));

			::game::SwitchMapResponse switchResponse;
			pPlayerMap->OnSwitchMapLogin(switchResponse);

			SerializeWorkerData(m_pResponse, switchResponse);
		}

		m_pResponse->set_result(SERVER_SUCCESS);
		//////////////////////////////////////////////////////////////////////////

		//TraceBehavior(userId, pPlayerData->GetAccount(), pPlayerData->GetName(),
		//	LOG_BEHAVIOR_LOGIN, pPlayerData->GetLevel());
	}
};

void CPlayerModule::HandleLogin(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if(pRequest.IsInvalid()) {
		return;
	}

    // response
    CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
    if(pResponse.IsInvalid()) {
        return;
    }

    uint64_t userId = pRequest->route();

	//OutputDebug("1 CPlayerModule::HandleLogin userId = " I64FMTD, userId);
	CWeakPointer<CWrapPlayer> pWrapPlayer(GetWorkerPlayer(request));
	if (pWrapPlayer.IsInvalid()) {
		pResponse->set_result(CANNT_FIND_PLAYER);
		return;
	}

	util::CUniquePointer<CLoginMessage> msg(new CLoginMessage(pRequest, pResponse));
	pWrapPlayer->Send(msg, false);
}


class CLogoutMessage : public thd::CWriteData<CPlayer> {
private:
	virtual void Process(util::CWeakPointer<CPlayer> pPlayer) final {
		if (pPlayer.IsInvalid()) {
			OutputError("pPlayer.IsInvalid() ");
			return;
		}
		// 退出做一些处理
		pPlayer->OnLogout();

		uint64_t userId = pPlayer->GetUserID();
		//////////////////////////////////////////////////////////////////////////
		// 退出自动保存数据
		CCacheUserManager::PTR_T pCacheRecordMgr(CCacheUserManager::Pointer());
		pCacheRecordMgr->RemoveCacheRecord(userId);
		/////////////////////////////////////////////////////////////////////////
		CAutoPointer<CPlayerBasic> pPlayerBasic(pPlayer->GetUnit(PLAYER_UNIT_BASIC));
		CAutoPointer<CPlayerData> pPlayerData(pPlayerBasic->GetPlayerData());
		TraceBehavior(userId, pPlayerData->GetAccount(), pPlayerData->GetName(),
			LOG_BEHAVIOR_LOGOUT, pPlayerData->GetLevel());
	}
};

void CPlayerModule::HandleLogout(const CWeakPointer<mdl::INotification>& request,
    CWeakPointer<mdl::IResponse>& reply)
{
    CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
    if(pRequest.IsInvalid()) {
        return;
    }

    CWeakPointer<CWrapPlayer> pWrapPlayer(GetWorkerPlayerSilence(request));
    if(pWrapPlayer.IsInvalid()) {
        return;
    }

	util::CUniquePointer<CLogoutMessage> msg(new CLogoutMessage);
	pWrapPlayer->Post(msg);
}

void CPlayerModule::HandleTime(const CWeakPointer<mdl::INotification>& request,
	CWeakPointer<mdl::IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
	if (pResponse.IsInvalid()) {
		return;
	}

	pResponse->set_result(SERVER_SUCCESS);
	
	::game::TimeResponse timeResponse;
	timeResponse.set_time(CTimestampManager::Pointer()->GetTimestamp());
	SerializeWorkerData(pResponse, timeResponse);
}