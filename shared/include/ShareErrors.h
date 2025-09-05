/*
 * File:   Errors.h
 * Author: Jehu Shaw
 *
 */

#ifndef SERVER_ERRORS_H
#define SERVER_ERRORS_H

#include "PrintStackTrace.h"
// TODO: handle errors better

// An assert isn't necessarily fatal, but we want to stop anyways
#define WPAssert( EXPR ) if (!(EXPR)) { arcAssertFailed(__FILE__, __LINE__, #EXPR); assert(EXPR); ((void(*)())0)(); }

#define WPError( assertion, errmsg ) if( ! (assertion) ) { PrintError("%s:%i ERROR:\n  %s\n", __FILE__, __LINE__, (char *)errmsg); assert(#assertion &&0); }
#define WPWarning( assertion, errmsg ) if( ! (assertion) ) { PrintError("%s:%i WARNING:\n  %s\n", __FILE__, __LINE__, (char *)errmsg); }

// This should always halt everything.  If you ever find yourself wanting to remove the assert( false ), switch to WPWarning or WPError
#define WPFatal( assertion, errmsg ) if( ! (assertion) ) { PrintError("%s:%i FATAL ERROR:\n  %s\n", __FILE__, __LINE__, (char *)errmsg); assert(#assertion &&0); abort(); }

#define ASSERT WPAssert

#define NEGATIVE_SIGNS -1

enum eServerError {
	CACHE_ERROR_POINTER_NULL = -2147483647,
	CACHE_ERROR_RECORD_EXISTS,
	CACHE_ERROR_DATABASE_INVALID,
	CACHE_ERROR_EMPTY_DATA,
	CACHE_ERROR_EMPTY_TABLE,
	CACHE_ERROR_EMPTY_KEY,
	CACHE_ERROR_EMPTY_VALUES,
	CACHE_ERROR_NOTFOUND,
	CACHE_ERROR_CMD_UNKNOWN,
	CACHE_ERROR_EMPTY_ROUTE,
	CACHE_ERROR_PARSE_REQUEST,
	CACHE_ERROR_SERIALIZE_REQUEST,
	CACHE_ERROR_ROUTE_TYPE,
	SERVER_ERROR_ALREADY_EXIST,
	SERVER_ERROR_SERVER_STOP,
	SERVER_ERROR_NOTFOUND_CHANNEL,
	SERVER_ERROR_SERIALIZE,
	SERVER_ERROR_NOTFOUND_USER,
	SERVER_ERROR_NOTFOUND_CHARACTER,
	// 消息报无法解析
	PARSE_PACKAGE_FAIL,
	// 路由ID 爲空
	ROUTE_ID_NULL,
	// 找不到玩家实例
	CANNT_FIND_PLAYER,
	// 最大角色数量
	MAX_CHARARER_LIMIT,
	// MD5 验证失败
	AGENT_MD5_CHECK_FAIL,
	//////////////////////////////////
	// 下面的位置填写业务层错误定义
	// 在配置表没有找到数据
	CONFIG_NOT_FOUND,
	// 任务已经接过了
	QUEST_ALREADY_TAKEN,
	// 添加任务失败
	QUEST_ADD_FAIL,
	// 更新任务失败
	QUEST_UPDATE_FAIL,
	// 获取任务失败
	QUEST_LOAD_FAIL,
	// 获取任务失败
	QUEST_GETS_FAIL,
	// 任务未接取
	QUEST_NOT_TAKEN,
	// 任务数据没找到
	QUEST_NOT_FOUND,
	// 任务已经完成了
	QUEST_ALREADY_COMPLETED,
	// 任务不是正在进行的状态
	QUEST_NO_TAKEN_STATUE,
	// 任务不是已完成未提交状态
	QUEST_NO_FINISHED_STATUE,
	// 不可以放弃的任务
	QUEST_DONNT_ABORT,
	// 背包满了
	BACKPACK_IS_FULL,
	// 格子位置不正确
	BACKPACK_POS_INCORRECT,
	// 不是同类型的物品
	BACKPACK_NOT_SAME_TYPE,
	// 角色名已经存在
	PLAYER_NAME_ALREADY_EXIST,
	// 没有足够的物品
	NOT_ENOUGH_ITEM,
	// 更新任务事件失败
	QUEST_EVENT_UPDATE_FAIL,
	// 角色名长度限制
	CHARACTER_NAME_LIMIT,
	// 没有找到物品实例
	CANNT_FIND_ITEM,
	// 物品不是获得的时候的限时类型
	NO_ITEM_CREATE_TIME_TYPE,
	// 物品还没过期
	ITEM_NOT_EXPIRED,
	// 物品不可以出售
	ITEM_CANNT_SELL,
	// 物品没有到使用期限内
	ITEM_NOT_TIME_TO_USE,
	// 客户端传给服务端的参数不正确
	CLIENT_PARAM_FAIL,
	// 对话未完成
	QUEST_DIALOG_NOT_COMPLETE,
	// 任务配置ID 无效
	QUEST_CFG_ID_INVALID,
	// 任务目标配置错误
	QUEST_CFG_TARGET_INVALID,
	// 任务目标配置的对话ID 和客户端提交的不匹配
	QUEST_DIALOG_CFG_ID_INVALID,

