#include "Player.h"
#include "CacheOperateHelper.h"
#include "WorkerOperateHelper.h"
#include "NodeDefines.h"
#include "Log.h"
#include "CacheRecordManager.h"

#include "CharacterLvTemplate.h"
#include "TimestampManager.h"

using namespace util;
using namespace evt;

CPlayer::CPlayer(uint64_t userId) 
    : m_userId(userId)
{
    m_pThis.SetRawPointer(this, false);
    m_pPlayerData.SetRawPointer(new CPlayerData);
}

CPlayer::~CPlayer(void)
{
}

void CPlayer::AddToRecordManager() {
	util::CValueStream strKeys;
	strKeys.Serialize(GetUserId());
    CCacheRecordManager::PTR_T pCacheRecordMgr(CCacheRecordManager::Pointer());
	eAddRecordResult arResult = pCacheRecordMgr->AddCacheRecord(
		GetUserId(), strKeys, m_pPlayerData, false);
    if(arResult == ADD_RECORD_FAIL) {
        OutputError("!AddCacheRecord(m_pPlayerData) userId = "I64FMTD, GetUserId());
    }
}

int CPlayer::AddExpAndUpgrade(int32_t nExp, bool& outUpgrade)
{
    outUpgrade = false;
    if(0 == nExp) {
        return FALSE;
    }

    int nAddExp = m_pPlayerData->GetExp() + nExp;
    if(nAddExp < 0) {
        nAddExp = 0;
    }
    m_pPlayerData->SetExp(nAddExp);

    CCharacterLvTemplate::PTR_T pCharLvTemp(CCharacterLvTemplate::Pointer());
    for(;;) {
        const CharacterLvRow* pCharacterLvRow = pCharLvTemp->GetRow(
            m_pPlayerData->GetLevel() + 1);
        if(NULL == pCharacterLvRow) {
            break;
        }
        int nExp = m_pPlayerData->GetExp() - pCharacterLvRow->nExp;
        if(nExp < 0) {
            break;
        }
        m_pPlayerData->SetLevel(m_pPlayerData->GetLevel() + 1);
        m_pPlayerData->SetExp(nExp);
        outUpgrade = true;
    }
    return TRUE;
}

bool CPlayer::AddCoin(int32_t nCoin)
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

bool CPlayer::AddGem(int32_t nGem)
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

bool CPlayer::CheckAttribPoint(ePlayerAttribType attribType, int32_t nNeed)
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

bool CPlayer::ConsumeAttribPoint(ePlayerAttribType attribType, int32_t nNeed)
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

bool CPlayer::AddAttribPoint(ePlayerAttribType attribType, int32_t nValue)
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

bool CPlayer::IsResetTime(const std::string& strLastTime, uint16_t nConfigTime)
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

















