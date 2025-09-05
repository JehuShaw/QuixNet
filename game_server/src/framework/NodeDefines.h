/*
 * File:   NodeDefines.h
 * Author: Jehu Shaw
 *
 */

// Configuration Header File

#ifndef NODEDEFINES_H
#define NODEDEFINES_H

#include "AppConfig.h"
#include "ShareDefines.h"

// this node register type
static const ControlCentreRegisterType NODE_REGISTER_TYPE = REGISTER_TYPE_WORKER;
// This node is running ?
extern volatile bool g_bExit;
// This node status
extern volatile long g_serverStatus;

#define CALL_INFINITE_WAITING -1

#define CALL_REGISTER_MS 15000

#define CALL_DEADLINE_MS 12000

#define CALL_UNREGISTER_MS 8000

#define TIMEOUT_MAX_TIMES_REMOVE_CHANNEL 100

#define TIMEOUT_MAX_TIMES_REMOVE_SERVER 100

// 3 second
#define KEEP_REGISTER_INTERVAL 300

// 3 keep register times
#define KEEP_REGISTER_TIMEOUT (KEEP_REGISTER_INTERVAL*3)

// MAX pet nums
#define MAX_PLAYER_PET_SIZE (4)

// CWrapPlayer keep time (600 second)
#define WRAP_PLAYER_KEEP_TIME 60000


// csv file define
const static char csv_field_terminator = ',';
const static char csv_line_terminator  = '\n';
const static char csv_enclosure_char   = '"';

//--------------------------------------
//  Module name Define
//--------------------------------------
#define CONTROLCENTRE_MODULE_NAME "ctrlcentre_module"
#define CONTROLUSER_MODULE_NAME "ctrluser_module"
#define LOGGING_MODEL_NAME "logging_module"
#define PLAYER_MODEL_NAME "player_module"
#define QUEST_MODEL_NAME "quest_module"
#define BACKPACK_MODEL_NAME "backpack_module"
#define WORLD_MODEL_NAME "world_module"
#define PET_MODEL_NAME "pet_module"
#define CHEAT_MODEL_NAME "cheat_module"
#define BATTLE_MODEL_NAME "battle_module"
#define DRESS_MODEL_NAME "dress_module"
#define CHAT_MODEL_NAME "chat_module"
#define FRIEND_MODEL_NAME "friend_module"
#define SHOP_MODEL_NAME "shop_module"
#define RANK_MODEL_NAME "rank_module"
#define MAIL_MODEL_NAME "mail_module"
#define PLAYER_BASIC_MODEL_NAME "player_basic_module"
#define ADVERT_MODEL_NAME "advert_module"
#define CARD_MODEL_NAME "card_module"
#define QUEST_LIST_MODEL_NAME "quest_list_module"

//--------------------------------------
//  AppConfig key name Define
//--------------------------------------
#define APPCONFIG_PROJECTNAME "ProjectName"
#define APPCONFIG_SERVERID "ServerID"
#define APPCONFIG_SERVERNAME "ServerName"
#define APPCONFIG_SERVERREGION "ServerRegion"
#define APPCONFIG_SERVERBIND "ServerBind"
#define APPCONFIG_ENDPOINT "EndPoint"
#define APPCONFIG_SERVANTCONNECT "ServantConnect"
#define APPCONFIG_SERVERCONNECT "ServerConnect"
#define APPCONFIG_RPCZTHREADS "RpczThreads"
#define APPCONFIG_GAMETHREADS "GameThreads"
#define APPCONFIG_ZMQIOTHREADS "ZmqioThreads"
#define APPCONFIG_TEMPLATEPATH "TemplatePath"
#define APPCONFIG_LOGGINGCACHEID "LoggingCacheID"
////////////////////////////////////////////////

//--------------------------------------
//  Property key type defines
//--------------------------------------