	PET_MODULE_INVALID,
	// 宠物已经获取
	PET_ALREADY_TAKEN,
	// 宠物添加失败
	PET_ADD_FAIL,
	// 宠物获取失败
	PET_GETS_FAIL,
	// 宠物配置表无效
	PET_PLAYER_DATA_INVALID,
	// 宠物类型已存在
	PET_TYPE_EXIST,
	// 宠物不存在
	CANNT_FIND_PET,
	// 宠物配置表ID错误
	PET_TYPE_INVALID,
	// 宠物配置表错误
	PET_TYPE_CONFIG,
	// 宠物进化上限
	PET_TYPE_TOP_LEVEL,
	// 宠物数量已达上限
	PET_NUM_MAX,
	// 宠物经验值无效
	PET_EXP_INVALID,
	// 宠物升级失败（无错误）
	PET_UPGRADE_FAIL,
	// 宠物进化失败（无错误,条件未满足）
	PET_EVOLVE_FAIL,
	// 宠物进化配置不符
	PET_EVOLVE_CFGID,

	// 获取触发器失败
	TRIGGER_GETS_FAIL,
	// 更新触发器失败
	TRIGGER_UPDATE_FAIL,

	// 找不到地图实例
	CANNT_FIND_MAP,
	// 找不到副本
	CANNT_FIND_INSTANCE,

	// 宠物装备配置表没有找到
	PET_EQUIP_CFG_CANNT_FIND,
	// 宠物装备创建失败
	PET_EQUIP_CREATE_FAIL,
	// 找不到宠物装备实例
	PET_EQUIP_CANNT_FIND,
	// 找不到地图数据
	CANNT_FIND_WORLD,
	// 进入线失败
	ENTER_WORLD_FAIL,
	// 离开线失败
	LEAVE_WORLD_FAIL,
	// 没办法识别切换地图的类型
	UNKNOW_SWITCH_MAP_TYPE,
	// 已经在该地图
	ALREADY_ON_THE_MAP,
	// 无解锁地图
	PLAYER_UNLOCK_MAP,
	// 传送点不是这张地图的
	NO_THIS_MAP_TRANSPORT,
	// 未解锁，不允许进入改地图
	PLAYER_NOT_ALLOW_ENTER_MAP,
	// 线已满
	WORLD_IS_FULL,
	// 换装部件数异常
	PLAYER_DRESS_ITEM_COUNT,
	// 装扮部件不存在
	PLAYER_DRESS_ITEM_NOT_EXIST,
	// 装扮部件已过期
	PLAYER_DRESS_ITEM_EXPIRED,
	// 装扮部件未过期 
	PLAYER_DRESS_ITEM_NOT_EXPIRED,
	// 装扮部件配置表没找到
	PLAYER_DRESS_CFG_CANNT_FIND,
	// 装扮部件配置表创建条件为空
	PLAYER_DRESS_CFG_CREATE,
	// 装扮部件创建失败
	PLAYER_DRESS_ITEM_CREATE_FAIL,
	// 玩家不在同一线
	PLAYER_NOT_IN_SAME_LINE,
	// 装扮数目不对或部位不全
	PLAYER_DEF_DRESS_ITEM_COUNT,
	// 装扮缓存加载异常
	PLAYER_DRESS_CACHE_RECORD,
	// 装扮配置表异常
	PLAYER_DRESS_CFG_ERR,
	// 装扮部件部位异常
	PLAYER_DRESS_PART,
    // 装扮性别不符
    PLAYER_DRESS_GENDER,
	// 好友不可以申请加自己为好友
	FRIEND_CANNT_APPLY_SELF,
    // 私聊自己
    PRIVATE_CHAT_SELF,
	// 不可以添加自己为好友
	FRIEND_CANNT_ADD_SELF,
	// 不可以删除自己为好友
	FRIEND_CANNT_REMOVE_SELF,
	// 在黑名单中
	IS_ON_BLACKLIST,
	// 私聊目标ID 为空
	CHAT_PRIVATE_TARGETID_NULL,
	// 不可以和自己聊天
	CHAT_PRIVATE_CANNT_SELF,
	// 不是好友
	NOT_BE_FRIEND,
	// 到达好友最大数量
	MAX_FRIEND_SIZE_LIMIT,

