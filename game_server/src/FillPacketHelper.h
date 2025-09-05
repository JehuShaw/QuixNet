/* 
 * File:   FillPacketHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef FILLPACKETHELPER_H
#define FILLPACKETHELPER_H

#include "Common.h"
#include "WeakPointer.h"

#include "common_packet.pb.h"
#include "ValueStream.h"

namespace game 
{
    class CharacterBasic;
    class PlayerAddexpPacket;
	class ItemPacket;
	class RewardPacket;
	class QuestPacket;
	class ConsumeItemPacket;
	class PetPacket;
	class UpgradePetResponse;
	class PVEBeginResponse;
	class MonsterPacket;
	class EquipPacket;
	class SyncPositionDataResponse;
	class PropertyPacket;
	class MapUpdateResponse;
	class DressItemPacket;
	class FriendCharacter;
	class FriendDetail;
	class RankPacket;
	class SendMailPacket;
	class MailCharacter;
    class CharacterTitlePacket;
    class CardPacket;
    class CardSchedulePacket;
	class QuestListItem;
}

namespace rank {
	class RankRowPacket;
}

namespace google {
    namespace protobuf {
        template <typename Element>
        class RepeatedPtrField;
    }
}

namespace node {
	class GetCharacterResponse;
}

class CPlayer;
class CPlayerData;
class IItemBase;
class CQuestData;
class IQuestAdapt;
class CItemCount;
class CPetData;
class CPet;
class CInstance;
struct MonsterRow;
class CPetEquip;
struct ObjBase;
class CWorld;
class CMapObject;
class CDressItem;
class CTitleItem;
class CMapStateData;
template<class ID_TYPE, class T_OBJ>
class CGeneralState;
class CTaskSchedule;
class CQuestListItem;

typedef util::CAutoPointer<const CGeneralState<uint32_t, CMapStateData>> MAP_STATE_CONST_T;

inline void FillVectorData(::game::VectorPacket* outVec, const std::vector<float>* inVec) {
	outVec->set_x(inVec->at(0));
	outVec->set_y(inVec->at(1));
	outVec->set_z(inVec->at(2));
}

inline void FillVectorData(::game::VectorPacket* outVec, float x, float y, float z) {
	outVec->set_x(x);
	outVec->set_y(y);
	outVec->set_z(z);
}

// 登录服获得角色信息
extern void FillGetCharacterData(
	::node::GetCharacterResponse& outCharPacket,
	const CPlayerData& playerData);

// 填充角色数据到通讯数据包
extern void FillPlayerData(::game::CharacterBasic* outBasic,
    const util::CAutoPointer<CPlayerData>& playerData);



#endif /* FILLPACKETHELPER_H */