// memory cache key
#define GAME_CHAT_SYSTEM_ADD "game_chat_system_add"
#define GAME_CHAT_SYSTEM_GET "game_chat_system_get"
#define GAME_FRIEND_RESULT_LIST "game_friend_result_list"
#define GAME_RAND_PLAYER_LIST "game_rand_player_list"
#define GAME_MATCH_PLAYER "game_match_player"
#define GAME_CHECK_BLACKLIST "game_check_blacklist"
#define GAME_CHAT_PRIVATE_INIT "game_chat_private_init"
#define GAME_CHAT_PRIVATE_ADD "game_chat_private_add"
#define GAME_CHAT_PRIVATE_GET "game_chat_private_get"
#define GAME_CHAT_PRIVATE_DEL "game_chat_private_del"
#define GAME_CHAT_PRIVATE_LIST_ADD "game_chat_private_list_add"
#define GAME_CHAT_PRIVATE_LIST_GET "game_chat_private_list_get"
#define GAME_CHAT_PRIVATE_LIST_EXIST "game_chat_private_list_exist"
#define GAME_CHAT_PRIVATE_LIST_DEL "game_chat_private_list_del"
#define GAME_CHAT_PRIVATE_LIST_DEL_ALL "game_chat_private_list_del_all"
#define GAME_MAIL_INIT "game_mail_init"
#define GAME_MAIL_ADD "game_mail_add"
#define GAME_MAIL_GET_LIST "game_mail_get_list"
#define GAME_MAIL_GET_DETAIL "game_mail_get_detail"
#define GAME_MAIL_GET_ATTACHMENT "game_mail_get_attachment"
#define GAME_MAIL_DEL "game_mail_del"
#define GAME_MAIL_DEL_ALL "game_mail_del_all"
#define GAME_MAIL_ATTACHMENT_TAKEN "game_mail_attachment_taken"
#define GAME_ADVERT_INCENT "game_advert_incent"
#define CHECK_EXIST_ESCAPE_TABLE_NAME "game_check_name"
/////////////////////////////////////////////////////////////////////////
// �ı����ñ�ID���ͻ���ͨ���ı����ñ�ID ��ȡ�ı�����
#define CHAT_CODE_ADD_FRIEND "85"
// ���н��������ı�
#define TEXT_CODE_RANK_REWARD_MAIL_TITLE ""
// ���н��������ı�
#define TEXT_CODE_RANK_REWARD_MAIL_CONTENT ""

// ��Ƶ�������������ı�
#define TEXT_CODE_ADVINC_REWARD_MAIL_TITLE ""
// ��Ƶ�������������ı�
#define TEXT_CODE_ADVINC_REWARD_MAIL_CONTENT ""

//////////////////////////////////////////////////////////////////////////

// private chat keep time (second)
#define CHAT_PRIVATE_DURATION (int)30*24*3600
// private chat keep record size
#define CHAT_PRIVATE_KEEP_SIZE 500


// ������,���¼���ȼ�
#define FRIEND_RAND_PLAYER_LEVEL_INTERVAL 5
// ���Ҵ���
#define FRIEND_RAND_PLAYER_LEVEL_TIMES 5

// mail keep time (second)
#define MAIL_DURATION (int)30*24*3600
// mail keep record size
#define MAIL_KEEP_SIZE 500


#define ARRAY_POSITION_NIL -1

inline static unsigned char TMWdayToConfigWeek(unsigned char tmWday) {
	if(0 == tmWday) {
		return 7;
	} else {
		return tmWday;
	}
}

inline static unsigned char ConfigWeekToTMWday(unsigned char atTimeWeek) {
	if(7 == atTimeWeek) {
		return 0;
	} else {
		return atTimeWeek;
	}
}

inline static uint64_t CombinedId64(uint32_t low, uint32_t high) {
	id64_t id64(low, high);
	return id64.u64;
}

// attribute
inline static long AttributeFormula1(long nBase, long nLevel, double nGrow) {
    return (long)((double)nBase + (double)(nLevel - 1) * nGrow);
}

typedef std::vector<std::pair<uint32_t, uint32_t> > ID32_RANDOM_ARR_T;
typedef std::vector<std::pair<uint64_t, uint32_t> > ID64_RANDOM_ARR_T;
#define RANDOM_ARR_T(t) std::vector<std::pair<t, uint32_t> >

