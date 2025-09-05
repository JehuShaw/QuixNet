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
// 文本配置表ID，客户端通过文本配置表ID 读取文本内容
#define CHAT_CODE_ADD_FRIEND "85"
// 排行奖励标题文本
#define TEXT_CODE_RANK_REWARD_MAIL_TITLE ""
// 排行奖励内容文本
#define TEXT_CODE_RANK_REWARD_MAIL_CONTENT ""

// 视频激励奖励标题文本
#define TEXT_CODE_ADVINC_REWARD_MAIL_TITLE ""
// 视频激励奖励内容文本
#define TEXT_CODE_ADVINC_REWARD_MAIL_CONTENT ""

//////////////////////////////////////////////////////////////////////////

// private chat keep time (second)
#define CHAT_PRIVATE_DURATION (int)30*24*3600
// private chat keep record size
#define CHAT_PRIVATE_KEEP_SIZE 500


// 随机玩家,上下间隔等级
#define FRIEND_RAND_PLAYER_LEVEL_INTERVAL 5
// 重找次数
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

// Get instance round id, 获得副本波次配置表ID
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

// 计算线内玩家占比
inline static int GetWorldPlayerRate(int nCurNum, int nMaxNum) {
	return nCurNum * 100 / nMaxNum;
}

// 游戏基础比率
#define BASE_PERCENT_VALUE (10000)

// 战斗力=（血量*0.012+攻击*1）*2.5+（暴击+格挡+命中+闪避）*1
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
	// 任务榜
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
    PLOG_TYPE_BEHAVIOUR,                    // 玩家日志类型
};

enum ePlayerLogSubType {
    PLOG_SUBTYPE_NIL,
};

// 玩家行为日志
enum ePlayerLogBehaviourAction {
    PLOG_BEHACTION_NIL,
    PLOG_BEHACTION_PVE_COUNT,               // 完成普通PVE副本的次数
    PLOG_BEHACTION_DIFFPVE_COUNT,           // 完成精英PVE副本的次数
    PLOG_BEHACTION_WARRIORS_COUNT,          // 完成无双副本的次数
    PLOG_BEHACTION_FIEFTRADE_COUNT,         // 完成封地贸易的次数
    PLOG_BEHACTION_FIEFMINE_COUNT,          // 完成封地采矿的次数
    PLOG_BEHACTION_ACTIVITYPVE_COUNT,       // 完成活动副本的次数
};

//////////////////////////////////////////////////////////////////////////

// 游戏事件类型定义
enum eGameEventType {
	GEVENT_NIL = 0,
	// 玩家升级
	GEVENT_PLAYER_UPGRADE,
	// 提交任务
	GEVENT_QUEST_SUBMITTED,
	// 完成对话
	GEVENT_DIALOG,
	// 战斗胜利
	GEVENT_BATTLE_SUCCESS,
	// 使用物品
	GEVENT_USE_ITEM,
	// 采集物品
	GEVENT_COLLECT_ITEMS,
	// 宠物升级
	GEVENT_PET_UPGRADE,
	// 宠物进化
	GEVENT_PET_EVOLVE,
	// 宠物战力更新
	GEVENT_PET_FC_UPDATE,
	// 角色创建的时候触发
	GEVENT_CHARACTER_CREATE,
	// 第二天00：00：00 觸發
	GEVENT_NEXT_DAY,
};

 // 类型: 1主线 2引导 3任务榜 4支线
enum eQuestType
{
	QUEST_MAIN = 1,
	QUEST_GUIDED = 2,
	QUEST_TASK_LIST = 3,
	QUEST_BRANCH = 4,
};

// 任务状态类型
enum eQuestStatus {
	// (可以接取）
	QUEST_STATUS_NIL = 0,
	// 已经完成(未提交）
	QUEST_STATUS_FINISHED = 1,
	// 已经接取
	QUEST_STATUS_TAKEN = 2,
	// 已完成并且提交了
	QUEST_STATUS_SUBMITTED = 3,
};

