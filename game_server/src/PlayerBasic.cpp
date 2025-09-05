#include "PlayerBasic.h"
#include "Player.h"
#include "CacheOperateHelper.h"
#include "WorkerOperateHelper.h"
#include "NodeDefines.h"
#include "Log.h"
#include "CacheUserManager.h"
#include "TimestampManager.h"
#include "msg_game_login.pb.h"
#include "PlayerData.h"
#include "LevelExpConfig.h"
#include "FillPacketHelper.h"
#include "CharacterConfig.h"
#include "msg_node_create_character.pb.h"
#include "msg_node_get_character.pb.h"
#include "msg_game_charater.pb.h"
#include "GameEventManager.h"

using namespace util;
using namespace evt;

#define IF_EXEC(cond, ret) do { if (cond) { ret; } } while (0) 

CPlayerBasic::CPlayerBasic(const util::CWeakPointer<CPlayer>& pPlayer)
    : m_pPlayer(pPlayer)
{
    m_pThis(this);
    m_pPlayerData.SetRawPointer(new CPlayerData);
}

CPlayerBasic::~CPlayerBasic(void)
{
}

int CPlayerBasic::OnCreate(uint64_t userId, const ::node::CreateCharacterRequest & req)
{
	CPlayerData playerData;
	std::string strTime(CTimestampManager::Pointer()->GetLocalDateTimeStr());
	if (req.name().empty()) {
		playerData.SetName(std::string("anonymous"));
	} else {
		std::string strName;
		McDBEscapeString(userId, req.name(), strName);
		playerData.SetName(strName);
	}
	playerData.SetAccount(req.account());
	playerData.SetCfgId(req.cfgid());
	playerData.SetCreateTime(strTime);
	playerData.SetLevel(1);
    playerData.SetPrologueProcess(PRO_PROCESS_TYPE_INIT);

	// 创建角色基础数据
	util::CValueStream strKeys;
	strKeys.Serialize(userId);
	MCResult mcRet = playerData.AddToCache(userId, strKeys);
	if (MCERR_OK != mcRet) {
		OutputError("MCERR_OK != pPlayerData.AddToCache()"
			" mcRet = %d userId = " I64FMTD, mcRet, userId);
		return SERVER_FAILURE;
	}
  
	return SERVER_SUCCESS;
}

int CPlayerBasic::OnDispose(uint64_t userId)
{
	util::CValueStream strKeys;
	strKeys.Serialize(userId);
	CPlayerData playerData;
	MCResult mcRet = playerData.DelFromCache(userId, strKeys);
	if (MCERR_OK != mcRet) {
		OutputError("MCERR_OK != pPlayerData.DelFromCache(), mcRet = %d userId = " I64FMTD, mcRet, userId);
		//return SERVER_FAILURE;
	}

	return SERVER_SUCCESS;
}

int CPlayerBasic::OnCharacterInfo(::node::GetCharacterResponse& outResponse, uint64_t userId)
{
	CPlayerData playerData;
	util::CValueStream strKeys;
	strKeys.Serialize(userId);
	MCResult nResult = playerData.LoadFromCache(userId, strKeys);
	if (MCERR_NOTFOUND == nResult) {
		return SERVER_ERROR_NOTFOUND_CHARACTER;
	} else if (MCERR_OK != nResult) {
		OutputError("MCERR_OK != pPlayerData->LoadFromCache"
			" nResult = %d userId = " I64FMTD, nResult, userId);
		// response
		return SERVER_FAILURE;
	}

	FillGetCharacterData(outResponse, playerData);

	return SERVER_SUCCESS;
}

void CPlayerBasic::OnLogin() {
	uint64_t userId = m_pPlayer->GetUserID();
	util::CValueStream strKeys;
	strKeys.Serialize(userId);
	CCacheUserManager::PTR_T pCacheRecordMgr(CCacheUserManager::Pointer());
	eAddRecordResult arResult = pCacheRecordMgr->AddCacheRecord(
		userId, strKeys, m_pPlayerData, false);
	if (ADD_RECORD_FAIL == arResult) {
		OutputError("!AddCacheRecord(m_pPlayerData) userId = " I64FMTD, userId);
	}
	// 设置角色在线
	m_pPlayerData->SetStatus(PLAYER_STATUS_ONLINE);

    CCharacterConfig::PTR_T characterConfig(CCharacterConfig::Pointer());
    m_pCharacterRow = characterConfig->GetRow(m_pPlayerData->GetCfgId());

}

void CPlayerBasic::OnLogout() {
	std::string strTime(CTimestampManager::Pointer()->GetLocalDateTimeStr());
	m_pPlayerData->SetOfflineTime(strTime);
	// 设置角色离线
	m_pPlayerData->SetStatus(PLAYER_STATUS_OFFLINE);

}