template<typename IDType>
inline static std::pair<IDType, uint32_t> GetRandomWeightItem(IDType id, uint32_t nWeight, uint32_t& nSum) {
	nSum += nWeight;
	return std::pair<IDType, uint32_t>(id, nSum);
}
// get random by weight value
template<typename IDType>
inline static IDType RandomByWeight(std::vector<std::pair<IDType, uint32_t> >& items, uint32_t& nSum, bool bUnique = true) {
	size_t nSize = items.size();
	if (nSize > 0) {
		uint32_t nPerWeight = 0;
		uint32_t nRand = Rand(1, nSum + 1);
		assert(nRand != 0);
		for (size_t j = 0; j < nSize; ++j) {
			if (nRand <= items[j].second) {
				if (bUnique) {
					items[j].second = 0;
					int32_t nDifWeight = items[j].second - nPerWeight;
					nSum -= nDifWeight;
					for (size_t i = j + 1; i < nSize; ++i) {
						items[i].second -= nDifWeight;
					}
				}
				return items[j].first;
			}
			nPerWeight = items[j].second;
		}
	}
	return ID_NULL;
}

// Get instance round id, ��ø����������ñ�ID
inline static uint32_t GetInstRoundCfgID(uint32_t nInstCfgID, int32_t nRound) {
	return (nInstCfgID * 100) + (nRound % 100);
}

#define GetJsonPairValue(json, index, value1, value2, userId, nCfgId) {\
	tiny::JArray arr;\
	json.GetAt(arr, index);\
	if (arr.Count() == 2) {\
		if (!arr.GetAt(value1, 0)) {\
			OutputError("!arr[0].Get() userId = " I64FMTD " nCfgId = %u ", userId, nCfgId);\
		}\
		if (!arr.GetAt(value2, 1)) {\
			OutputError("!arr[1].Get() userId = " I64FMTD " nCfgId = %u ", userId, nCfgId);\
		}\
	} else {\
		OutputError("arr.Count() != 2 userId = " I64FMTD " nCfgId = %u ", userId, nCfgId);\
	}\
}

#define GetTwoJsonValue(json1, json2, index, value1, value2, userId, nCfgId) {\
	if (!json1.GetAt(value1, index)) {\
		OutputError("!json1[%d].Get() userId = " I64FMTD " nCfgId = %u ", index, userId, nCfgId);\
	}\
	if (!json2.GetAt(value2, index)) {\
		OutputError("!json2[%d].Get() userId = " I64FMTD " nCfgId = %u ", index, userId, nCfgId);\
	}\
}

// attribute
inline static int CoordsToGrideXY(float fValue) {
	return (int)ceil(fValue);
}

// �����������ռ��
inline static int GetWorldPlayerRate(int nCurNum, int nMaxNum) {
	return nCurNum * 100 / nMaxNum;
}

// ��Ϸ��������
#define BASE_PERCENT_VALUE (10000)

// ս����=��Ѫ��*0.012+����*1��*2.5+������+��+����+���ܣ�*1
inline static int32_t G_CalcFightPower(int hp, int atk, int crit, int block, int hit, int dodge) {
    return (int32_t)((hp*0.012 + atk)*2.5 + (crit + block + hit + dodge) / BASE_PERCENT_VALUE);
}

// 
enum ePlayerUnit
{
	PLAYER_UNIT_BASIC,
	PLAYER_UNIT_BACKPACK,
	PLAYER_UNIT_MAP,
	PLAYER_UNIT_QUEST,
	PLAYER_UNIT_PET,
	PLAYER_UNIT_TRIGGER,
	PLAYER_UNIT_DRESS,
	PLAYER_UNIT_CHAT,
	PLAYER_UNIT_FRIEND,
	PLAYER_UNIT_RANK,
	PLAYER_UNIT_CARD,
	// �����
	PLAYER_UNIT_QUEST_LIST,
	PLAYER_UNIT_SIZE,
};

enum ePlayerStatus
{
	PLAYER_STATUS_NIL,
    PLAYER_STATUS_OFFLINE,
    PLAYER_STATUS_ONLINE,
};

enum eIDType {
    IDTYPE_USER,
    IDTYPE_ROBOT,
};

