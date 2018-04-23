/* 
 * File:   FillPacketHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef __FILLPACKETHELPER_H__
#define __FILLPACKETHELPER_H__

#include "Common.h"
#include "WeakPointer.h"

namespace game 
{
    class CharacterPacket;
    class PlayerAddexpPacket;
}

namespace google {
    namespace protobuf {
        template <typename Element>
        class RepeatedPtrField;
    }
}

class CPlayerData;

// 填充角色数据到通讯数据包
extern void FillPlayerData(::game::CharacterPacket& outCharPacket,
    const CPlayerData& playerData);

// 玩家经验值改变
extern void FillPlayerAddexp(::game::PlayerAddexpPacket& playerAddexp,
    bool bPlayerUpgrade, const CPlayerData& playerData);


#endif /* __FILLPACKETHELPER_H__ */
