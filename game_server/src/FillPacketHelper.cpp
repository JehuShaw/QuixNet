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
#include "msg_game_player_addexp.pb.h"
#include "Log.h"

#include "PlayerData.h"
#include "ConfigTemplate.h"

// 填充角色数据到通讯数据包
void FillPlayerData(::game::CharacterPacket& outCharPacket,
    const CPlayerData& playerData)
{
    outCharPacket.set_account(playerData.GetAccount());
    outCharPacket.set_name(playerData.GetName());
    outCharPacket.set_level(playerData.GetLevel());
    outCharPacket.set_exp(playerData.GetExp());
    outCharPacket.set_gem(playerData.GetGem());
    outCharPacket.set_coin(playerData.GetCoin());
}

// 玩家经验值改变
void FillPlayerAddexp(::game::PlayerAddexpPacket& playerAddexp,
    bool bPlayerUpgrade, const CPlayerData& playerData) {
    playerAddexp.set_exp(playerData.GetExp());
    if(bPlayerUpgrade) {
        ::game::PlayerUpgradeChangePacket* pPlayerUpgradeChange 
            = playerAddexp.mutable_upgrade_change();
        pPlayerUpgradeChange->set_level(playerData.GetLevel());
        //pPlayerUpgradeChange->set_phy_power(playerData.GetPhyPower());
        //pPlayerUpgradeChange->set_phy_power_limit(playerData.GetPhyPowerLimit());
        //pPlayerUpgradeChange->set_general_lv_limit(playerData.GetGeneralLvLimit());
    }
}