enum ePlayerLogType {
    PLOG_TYPE_NIL,
    PLOG_TYPE_BEHAVIOUR,                    // �����־����
};

enum ePlayerLogSubType {
    PLOG_SUBTYPE_NIL,
};

// �����Ϊ��־
enum ePlayerLogBehaviourAction {
    PLOG_BEHACTION_NIL,
    PLOG_BEHACTION_PVE_COUNT,               // �����ͨPVE�����Ĵ���
    PLOG_BEHACTION_DIFFPVE_COUNT,           // ��ɾ�ӢPVE�����Ĵ���
    PLOG_BEHACTION_WARRIORS_COUNT,          // �����˫�����Ĵ���
    PLOG_BEHACTION_FIEFTRADE_COUNT,         // ��ɷ��ó�׵Ĵ���
    PLOG_BEHACTION_FIEFMINE_COUNT,          // ��ɷ�زɿ�Ĵ���
    PLOG_BEHACTION_ACTIVITYPVE_COUNT,       // ��ɻ�����Ĵ���
};

//////////////////////////////////////////////////////////////////////////

// ��Ϸ�¼����Ͷ���
enum eGameEventType {
	GEVENT_NIL = 0,
	// �������
	GEVENT_PLAYER_UPGRADE,
	// �ύ����
	GEVENT_QUEST_SUBMITTED,
	// ��ɶԻ�
	GEVENT_DIALOG,
	// ս��ʤ��
	GEVENT_BATTLE_SUCCESS,
	// ʹ����Ʒ
	GEVENT_USE_ITEM,
	// �ɼ���Ʒ
	GEVENT_COLLECT_ITEMS,
	// ��������
	GEVENT_PET_UPGRADE,
	// �������
	GEVENT_PET_EVOLVE,
	// ����ս������
	GEVENT_PET_FC_UPDATE,
	// ��ɫ������ʱ�򴥷�
	GEVENT_CHARACTER_CREATE,
	// �ڶ���00��00��00 �|�l
	GEVENT_NEXT_DAY,
};

 // ����: 1���� 2���� 3����� 4֧��
enum eQuestType
{
	QUEST_MAIN = 1,
	QUEST_GUIDED = 2,
	QUEST_TASK_LIST = 3,
	QUEST_BRANCH = 4,
};

// ����״̬����
enum eQuestStatus {
	// (���Խ�ȡ��
	QUEST_STATUS_NIL = 0,
	// �Ѿ����(δ�ύ��
	QUEST_STATUS_FINISHED = 1,
	// �Ѿ���ȡ
	QUEST_STATUS_TAKEN = 2,
	// ����ɲ����ύ��
	QUEST_STATUS_SUBMITTED = 3,
};

// ��������
enum eQuestTargetType {
	QUEST_TARGET_NIL,
	QUEST_TARGET_DIALOG,					// ��ɶԻ�
	QUEST_TARGET_PVE,						// ���PVE
	QUEST_TARGET_COLLECTION,				// ��ɲɼ�
	QUEST_TARGET_ARRIVAL,					// ��ɲȵ�
	QUEST_TARGET_BUBBLE,					// ���ð��
	QUEST_TARGET_POPUP,						// ��ɵ���
	QUEST_TARGET_USE_ITEM,					// ���ʹ����Ʒ
	QUEST_TARGET_INTERACTION,				// ��ɽ���
	QUEST_TARGET_ANSWER,					// ��ɴ���
	QUEST_TARGET_PET_EVOLVE,				// ��ɳ������
	QUEST_TARGET_SIZE,
};

// ս������ ������ֱ��������ȡȫ�ֱ������˺�ϵ����
enum eBattleRate
{
	BATTLE_RATE_S = 0,
	BATTLE_RATE_A = 1,
	BATTLE_RATE_B = 2,
};

// ��������
enum eDropType
{
	DROP_NIL,
	DROP_DIRECT,
	DROP_RANDOM,
	DROP_RANDOM_UNIQUE,
	DROP_RANGE,
	DROP_RANGE_RANDOM,
	DROP_RANGE_RANDOM_UNIQUE
};

