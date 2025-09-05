/* 
 * File:   PlayerMap.h
 * Author: Jehu Shaw
 * 
 * Created on 2020_5_12, 17:10
 */

#ifndef PLAYERMAP_H
#define	PLAYERMAP_H

#include "IPlayerUnit.h"
#include "WeakPointer.h"
#include "PlayerPosData.h"
#include "ReferObject.h"
#include "AgentMethod.h"
#include "ObjectSet.h"
#include "PlayerMapData.h"

class CPlayer;

namespace node {
	class CreateCharacterRequest;
	class GetCharacterResponse;
}

typedef std::set<uint32_t> PLAYER_MAP_SET_T;

namespace game {
	class SwitchMapResponse;
}

class CPlayerMap : public IPlayerUnit
{
public:
	CPlayerMap(const util::CWeakPointer<CPlayer>& pPlayer);

	~CPlayerMap(void);

	// Create character callback
    static int OnCreate(uint64_t userId, const ::node::CreateCharacterRequest & req);
	// Dispose character callback
	static int OnDispose(uint64_t userId);
	// 登录的时候获得角色信息（这时候角色对象还没创建）
	static int OnCharacterInfo(::node::GetCharacterResponse& outResponse, uint64_t userId);
	// 
	virtual void OnLogin();

	virtual void OnLogout();

	virtual void OnInitClient(game::LoginResponse& outResponse);

	// 玩家位置数据
	util::CAutoPointer<CPlayerPosData> GetPlayerPosData() {
		return m_pPlayerPosData;
	}

	util::CAutoPointer<const CPlayerPosData> GetPlayerPosData() const {
		return m_pPlayerPosData;
	}

	inline float GetX() const {
		return m_pPlayerPosData->GetX();
	}

	inline float GetY() const {
		return m_pPlayerPosData->GetY();
	}

	inline float GetZ() const {
		return m_pPlayerPosData->GetZ();
	}

	inline float GetFaceX() const {
		return m_pPlayerPosData->GetFaceX();
	}

	inline float GetFaceY() const {
		return m_pPlayerPosData->GetFaceY();
	}

	inline float GetFaceZ() const {
		return m_pPlayerPosData->GetFaceZ();
	}

	// 是否允许进入该地图（解锁）
	inline bool EnableMap(uint32_t nMapId) const {
		return m_pPlayerMapData->HadMap(nMapId);
	}

	inline static bool EnableMap(const CPlayerMapData& playerMapData, uint32_t nMapId) {
		return playerMapData.HadMap(nMapId);
	}

	void OnSwitchMapLogin(::game::SwitchMapResponse& switchResponse);

private:
	util::CWeakPointer<CPlayer> m_pPlayer;
	util::CReferObject<CPlayerMap> m_pThis;
	util::CAutoPointer<CPlayerPosData> m_pPlayerPosData;

	util::CAutoPointer<CPlayerMapData> m_pPlayerMapData;
};

#endif /* PLAYERMAP_H */
