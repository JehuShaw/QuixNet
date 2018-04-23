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

    // 设置玩家角色经验，伴随升级操作
    int AddExpAndUpgrade(int32_t nExp, bool& outUpgrade);

    bool AddCoin(int32_t nCoin);

    bool AddGem(int32_t nGem);

    // 判断玩家属性值是否充足
    bool CheckAttribPoint(ePlayerAttribType attribType, int32_t nNeed);
    // 消耗玩家属性值
    bool ConsumeAttribPoint(ePlayerAttribType attribType, int32_t nNeed);
    // 增加或减少属性值
    bool AddAttribPoint(ePlayerAttribType attribType, int32_t nValue);
    /////////////////////////////////////////////////////////////////////////
    // 判断是否可以重置, 在配置的时间点一天内不会重置,隔天就会判断需要重置
    static bool IsResetTime(const std::string& strLastTime, uint16_t nConfigTime);
	// 玩家数据
	util::CAutoPointer<CPlayerData> GetPlayerData() {
		return m_pPlayerData;     
	}

private:
	uint64_t m_userId;
    util::CAutoPointer<CPlayer> m_pThis;
    util::CAutoPointer<CPlayerData> m_pPlayerData;

};

#endif /* _PLAYER_H_ */