// ������Ʒ���
enum eDropItemType
{
    DROP_ITEM_NIL,
    DROP_ITEM_ITEM,     // ��Ʒ
    DROP_ITEM_PIECE,    // ��Ƭ������ϵͳ��
    DROP_ITEM_CARD,     // ��Ƭ������ϵͳ��
};

inline static bool IsDropTypeCardOrPiece(int32_t type) {
    return (DROP_ITEM_PIECE == type 
        || DROP_ITEM_CARD == type);
}

// �����¼���������
enum eEventQuestCond
{
	EVENT_QUEST_COND_NIL,
	// ����Ƿ񵽴�ĳλ��
	EVENT_QUEST_COND_POS,
	// �����Ƿ񲥷����
	EVENT_QUEST_COND_PLAY_ANIM,
	// ��������״ֱ̬�Ӵ���
	EVENT_QUEST_COND_STATE,
	// ����Ѱ·����
	EVENT_QUEST_COND_NAVIGATE,
};

// ����������������
enum eTriggerCType
{
	TRIGGER_CTYPE_NIL,
	// ��ҵȼ�
	TRIGGER_CTYPE_PLAYER_LEVEL,
	// �������
	TRIGGER_CTYPE_QUEST_SUBMITTED,
	// ����ȼ�
	TRIGGER_CTYPE_PET_LEVEL,
	// ʹ����Ʒ
	TRIGGER_CTYPE_USE_ITEM,

};

// ���������¼�����
enum eTriggerEType
{
	TRIGGER_ETYPE_NIL,
	// ����ð��
	TRIGGER_ETYPE_QUEST_BUBBLE,
	// ���ŵ���
	TRIGGER_ETYPE_QUEST_POPUP,
	// �����԰�
	TRIGGER_ETYPE_QUEST_NARRATOR,
	// ���Ŷ���
	TRIGGER_ETYPE_QUEST_PLAY_ANIM,
	// ����
	TRIGGER_ETYPE_QUEST_TRANSFER,
	// �ύ��Ʒ
	TRIGGER_ETYPE_QUEST_SUBMIT_ITEMS,
	// ˢ��NPC
	TRIGGER_ETYPE_QUEST_SPAWN_NPC,
	// ����NPC
	TRIGGER_ETYPE_QUEST_DESTROY_NPC,
	// ʹ����Ʒ
	TRIGGER_ETYPE_QUEST_USE_ITEM,
	// NPC�ƶ�
	TRIGGER_ETYPE_QUEST_NPC_MOVE,
	// ˢ���ɼ���
	TRIGGER_ETYPE_QUEST_SPAWN_COLLECTION,
	// ���ٲɼ���
	TRIGGER_ETYPE_QUEST_DESTORY_COLLECTION,
	// ��ת�Ի�
	TRIGGER_ETYPE_QUEST_CONVERSATION,
	// �¼�����
	TRIGGER_ETYPE_SIZE,
};

// ��Ʒʱ������
enum eItemTimeType
{
	ITEM_TIME_TYPE_NIL,
	// �������ʱ
	ITEM_TIME_TYPE_CREATE,
	// �������ڿ���ʹ��
	ITEM_TIME_TYPE_RANGE,
};


// 1��ѪҩƷ:Ч��ֵ��1�ٷֱȣ�2�����ֵ�� | ֵ����ȴʱ��
// 2�����ID(�ߵ�����ID��
// 3���������򿪺��ȡ������ֵ
// 4���ﾭ�鵤��ʹ�û�ȡ�ĳ��ﾭ��ֵ
// 7���ͣ�ʹ�ü����Ӧ�ĳƺ�
// 8���ͣ�ʹ��ֱ�����ĳ����ĵ��ߣ����� | ��ɵ�����id
// 9���ͣ�ʹ�ÿɻ�ó������ | ��õĳ���id
// 10���ͣ�ʹ�ÿɻ�ó���װ�������� | ���װ��id
// 12���ͣ�����|��õ�ͷ��id
// 13���ͣ�����|��õ�ͷ���id