void CPlayerBasic::OnInitClient(game::LoginResponse& outResponse) 
{
    // PlayerData
	FillPlayerData(outResponse.mutable_basic(), m_pPlayerData);

}

int CPlayerBasic::AddExpAndUpgrade(int32_t nExp, int& outUpgradeCount)
{
	outUpgradeCount = 0;
    if(0 == nExp) {
        return SERVER_FAILURE;
    }

    int nAddExp = m_pPlayerData->GetExp() + nExp;
    if(nAddExp < 0) {
        nAddExp = 0;
    }
    m_pPlayerData->SetExp(nAddExp);

	CLevelExpConfig::PTR_T pCharLvTemp(CLevelExpConfig::Pointer());
    for(;;) {
        const LevelExpRow* pLvExpRow = pCharLvTemp->GetRow(
            m_pPlayerData->GetLevel()/* + 1*/);
        if(NULL == pLvExpRow) {
            break;
        }
        int nExp = m_pPlayerData->GetExp() - pLvExpRow->nExp;
        if(nExp < 0) {
            break;
        }
        m_pPlayerData->SetLevel(m_pPlayerData->GetLevel() + 1);
        m_pPlayerData->SetExp(nExp);
		++outUpgradeCount;
    }
	// trigger upgrade event
	if(outUpgradeCount > 0) {
		CArgumentBitStream arg;
		arg.WriteUInt64(m_pPlayer->GetUserID());
		arg.WriteInt32(m_pPlayerData->GetExp());
		arg.WriteInt32(m_pPlayerData->GetLevel());
		arg.WriteInt32(nExp);
		arg.WriteInt32(outUpgradeCount);
		m_pPlayer->DispatchEvent(GEVENT_PLAYER_UPGRADE, arg);
	}
    return SERVER_SUCCESS;
}

bool CPlayerBasic::AddCoin(int32_t nCoin)
{
    if(0 == nCoin) {
        return false;
    }
    int nDifCoin = m_pPlayerData->GetCoin() + nCoin;
    if(nDifCoin < 0) {
        nDifCoin = 0;
    }
    m_pPlayerData->SetCoin(nDifCoin);
    return true;
}

bool CPlayerBasic::AddGem(int32_t nGem)
{
    if(0 == nGem) {
        return false;
    }
    int nDifGem = m_pPlayerData->GetGem() + nGem;
    if(nDifGem < 0) {
        nDifGem = 0;
    }
    m_pPlayerData->SetGem(nDifGem);
    return true;
}

bool CPlayerBasic::CheckAttribPoint(ePlayerAttribType attribType, int32_t nNeed)
{
    switch(attribType) {
    case PLAYER_ATTRIB_GEM:
        return m_pPlayerData->GetGem() >= nNeed;
    case PLAYER_ATTRIB_COIN:
        return m_pPlayerData->GetCoin() >= nNeed;
    default:
        break;
    }
    return false;
}

bool CPlayerBasic::ConsumeAttribPoint(ePlayerAttribType attribType, int32_t nNeed)
{
    if(nNeed == 0) {
        return false;
    }

    if(nNeed > 0) {
        nNeed = -nNeed;
    }

    switch(attribType) {
    case PLAYER_ATTRIB_GEM:
        return AddGem(nNeed);
        break;
    case PLAYER_ATTRIB_COIN:
        return AddCoin(nNeed);
        break;
    default:
        break;
    }
    return true;
}

bool CPlayerBasic::AddAttribPoint(ePlayerAttribType attribType, int32_t nValue)
{
    if(nValue == 0) {
        return false;
    }

    switch(attribType) {
    case PLAYER_ATTRIB_GEM:
        return AddGem(nValue);
        break;
    case PLAYER_ATTRIB_COIN:
        return AddCoin(nValue);
        break;
    default:
        break;
    }
    return true;
}

bool CPlayerBasic::IsResetTime(const std::string& strLastTime, uint16_t nConfigTime)
{
    CTimestampManager::PTR_T pTsMgr(CTimestampManager::Pointer());
    struct tm lastTM = {0,0,0,0,0,0,0,0,0};
    if(!pTsMgr->StringToTM(strLastTime.c_str(), &lastTM)) {
        return true;
    }
    time_t lastTimestamp = pTsMgr->TMtoTimestamp(&lastTM);

    lastTM.tm_hour = ((int)((nConfigTime / 10000) % 100) % 24); 
    lastTM.tm_min = ((int)((nConfigTime / 100) % 100) % 60); 
    lastTM.tm_sec = (int)(nConfigTime % 100) % 60;
    time_t lastEndTime = pTsMgr->TMtoTimestamp(&lastTM);

    if(lastTimestamp < lastEndTime) {
        if(pTsMgr->GetTimestamp() >= lastEndTime) {
            return true;
        }
    } else {
        if(pTsMgr->GetTimestamp() >= (lastEndTime + 86400)) {
            return true;
        }
    }
    return false;
}