// 任务类型
enum eQuestTargetType {
	QUEST_TARGET_NIL,
	QUEST_TARGET_DIALOG,					// 完成对话
	QUEST_TARGET_PVE,						// 完成PVE
	QUEST_TARGET_COLLECTION,				// 完成采集
	QUEST_TARGET_ARRIVAL,					// 完成踩点
	QUEST_TARGET_BUBBLE,					// 完成冒泡
	QUEST_TARGET_POPUP,						// 完成弹窗
	QUEST_TARGET_USE_ITEM,					// 完成使用物品
	QUEST_TARGET_INTERACTION,				// 完成交互
	QUEST_TARGET_ANSWER,					// 完成答题
	QUEST_TARGET_PET_EVOLVE,				// 完成宠物进化
	QUEST_TARGET_SIZE,
};

// 战斗评级 （可以直接用来获取全局表评级伤害系数）
enum eBattleRate
{
	BATTLE_RATE_S = 0,
	BATTLE_RATE_A = 1,
	BATTLE_RATE_B = 2,
};

// 掉落类型
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

// 掉落物品类别
enum eDropItemType
{
    DROP_ITEM_NIL,
    DROP_ITEM_ITEM,     // 物品
    DROP_ITEM_PIECE,    // 碎片（集卡系统）
    DROP_ITEM_CARD,     // 卡片（集卡系统）
};

inline static bool IsDropTypeCardOrPiece(int32_t type) {
    return (DROP_ITEM_PIECE == type 
        || DROP_ITEM_CARD == type);
}

// 任务事件触发条件
enum eEventQuestCond
{
	EVENT_QUEST_COND_NIL,
	// 玩家是否到达某位置
	EVENT_QUEST_COND_POS,
	// 动画是否播放完成
	EVENT_QUEST_COND_PLAY_ANIM,
	// 满足任务状态直接触发
	EVENT_QUEST_COND_STATE,
	// 任务寻路触发
	EVENT_QUEST_COND_NAVIGATE,
};

// 触发器，条件类型
enum eTriggerCType
{
	TRIGGER_CTYPE_NIL,
	// 玩家等级
	TRIGGER_CTYPE_PLAYER_LEVEL,
	// 完成任务
	TRIGGER_CTYPE_QUEST_SUBMITTED,
	// 宠物等级
	TRIGGER_CTYPE_PET_LEVEL,
	// 使用物品
	TRIGGER_CTYPE_USE_ITEM,

};

// 触发器，事件类型
enum eTriggerEType
{
	TRIGGER_ETYPE_NIL,
	// 播放冒泡
	TRIGGER_ETYPE_QUEST_BUBBLE,
	// 播放弹窗
	TRIGGER_ETYPE_QUEST_POPUP,
	// 播放旁白
	TRIGGER_ETYPE_QUEST_NARRATOR,
	// 播放动画
	TRIGGER_ETYPE_QUEST_PLAY_ANIM,
	// 传送
	TRIGGER_ETYPE_QUEST_TRANSFER,
	// 提交物品
	TRIGGER_ETYPE_QUEST_SUBMIT_ITEMS,
	// 刷出NPC
	TRIGGER_ETYPE_QUEST_SPAWN_NPC,
	// 销毁NPC
	TRIGGER_ETYPE_QUEST_DESTROY_NPC,
	// 使用物品
	TRIGGER_ETYPE_QUEST_USE_ITEM,
	// NPC移动
	TRIGGER_ETYPE_QUEST_NPC_MOVE,
	// 刷出采集物
	TRIGGER_ETYPE_QUEST_SPAWN_COLLECTION,
	// 销毁采集物
	TRIGGER_ETYPE_QUEST_DESTORY_COLLECTION,
	// 跳转对话
	TRIGGER_ETYPE_QUEST_CONVERSATION,
	// 事件数量
	TRIGGER_ETYPE_SIZE,
};

// 物品时间类型
enum eItemTimeType
{
	ITEM_TIME_TYPE_NIL,
	// 创建后计时
	ITEM_TIME_TYPE_CREATE,
	// 在区间内可以使用
	ITEM_TIME_TYPE_RANGE,
};


