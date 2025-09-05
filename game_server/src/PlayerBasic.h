/* 
 * File:   PlayerBasic.h
 * Author: Jehu Shaw
 * 
 * Created on 2020_5_12, 17:10
 */

#ifndef PLAYERBASIC_H
#define	PLAYERBASIC_H

#include "IPlayerUnit.h"
#include "WeakPointer.h"
#include "NodeDefines.h"
#include "PlayerData.h"
#include "CharacterConfig.h"
#include "ObjectSet.h"
#include "CommandManager.h"
#include "ReferObject.h"

class CPlayer;
//class CTitleItem;
class CPlayerTitleData;
class CPlayerHeadData;
class CPlayerHeadFrameData;
//class CHeadStateData;
//class CHeadFrameStateData;
template<class ID_TYPE, class T_OBJ>
class CGeneralState;

enum MCResult;

namespace node {
	class CreateCharacterRequest;
	class GetCharacterResponse;
}

enum ePlayerAttribType {
	PLAYER_ATTRIB_NIL,
	PLAYER_ATTRIB_COIN,
	PLAYER_ATTRIB_GEM,
};


class CPlayerBasic : public IPlayerUnit
{
public:
	CPlayerBasic(const util::CWeakPointer<CPlayer>& pPlayer);

	~CPlayerBasic(void);

	// 角色创建的时候触发
	static int OnCreate(uint64_t userId, const ::node::CreateCharacterRequest & req);
	// Dispose character callback
	static int OnDispose(uint64_t userId);
	// 登录的时候获得角色信息（这时候角色对象还没创建）
	static int OnCharacterInfo(::node::GetCharacterResponse& outResponse, uint64_t userId);
	// 角色登录的时候触发
	virtual void OnLogin();
	// 玩家离线的时候触发
	virtual void OnLogout();
	// 玩家登录成功后向客户端返回的数据
	virtual void OnInitClient(game::LoginResponse& outResponse);

    // 设置玩家角色经验，伴随升级操作, outUpgradeCount 返回升了几级，如果没有升级返回0
    int AddExpAndUpgrade(int32_t nExp, int& outUpgradeCount);

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
	util::CAutoPointer<CPlayerData> GetPlayerData() const {
		return m_pPlayerData;     
	}
	// 获得玩家角色
	inline int32_t GetPlayerLevel() const {
		return m_pPlayerData->GetLevel();
	}
    //　获取角色经验
    inline int32_t GetPlayerExp() const {
        return m_pPlayerData->GetExp();
    }
	// 角色配置表ID
	inline uint32_t GetCfgID() const {
		return m_pPlayerData->GetCfgId();
	}
	// 角色名
	inline std::string GetName() const {
		return m_pPlayerData->GetName();
	}
    // 角色性别
    inline int32_t GetGender() const {
        return (NULL != m_pCharacterRow) ? (m_pCharacterRow->nGender) : (GENDER_TYPE_DEFAULT);
    }

	int32_t CalcPlayerFC() const;


private:
    util::CReferObject<CPlayerBasic> m_pThis;
	util::CWeakPointer<CPlayer> m_pPlayer;
	util::CAutoPointer<CPlayerData> m_pPlayerData;
    const CharacterRow * m_pCharacterRow;
};

#endif /* _PLAYERBASIC_H_ */