// ��Ʒʹ��ʱ����Ĳ�������
enum eItemUseOperateType
{
	ITEM_USE_NIL,
	ITEM_USE_ADD_HP,
	ITEM_USE_GIFT,
	ITEM_USE_ADD_POWER,
	ITEM_USE_ADD_PET_EXP,
	ITEM_USE_TITLE = 7,
	ITEM_USE_FINISHED_QUEST,
	ITEM_USE_CREATE_PET,
	ITEM_USE_CREATE_PET_EQUIP,
	ITEM_USE_HEAD,
	ITEM_USE_HEADFRAME,
};

// ��������
enum ePetSkillType
{
	PET_SKILL_TYPE_ERR	= 0, 
	// ��ͨ����
	PET_SKILL_TYPE_NOR	= 1, 
	// BUFF����
	PET_SKILL_TYPE_BUFF	= 2, 
};

// ����״̬
enum ePetStatusType
{
	// ��Ϣ
	PET_STATUS_TYPE_REST = 0, 
	// ��ս
	PET_STATUS_TYPE_BATTLE = 1, 
};

// ��������(���ñ����)
enum ePetAttrType
{
	PET_ATTR_NIL,
	// Ѫ��
	PET_ATTR_HP,
	// ������
	PET_ATTR_ATK,
	// ����
	PET_ATTR_CRIT,
	// ��
	PET_ATTR_BLOCK,
	// ����
	PET_ATTR_HIT,
	// ����
	PET_ATTR_DODGE,
	// �ƶ��ٶ�
	PET_ATTR_SPEED,
	// Ѫ��
	PET_ATTR_MAX_HP
};

// ����װ��λ��
enum ePetEquipPartType {
	PET_EQUIP_PART_NIL,
	// ͷ��
	PET_EQUIP_PART_HEAD,
	// �·�
	PET_EQUIP_PART_BODY,
	// ��Ȧ
	PET_EQUIP_PART_NECK,
	// ����
	PET_EQUIP_PART_HAND,
};

// ��ͼ���Ӵ洢�Ķ�������
enum eMapObjType {
	MAP_OBJ_NIL,
	MAP_OBJ_PLAYER,
};

// �л���ͼ��������
enum eSwitchMapType {
	SWITCH_MAP_NIL,
	// ָ����ͼ����
	SWITCH_MAP_SPECIFY,
	// ���͵㴫��
	SWITCH_MAP_TRANSPORT,
};

// ��״̬
enum eWorldStatus {
	WORLD_STATUS_NIL,
	// ����
	WORLD_STATUS_SMOOTH,
	// ӵ��
	WORLD_STATUS_CONGESTION,
	// ����
	WORLD_STATUS_FULL,
};

// ��ͼ״̬
enum eMapStatusType
{
	MAP_STATUS_NIL = 0,
	// δ����
	MAP_STATUS_LOCK = 1,
	// ����(����)
	MAP_STATUS_OPEN = 2,
};

// װ�粿��״̬
enum eDressStatusType
{
	DRESS_STATUS_NIL = 0,
	// δװ��
	DRESS_STATUS_UNDRESS = 1,
	// ��װ��
	DRESS_STATUS_DRESSED = 2,
};

// װ��Ĭ��״̬
enum eDressInitType
{
	DRESS_INIT_NIL = 0,
	// δѡ��
	DRESS_INIT_UNSELECT = 1,
	// Ĭ��ѡ��
	DRESS_INIT_SELECTED = 2,
};

// װ���
enum ePlayerDressUnit
{
	PLAYER_DRESS_PART_NIL,
	PLAYER_DRESS_PART_HEAD,
	PLAYER_DRESS_PART_FACE,
	PLAYER_DRESS_PART_TOP,  // ��װ
	PLAYER_DRESS_PART_BUTTON, // ��װ
	PLAYER_DRESS_PART_SHOES,
	PLAYER_DRESS_PART_HAND,
	PLAYER_DRESS_PART_BACK,
	PLAYER_DRESS_SIZE
};

// װ��������������
enum eDressOtherUnlockType
{
    DRESS_OTHER_UNLOCK_NIL,
    DRESS_OTHER_UNLOCK_GIFT, // �������
};

enum eGenderType
{
    GENDER_TYPE_DEFAULT = 0,
    GENDER_TYPE_FEMALE = 0, // Ů
    GENDER_TYPE_MALE = 1,   // ��
};