// 1回血药品:效果值（1百分比，2具体的值） | 值，冷却时间
// 2礼包：ID(走掉落表的ID）
// 3体力丹：打开后获取的体力值
// 4宠物经验丹：使用获取的宠物经验值
// 7类型：使用激活对应的称号
// 8类型：使用直接完成某任务的道具，类型 | 完成的任务id
// 9类型：使用可获得宠物，类型 | 获得的宠物id
// 10类型：使用可获得宠物装备，类型 | 获得装备id
// 12类型：类型|获得的头像id
// 13类型：类型|获得的头像框id

// 物品使用时具体的操作类型
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

// 技能类型
enum ePetSkillType
{
	PET_SKILL_TYPE_ERR	= 0, 
	// 普通攻击
	PET_SKILL_TYPE_NOR	= 1, 
	// BUFF攻击
	PET_SKILL_TYPE_BUFF	= 2, 
};

// 宠物状态
enum ePetStatusType
{
	// 休息
	PET_STATUS_TYPE_REST = 0, 
	// 出战
	PET_STATUS_TYPE_BATTLE = 1, 
};

// 宠物属性(配置表关联)
enum ePetAttrType
{
	PET_ATTR_NIL,
	// 血量
	PET_ATTR_HP,
	// 攻击力
	PET_ATTR_ATK,
	// 暴击
	PET_ATTR_CRIT,
	// 格挡
	PET_ATTR_BLOCK,
	// 命中
	PET_ATTR_HIT,
	// 闪避
	PET_ATTR_DODGE,
	// 移动速度
	PET_ATTR_SPEED,
	// 血条
	PET_ATTR_MAX_HP
};

// 宠物装备位置
enum ePetEquipPartType {
	PET_EQUIP_PART_NIL,
	// 头饰
	PET_EQUIP_PART_HEAD,
	// 衣服
	PET_EQUIP_PART_BODY,
	// 项圈
	PET_EQUIP_PART_NECK,
	// 护腕
	PET_EQUIP_PART_HAND,
};

// 地图格子存储的对象类型
enum eMapObjType {
	MAP_OBJ_NIL,
	MAP_OBJ_PLAYER,
};

// 切换地图操作类型
enum eSwitchMapType {
	SWITCH_MAP_NIL,
	// 指定地图传送
	SWITCH_MAP_SPECIFY,
	// 传送点传送
	SWITCH_MAP_TRANSPORT,
};

// 线状态
enum eWorldStatus {
	WORLD_STATUS_NIL,
	// 流畅
	WORLD_STATUS_SMOOTH,
	// 拥挤
	WORLD_STATUS_CONGESTION,
	// 爆满
	WORLD_STATUS_FULL,
};

// 地图状态
enum eMapStatusType
{
	MAP_STATUS_NIL = 0,
	// 未解锁
	MAP_STATUS_LOCK = 1,
	// 解锁(开放)
	MAP_STATUS_OPEN = 2,
};

// 装扮部件状态
enum eDressStatusType
{
	DRESS_STATUS_NIL = 0,
	// 未装扮
	DRESS_STATUS_UNDRESS = 1,
	// 已装扮
	DRESS_STATUS_DRESSED = 2,
};

// 装扮默认状态
enum eDressInitType
{
	DRESS_INIT_NIL = 0,
	// 未选择
	DRESS_INIT_UNSELECT = 1,
	// 默认选择
	DRESS_INIT_SELECTED = 2,
};

// 装扮槽
enum ePlayerDressUnit
{
	PLAYER_DRESS_PART_NIL,
	PLAYER_DRESS_PART_HEAD,
	PLAYER_DRESS_PART_FACE,
	PLAYER_DRESS_PART_TOP,  // 上装
	PLAYER_DRESS_PART_BUTTON, // 下装
	PLAYER_DRESS_PART_SHOES,
	PLAYER_DRESS_PART_HAND,
	PLAYER_DRESS_PART_BACK,
	PLAYER_DRESS_SIZE
};

// 装扮其他解锁类型
enum eDressOtherUnlockType
{
    DRESS_OTHER_UNLOCK_NIL,
    DRESS_OTHER_UNLOCK_GIFT, // 礼包解锁
};