    // 战斗配置表异常
    BATTLE_CONFIG_ERR,
    // 战斗副本波次找不到
    BATTLE_CONFIG_ROUND_CANNOT_FIND,
    // 战斗验证操作时间（小于极限值，判定作弊）
    BATTLE_VERIFY_TIME_INTERVAL,
    // 战斗验证宠物配置错误
    BATTLE_VERIFY_PET,
    // 战斗验证怪物配置错误
    BATTLE_VERIFY_MONSTER,
    // 战斗验证技能校验失败
    BATTLE_VERIFY_SKILL,
    // 战斗验证伤害校验失败
    BATTLE_VERIFY_HURT,
    // 战斗验证血条校验失败
    BATTLE_VERIFY_HP,

	// 无法识别的排行类型
	UNKNOW_RANK_TYPE,

	// 邮件接收者ID 为空
	MAIL_RECEIVERID_NULL,
	// 不可以发邮件给自己
	MAIL_CANNT_SEND_SELF,
	// 找不到对应的邮件
	MAIL_CANNT_FOUND,
	// 邮件附件已经领取过
	MAIL_ATTACHMENT_ALREADY_TAKEN,

    // 角色ID不匹配
    PLAYER_ID_NOT_MATCH,
    // 角色无效
    PLAYER_IS_INVALID,
    // 找不到对应的实例或配置，
    ITEM_CANNT_FOUND,
    // 称号未过期
    TITLE_NOT_EXPIRED,

	// 激励视频最大次数限制
	ADVERT_INCENT_MAX_COUNT_LIMIT,

    // 合成失败，卡片不全
    COMPOSE_CARD_FAIL,
    // 卡片已存在
    CARD_ALREADY_EXIST,
    // 找不到对应进度（任务）
    CANNT_FIND_SCHEDULE,
    // 进度已完成
    SCHEDULE_ALREADY_COMPLETED,
    // 进度未达成
    SCHEDULE_COMPLETE_COND,

    // 昵称非法字符
    CHARACTER_NAME_INVALID,

    // 序章设置失败
    PROLOGUE_PROCESS_FAIL,
	// 不是任务榜类型
	NO_QUEST_LIST_TYPE,

	// 远程调用超时
	SERVER_CALL_DEADLINE = -1,
	/////////////////////////////////
	// 未给错误信息的错误
	SERVER_ERROR_UNKNOW = 0,
	SERVER_FAILURE = 0,
	SERVER_SUCCESS = 1,
	SERVER_NO_RESPOND = 2,
	// 这个位置不添加值
};

#endif /* SERVER_ERRORS_H */