enum eChatType
{
	CHAT_TYPE_NIL,
	// ����Ƶ��
	CHAT_TYPE_GLOBAL,
	// ����Ƶ��(ͬ��ͼ��ң�
	CHAT_TYPE_NEAR,
	// ���Ƶ��
	CHAT_TYPE_GROUP,
	// ˽��Ƶ��
	CHAT_TYPE_PRIVATE,
	// ϵͳƵ��
	CHAT_TYPE_SYSTEM,
};

enum eChatContentType
{
	CHAT_CONTENT_TYPE_NIL,
	// ���������ı�
	CHAT_CONTENT_TYPE_TEXT,
	// ������������
	CHAT_CONTENT_TYPE_SOUND,
	// ��������ͼƬ
	CHAT_CONTENT_TYPE_PICTURE,
	// ���ò��ҵ���Ϣ
	CHAT_CONTENT_TYPE_CODE,
};

enum eApplyFriendResult
{
	APPLY_FRIEND_NIL,
	APPLY_FRIEND_AGREE,
	APPLY_RRIEND_REFUSE,
};

// ��Ʒ�ۿ�
enum eShopItemDiscount
{
    SHOP_ITEM_DISCOUNT_NIL = 0,
};

// ����ѧ������
enum ePetSubjectType
{
    PET_TYPE_NIL = 0,
    PET_TYPE_CHINESE,               // ����
    PET_TYPE_SCIENCE,               // ��ѧ
    PET_TYPE_ENGLISH,               // Ӣ��
    PET_TYPE_MATH,                  // ��ѧ
    PET_TYPE_SIZE
};

// С��Ϸ��ֻ�������⣩
enum eGameIDType
{
    GAME_ID_NIL = 0,
    GAME_ID_CIRCUMFERENCE = 1,      // �ܳ�
};

enum eBattleResultType
{
    BATTLE_RET_VERIFY = -1,         // ��֤ʧ��
    BATTLE_RET_FAIL = 0,            // ս��ʧ��
    BATTLE_RET_PASS = 1             // ս��ͨ��
};

// ��������
enum eFBType
{
    FB_TYPE_NIL = 0,
    FB_TYPE_SINGLE = 1,             // ���˸���
    FB_TYPE_MULTI = 2,              // ���˸���
};

// ս������
enum eBattleType
{
    BATTLE_TYPE_NIL = 0,
    BATTLE_TYPE_AUTO,               // �Զ���������ʱ�չ���
    BATTLE_TYPE_MANUAL,             // �ֶ�
    BATTLE_TYPE_BUFF,               // BUFF
};

enum eBattleSummaryRet
{
    BAT_SUMMARY_PET = -2,           // �����չ�
    BAT_SUMMARY_MON = -1,           // �����չ�
    BAT_SUMMARY_FAIL = 0,           // ʧ��

    BAT_SUMMARY_B = 1,              // ���� һ��
    BAT_SUMMARY_A = 2,              // ���� ����
    BAT_SUMMARY_S = 3,              // ���� ����

};

//  �����ͷ�����
enum eSkillReleaseCond
{
    SKILL_REL_COND_NIL,             // ��������
    SKILL_REL_COND_WIN_ONE,         // ���1��
    SKILL_REL_COND_WIN_KEEP,        // �������X��
    SKILL_REL_COND_WIN_CNT_AND,     // �ۼƴ��X�������� S A
    SKILL_REL_COND_WIN_CNT,         // �ۼƴ��X��
    SKILL_REL_COND_WIN_KEEP_AND,    // �������X�������� S A
    SKILL_REL_COND_WRONG_ONE,       // ���1��
};

// ���ܶ���Ч������
enum eSkillAniEffType
{
    SKILL_ANI_EFF_NOR = 0,          // ֱ��Ч��(Ĭ��)
    SKILL_ANI_EFF_BOMB,             // �չ�
};


// ɾ����������
enum eRemoveFriendType
{
	// ����ɾ������
	REMOVE_FRIEND_NORMAL = 0,
	// ���������ɾ������
	REMOVE_FRIEND_BACKLIST = 1,
};