enum eGenderType
{
    GENDER_TYPE_DEFAULT = 0,
    GENDER_TYPE_FEMALE = 0, // 女
    GENDER_TYPE_MALE = 1,   // 男
};

enum eChatType
{
	CHAT_TYPE_NIL,
	// 世界频道
	CHAT_TYPE_GLOBAL,
	// 附近频道(同地图玩家）
	CHAT_TYPE_NEAR,
	// 组队频道
	CHAT_TYPE_GROUP,
	// 私聊频道
	CHAT_TYPE_PRIVATE,
	// 系统频道
	CHAT_TYPE_SYSTEM,
};

enum eChatContentType
{
	CHAT_CONTENT_TYPE_NIL,
	// 聊天内容文本
	CHAT_CONTENT_TYPE_TEXT,
	// 聊天内容语音
	CHAT_CONTENT_TYPE_SOUND,
	// 聊天内容图片
	CHAT_CONTENT_TYPE_PICTURE,
	// 配置查找的消息
	CHAT_CONTENT_TYPE_CODE,
};

enum eApplyFriendResult
{
	APPLY_FRIEND_NIL,
	APPLY_FRIEND_AGREE,
	APPLY_RRIEND_REFUSE,
};

// 商品折扣
enum eShopItemDiscount
{
    SHOP_ITEM_DISCOUNT_NIL = 0,
};

// 宠物学科类型
enum ePetSubjectType
{
    PET_TYPE_NIL = 0,
    PET_TYPE_CHINESE,               // 语文
    PET_TYPE_SCIENCE,               // 科学
    PET_TYPE_ENGLISH,               // 英语
    PET_TYPE_MATH,                  // 数学
    PET_TYPE_SIZE
};

// 小游戏（只定义特殊）
enum eGameIDType
{
    GAME_ID_NIL = 0,
    GAME_ID_CIRCUMFERENCE = 1,      // 周长
};

enum eBattleResultType
{
    BATTLE_RET_VERIFY = -1,         // 验证失败
    BATTLE_RET_FAIL = 0,            // 战斗失败
    BATTLE_RET_PASS = 1             // 战斗通过
};

// 副本类型
enum eFBType
{
    FB_TYPE_NIL = 0,
    FB_TYPE_SINGLE = 1,             // 单人副本
    FB_TYPE_MULTI = 2,              // 多人副本
};

// 战斗类型
enum eBattleType
{
    BATTLE_TYPE_NIL = 0,
    BATTLE_TYPE_AUTO,               // 自动触发（定时普攻）
    BATTLE_TYPE_MANUAL,             // 手动
    BATTLE_TYPE_BUFF,               // BUFF
};

enum eBattleSummaryRet
{
    BAT_SUMMARY_PET = -2,           // 宠物普攻
    BAT_SUMMARY_MON = -1,           // 怪物普攻
    BAT_SUMMARY_FAIL = 0,           // 失败

    BAT_SUMMARY_B = 1,              // 评价 一般
    BAT_SUMMARY_A = 2,              // 评价 优秀
    BAT_SUMMARY_S = 3,              // 评价 完美

};

//  技能释放条件
enum eSkillReleaseCond
{
    SKILL_REL_COND_NIL,             // 无需条件
    SKILL_REL_COND_WIN_ONE,         // 答对1题
    SKILL_REL_COND_WIN_KEEP,        // 连续答对X题
    SKILL_REL_COND_WIN_CNT_AND,     // 累计答对X题且评价 S A
    SKILL_REL_COND_WIN_CNT,         // 累计答对X题
    SKILL_REL_COND_WIN_KEEP_AND,    // 连续答对X题且评价 S A
    SKILL_REL_COND_WRONG_ONE,       // 打错1题
};

// 技能动画效果类型
enum eSkillAniEffType
{
    SKILL_ANI_EFF_NOR = 0,          // 直接效果(默认)
    SKILL_ANI_EFF_BOMB,             // 普攻
};


