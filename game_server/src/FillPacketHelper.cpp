/* 
 * File:   FillPacketHelper.cpp
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */
#include "FillPacketHelper.h"

#include "Common.h"
#include "WeakPointer.h"
#include "msg_game_login.pb.h"
#include "Log.h"

#include "PlayerData.h"
#include "msg_node_get_character.pb.h"
#include "PlayerBasic.h"
#include "TimestampManager.h"


#include "mail_packet.pb.h"

#include "PlayerMap.h"



using namespace evt;


 // 填充角色数据到通讯数据包
void FillGetCharacterData(::node::GetCharacterResponse& outCharPacket,
	const CPlayerData& playerData)
{
	outCharPacket.set_cfgid(playerData.GetCfgId());
	outCharPacket.set_name(playerData.GetName());
}

// 填充角色数据到通讯数据包
void FillPlayerData(::game::CharacterBasic* outBasic,
	const util::CAutoPointer<CPlayerData>& playerData)
{
	outBasic->set_account(playerData->GetAccount());
	outBasic->set_cfgid(playerData->GetCfgId());
	outBasic->set_name(playerData->GetName());
	outBasic->set_level(playerData->GetLevel());
	outBasic->set_exp(playerData->GetExp());
	outBasic->set_gem(playerData->GetGem());
	outBasic->set_coin(playerData->GetCoin());
	outBasic->set_curtime(evt::CTimestampManager::Pointer()->GetTimestamp());
    outBasic->set_prologueprocess(playerData->GetPrologueProcess());
}