// �ʼ���������
enum eMailTitleType
{
	// �ı�����
	MAIL_TITLE_TEXT,
	// ��������, �ͻ���ͨ�����ñ�ID ��ȡ�ı�
	MAIL_TITLE_CODE,
};

// �ʼ���������
enum eMailContentType
{
	// �ı�����
	MAIL_CONTENT_TEXT,
	// json ��ʽ��������
	MAIL_CONTENT_JSON_CODE,
};

// �ʼ���������
enum eMailAttachmentType
{
	// �ı�����
	MAIL_ATTACHMENT_NORMAL,
	// json ��ʽ[��ƷID|��Ʒ����]����
	MAIL_ATTACHMENT_JSON_ITEMS,
};

// ���״̬ ͨ��
enum eItemStatusType
{
	ITEM_STATUS_NIL,
	ITEM_STATUS_UNUSE,  	    // δʹ��(δ����)
	ITTEM_STATUS_USED,      	// ��ʹ��(����)
};

enum eTiTleAndHeadInfoType
{
    TITLE_HEAD_INFO_NIL,
    TITLE_HEAD_INFO_TITLE,      // �ƺ�
    TITLE_HEAD_INFO_HEAD,       // ͷ��
    TITLE_HEAD_INFO_HEAD_FRAME, // ͷ���
};

// ������Ƶ��������
enum eAdvIncRewardType
{
	// δȷ������
	ADV_INC_REWARD_NIL,
	// ������Ʒ-��Ӧreward���id
	ADV_INC_REWARD_CFG,
	// ��������
	ADV_INC_REWARD_RELIVE,
	// ����С��Ϸ��Ŀ�𰸣���ȡanswer��
	ADV_INC_REWARD_ANSWER,
};

// �����������
enum eTaskScheduleType
{
    TASK_SCHEDULE_NIL,
    TASK_SCHEDULE_CARD,         // ��������
};

// �������״̬
enum eScheduleStatusType
{
    SCH_STATUS_NIL,
    SCH_STATUS_INCOMPLETE,      // δ���
    SCH_STATUS_COMPLETED,       // �����    
};

// ����ϵͳ��Ƭ����
enum eCardType
{
    CARD_TYPE_NIL,
    CARD_TYPE_FPEPOLE,          // ���˿�
    CARD_TYPE_DATE,             // ���ڿ�
    CARD_TYPE_FUNSCIENCE,       // Ȥζ���տ�   
    CARD_TYPE_COMMONSENCE,      // ��ʶ��
    CARD_TYPE_OTHER,            // �ջ����ۿ� what ?xxk,,
    CARD_TYPE_ANY = 10,         // �κ�����
};

// ��������(reward table)
enum eRewardType
{
    REWARD_TYPE_NIL,
    REWARD_TYPE_DAY_CHECKIN,            // ÿ��ǩ��
    REWARD_TYPE_CML_CHECKIN,            // �ۼ�ǩ��   
    REWARD_TYPE_LEVEL_GIFT,             // �ȼ�ǩ��   
    REWARD_TYPE_ACTIVE_REWARD = 4,      // ��Ծ

    REWARD_TYPE_ONLINE = 101,           // ����ʱ��
    REWARD_TYPE_COMP_CARNIVAL = 122,    // 7�տ���ɶ�

    REWARD_TYPE_LEVEL_PLAYER = 140,     // ���˵ȼ����� ǰ��

    REWARD_TYPE_CARD_SCHEDUAL = 201,    // ����ϵͳ����
};

enum ePrologueProcessType
{
    PRO_PROCESS_TYPE_NIL,
    PRO_PROCESS_TYPE_INIT,              // ��ʼ
    PRO_PROCESS_TYPE_COMPL,             // ���
};

// ���ܽ�����Ӧ�Ĺ�������
enum eFunctionType {
	FUNCTION_QUEST_LIST = 1,
};

// ���������
enum eQuestListType {
	QUEST_LIST_ONE = 1,
	QUEST_LIST_TWO,
	QUEST_LIST_THREE,
	QUEST_LIST_FOUR,
};


#endif  /* NODEDEFINES_H */

