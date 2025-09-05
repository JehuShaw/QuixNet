#include "PlayerMap.h"
#include "Player.h"
#include "CacheOperateHelper.h"
#include "WorkerOperateHelper.h"
#include "NodeDefines.h"
#include "Log.h"
#include "CacheUserManager.h"
#include "msg_game_login.pb.h"
#include "msg_game_map.pb.h"
// #include "WorldManager.h"
#include "FillPacketHelper.h"
#include "PlayerMapData.h"
#include "PlayerBasic.h"
#include "msg_node_create_character.pb.h"



using namespace util;

CPlayerMap::CPlayerMap(const util::CWeakPointer<CPlayer>& pPlayer)
    : m_pPlayer(pPlayer)
{
	m_pThis(this);
	m_pPlayerPosData.SetRawPointer(new CPlayerPosData);
	m_pPlayerMapData.SetRawPointer(new CPlayerMapData);
}

CPlayerMap::~CPlayerMap(void)
{
}

int CPlayerMap::OnCreate(uint64_t userId, const ::node::CreateCharacterRequest & req)
{


	return SERVER_SUCCESS;
}

int CPlayerMap::OnDispose(uint64_t userId)
{

	return SERVER_SUCCESS;
}

int CPlayerMap::OnCharacterInfo(::node::GetCharacterResponse& outResponse, uint64_t userId)
{
	return SERVER_SUCCESS;
}

void CPlayerMap::OnLogin() {
	
}

void CPlayerMap::OnLogout() {

}

void CPlayerMap::OnInitClient(game::LoginResponse& outResponse)
{

	
}

void CPlayerMap::OnSwitchMapLogin(::game::SwitchMapResponse& switchResponse)
{
	switchResponse.set_mapid(m_pPlayer->GetMapID());
	FillVectorData(switchResponse.mutable_pos()
		, m_pPlayerPosData->GetX()
		, m_pPlayerPosData->GetY()
		, m_pPlayerPosData->GetZ());
	FillVectorData(switchResponse.mutable_face()
		, m_pPlayerPosData->GetFaceX()
		, m_pPlayerPosData->GetFaceY()
		, m_pPlayerPosData->GetFaceZ());
}