// 删除好友类型
enum eRemoveFriendType
{
	// 常规删除好友
	REMOVE_FRIEND_NORMAL = 0,
	// 拉入黑名单删除好友
	REMOVE_FRIEND_BACKLIST = 1,
};

// 邮件标题类型
enum eMailTitleType
{
	// 文本类型
	MAIL_TITLE_TEXT,
	// 编码数据, 客户端通过配置表ID 读取文本
	MAIL_TITLE_CODE,
};

// 邮件内容类型
enum eMailContentType
{
	// 文本类型
	MAIL_CONTENT_TEXT,
	// json 格式编码数据
	MAIL_CONTENT_JSON_CODE,
};

// 邮件内容类型
enum eMailAttachmentType
{
	// 文本类型
	MAIL_ATTACHMENT_NORMAL,
	// json 格式[物品ID|物品数量]数据
	MAIL_ATTACHMENT_JSON_ITEMS,
};

// 物件状态 通用
enum eItemStatusType
{
	ITEM_STATUS_NIL,
	ITEM_STATUS_UNUSE,  	    // 未使用(未解锁)
	ITTEM_STATUS_USED,      	// 已使用(解锁)
};

enum eTiTleAndHeadInfoType
{
    TITLE_HEAD_INFO_NIL,
    TITLE_HEAD_INFO_TITLE,      // 称号
    TITLE_HEAD_INFO_HEAD,       // 头像
    TITLE_HEAD_INFO_HEAD_FRAME, // 头像框
};

// 激励视频奖励类型
enum eAdvIncRewardType
{
	// 未确定类型
	ADV_INC_REWARD_NIL,
	// 奖励物品-对应reward表的id
	ADV_INC_REWARD_CFG,
	// 奖励复活
	ADV_INC_REWARD_RELIVE,
	// 奖励小游戏题目答案（读取answer表）
	ADV_INC_REWARD_ANSWER,
};

// 任务进度类型
enum eTaskScheduleType
{
    TASK_SCHEDULE_NIL,
    TASK_SCHEDULE_CARD,         // 集卡进度
};

// 任务进度状态
enum eScheduleStatusType
{
    SCH_STATUS_NIL,
    SCH_STATUS_INCOMPLETE,      // 未完成
    SCH_STATUS_COMPLETED,       // 已完成    
};

// 集卡系统卡片类型
enum eCardType
{
    CARD_TYPE_NIL,
    CARD_TYPE_FPEPOLE,          // 名人卡
    CARD_TYPE_DATE,             // 日期卡
    CARD_TYPE_FUNSCIENCE,       // 趣味科普卡   
    CARD_TYPE_COMMONSENCE,      // 常识卡
    CARD_TYPE_OTHER,            // 日积月累卡 what ?xxk,,
    CARD_TYPE_ANY = 10,         // 任何类型
};

// 奖励类型(reward table)
enum eRewardType
{
    REWARD_TYPE_NIL,
    REWARD_TYPE_DAY_CHECKIN,            // 每日签到
    REWARD_TYPE_CML_CHECKIN,            // 累计签到   
    REWARD_TYPE_LEVEL_GIFT,             // 等级签到   
    REWARD_TYPE_ACTIVE_REWARD = 4,      // 活跃

    REWARD_TYPE_ONLINE = 101,           // 在线时长
    REWARD_TYPE_COMP_CARNIVAL = 122,    // 7日狂欢完成度

    REWARD_TYPE_LEVEL_PLAYER = 140,     // 个人等级排名 前三

    REWARD_TYPE_CARD_SCHEDUAL = 201,    // 集卡系统进度
};

enum ePrologueProcessType
{
    PRO_PROCESS_TYPE_NIL,
    PRO_PROCESS_TYPE_INIT,              // 开始
    PRO_PROCESS_TYPE_COMPL,             // 完成
};

// 功能解锁对应的功能类型
enum eFunctionType {
	FUNCTION_QUEST_LIST = 1,
};

// 任务版类型
enum eQuestListType {
	QUEST_LIST_ONE = 1,
	QUEST_LIST_TWO,
	QUEST_LIST_THREE,
	QUEST_LIST_FOUR,
};


#endif  /* NODEDEFINES_H */

