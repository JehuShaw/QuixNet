/* 
 * File:   Player.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef _PLAYER_H_
#define	_PLAYER_H_

#include "ChannelManager.h"
#include "PlayerMutex.h"
#include "PoolBase.h"
#include "PlayerData.h"


enum MCResult;

enum ePlayerAttribType {
    PLAYER_ATTRIB_NIL,
    PLAYER_ATTRIB_COIN,
    PLAYER_ATTRIB_GEM,
};

class CPlayer : public CPlayerMutex, public util::PoolBase<CPlayer>
{
public:
	CPlayer(uint64_t userId);
	~CPlayer(void);

	virtual uint64_t GetUserId() const {
		return m_userId; 
	}

    void AddToRecordManager();

    // ������ҽ�ɫ���飬������������
    int AddExpAndUpgrade(int32_t nExp, bool& outUpgrade);

    bool AddCoin(int32_t nCoin);

    bool AddGem(int32_t nGem);

    // �ж��������ֵ�Ƿ����
    bool CheckAttribPoint(ePlayerAttribType attribType, int32_t nNeed);
    // �����������ֵ
    bool ConsumeAttribPoint(ePlayerAttribType attribType, int32_t nNeed);
    // ���ӻ��������ֵ
    bool AddAttribPoint(ePlayerAttribType attribType, int32_t nValue);
    /////////////////////////////////////////////////////////////////////////
    // �ж��Ƿ��������, �����õ�ʱ���һ���ڲ�������,����ͻ��ж���Ҫ����
    static bool IsResetTime(const std::string& strLastTime, uint16_t nConfigTime);
	// �������
	util::CAutoPointer<CPlayerData> GetPlayerData() {
		return m_pPlayerData;     
	}

private:
	uint64_t m_userId;
    util::CAutoPointer<CPlayer> m_pThis;
    util::CAutoPointer<CPlayerData> m_pPlayerData;

};

#endif /* _PLAYER_H_ */
