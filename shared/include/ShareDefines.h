/* 
 * File:   defines.h
 * Author: Jehu Shaw
 *
 * Created on 2011_1_12 AM 8:11
 */

#ifndef DEFINES_H
#define	DEFINES_H

#include <string>
#include <map>
#include "ControlCMD.h"

#define APP_CONFIG_NAME "App.config"

#define KEEPALIVE_MAX_TEST_TIMES 3
#define TIME_BUFFER_SIZE 32
#define MAX_NUMBER_SIZE 65
#define MAX_NANE_SIZE 256
#define MAX_BUF_SIZE 1024
#define FALSE       0
#define TRUE        1
#define ID_NULL 0

// 2014-1-1 00:00:00
#define START_FILETIME (1391184000)

#define ID_TYPE_32BIT_MASK 0xFFFFFFFF
#define ID_TYPE_32BIT 32
#define ID_TYPE_3BIT_MASK 0x7
#define ID_TYPE_29BIT_MASK 0x1FFFFFFF
#define ID_TYPE_29BIT 29


enum eIdType
{
	ID_TYPE_USER = 0,
	ID_TYPE_SERVERKEY = 1,
	ID_TYPE_LOCAL = 2,
	ID_TYPE_GLOBAL = 3,
};


enum eBUF_SIZE_TYPE
{
    eBUF_SIZE_4              = 4,
    eBUF_SIZE_8              = 8,
    eBUF_SIZE_16             = 16,
    eBUF_SIZE_32             = 32,
    eBUF_SIZE_64             = 64,
    eBUF_SIZE_128            = 128,
    eBUF_SIZE_256            = 256,
    eBUF_SIZE_512            = 512,
    eBUF_SIZE_1024           = 1024,
    eBUF_SIZE_1024_2         = 1024 * 2,
    eBUF_SIZE_1024_4         = 1024 * 4,
    eBUF_SIZE_1024_8         = 1024 * 8,
    eBUF_SIZE_1024_10        = 1024 * 10,
    eBUF_SIZE_1024_16        = 1024 * 16,
    eBUF_SIZE_1024_20        = 1024 * 20,
    eBUF_SIZE_1024_30        = 1024 * 30,
    eBUF_SIZE_1024_50        = 1024 * 50,
    eBUF_SIZE_1024_64        = 1024 * 64,
    eBUF_SIZE_1024_80        = 1024 * 80,
    eBUF_SIZE_1024_100       = 1024 * 100,
    eBUF_SIZE_1024_160       = 1024 * 160,
    eBUF_SIZE_1024_200       = 1024 * 200,
    eBUF_SIZE_1024_300       = 1024 * 300,
    eBUF_SIZE_1024_320       = 1024 * 320,
    eBUF_SIZE_1024_500       = 1024 * 500,
    eBUF_SIZE_1024_1024      = 1024 * 1024,
}; 

#define SRand(s) srand((unsigned)s + (unsigned int)time(0))

// [min, max)
#define Rand(min,max) (rand()%(max - min) + min)

// [0, 1)
#define RandDecimal() ((double)rand()/(double)(RAND_MAX + 1.0))

#define IsFixedRegion(n32Region) ((n32Region & 0x10000) != 0)

#define AddFixedRegionFlag(n32Region) (n32Region | 0x10000)

/**********************Game Config****************************/

// system userid
#define SYSTEM_USERID 1

// system chat keep time (second)
#define CHAT_SYSTEM_DURATION (int)30*24*3600
// system chat keep record size
#define CHAT_SYSTEM_KEEP_SIZE 500


#define GLOBAL_CACHE_USERID 0

#define DEFAULT_ACCOUNT 0

#define DEFAULT_USERID 0

#define DEFAULT_MAPID 0


enum ControlCentreRegisterType {
    REGISTER_TYPE_NIL,
    REGISTER_TYPE_WORKER,
    REGISTER_TYPE_CACHE,
    REGISTER_TYPE_SERVANT,
    REGISTER_TYPE_NODE,
	REGISTER_TYPE_MASTER,
};
//////////////////////////////////////////////////////////////////////////
#define NODE_CONTROL_STATE_KEY "node_control_state"
//////////////////////////////////////////////////////////////////////////

enum ServerStatusType {
    SERVER_STATUS_IDLE,
	SERVER_STATUS_START,
    SERVER_STATUS_STOP,
};


//--------------------------------------
//  Data packet route type
//--------------------------------------
enum eRouteType {
    ROUTE_BALANCE_USERID,
    ROUTE_BALANCE_SERVERID,
	ROUTE_DIRECT_SERVERID,
	ROUTE_BROADCAST_USER,
	ROUTE_HASH_32KEY,
};

//--------------------------------------
//  CentreService return result
//--------------------------------------
enum eCentreServiceResult {
	CSR_SUCCESS_AND_START = 2,
	CSR_SUCCESS = 1,
	CSR_TIMEOUT = 0,
	CSR_FAIL = -1,
	CSR_WITHOUT_REGISTER_TYPE = -2,
	CSR_NO_ICHANNELCONTROL = -3,
	CSR_CHANNEL_ALREADY_EXIST = -4,
	CSR_WITHOUT_THIS_MODULE = -5,
	CSR_WITHOUT_THIS_CHANNEL = -6,
	CSR_REMOVE_CHANNEL_FAIL = -7,
	CSR_REGION_INCONFORMITY = -8,
	CSR_NOT_FOUND = -9,
};

//--------------------------------------
//  WorkerService return result
//--------------------------------------
enum eWorkerServiceResult {
    WORKER_RESULT_NIL,
};

enum eMailType {
	MAIL_TYPE_NORMAIL,
};

enum eMailSenderType {
    MAIL_SENDER_USER,
    MAIL_SENDER_GM,
};

enum eMailReceiverType {
    MAIL_RECEIVER_SPECIAL_USER,
    MAIL_RECEIVER_GLOBAL_USER,
};

// Rank type
enum eRankType
{
	RANK_TYPE_NIL,
	// 角色等级排行
	RANK_TYPE_CHAR_LV = 140,
	// 角色战力排行
	RANK_TYPE_CHAR_FC = 141,
	// 宠物战力排行
	RANK_TYPE_PET_FC = 142,
	// 宠物语文排行
	RANK_TYPE_PET_CH = 143,
	// 宠物数学排行
	RANK_TYPE_PET_MA = 144,
	// 宠物英语排行
	RANK_TYPE_PET_EN = 145,
	// 宠物科学排行
	RANK_TYPE_PET_SC =146,
};

//--------------------------------------
//  Log types
//--------------------------------------
enum eLogType {
	LOG_TYPE_NIL,
	// 玩家基本行为日志
	LOG_TYPE_BEHAVIOR,
	// 收支日志
	LOG_TYPE_BUDGET,
	// 新手引导
	LOG_TYPE_BEGINNERS_GUIDE,
	// 主线任务
	LOG_TYPE_MAIN_QUEST,
	// 日常任务
	LOG_TYPE_DAILY,
	// PVE副本
	LOG_TYPE_PVE,
	// 组织
	LOG_TYPE_ORGANIZE,
	// 排行
	LOG_TYPE_RANK,
};

enum eLogBehaviorType {
	LOG_BEHAVIOR_NIL,
	// 玩家登入
	LOG_BEHAVIOR_LOGIN,
	// 玩家登出
	LOG_BEHAVIOR_LOGOUT,
};

enum eLogBudgetType {
	LOG_BUDGET_NIL,
};

enum eObjType {
	LOG_OBJ_NIL,
	LOG_OBJ_GEM,
	LOG_OBJ_GOLD,
	LOG_OBJ_EXP,
};

enum eLoginReason {
	LOGIN_NORMAL = 0,
	LOGIN_SWITCH_MAP,
	LOGIN_RECOVER,
};

enum eLogoutReason {
	LOGOUT_OFFLINE = 0,
	LOGOUT_KICK,
	LOGOUT_CLOSEALL,
	LOGOUT_SWITCH_MAP,
	LOGOUT_LOGIN_TIMEOUT,
	LOGOUT_NOT_VALIDATED,
};

enum eStringMatchType {
	STRING_MATCH_MID,
	STRING_MATCH_LEFT,
	STRING_MATCH_RIGHT,
};

// 
#define NODE_CONTROLCMD_OFFSET C_CMD_CTM_SEND_MAIL

//--------------------------------------
//  notification name definition
//--------------------------------------
enum eNotificationCMD {
    N_CMD_ADD = 0,
    N_CMD_LOAD = 1,
    N_CMD_STORE = 2,
    N_CMD_GET = 3,
    N_CMD_SET = 4,
    N_CMD_GETS = 5,
    N_CMD_CAS = 6,
    N_CMD_DEL = 7,
	N_CMD_LOADALL = 8,
    N_CMD_DB_INSERT = 9,
    N_CMD_DB_SELECT = 10,
    N_CMD_DB_UPDATE = 11,
    N_CMD_DB_DELETE = 12,
    N_CMD_DB_SELECTALL = 13,
    N_CMD_DB_ESCAPESTRING = 14,
    N_CMD_DB_STOREDPROCEDURES = 15,
	N_CMD_DB_ASYNCSTOREDPROCEDURES = 16,
	N_CMD_DB_CHECK_GLOBAL_EXISTS = 17,
	N_CMD_DB_CHKEXIST_ESCAPESTRING = 18,
	N_CMD_SEND_TO_CLIENT = 19,
	N_CMD_BROADCAST_TO_CLIENT = 20,
	N_CMD_CLOSE_CLIENT = 21,
	N_CMD_CLOSE_ALLCLIENTS = 22,
	N_CMD_SEND_TO_WORKER = 23,
	N_CMD_KICK_LOGGED = 24,
	N_CMD_ALL_DB_STOREDPROCEDURES = 25,
	N_CMD_SEND_TO_PLAYER = 26,
	N_CMD_POST_TO_PLAYER = 27,
	N_CMD_CHECK_CREATE_CHARACTER,
	N_CMD_CREATE_CHARACTER,
	N_CMD_GET_CHARACTER,
	N_CMD_LOGIN_USER,
    N_CMD_LOGIN_CHECK_WEB_RESULT,
    N_CMD_NODE_REGISTER,
    N_CMD_NODE_REMOVE,
	N_CMD_NODE_KEEPTIMEOUT,
	N_CMD_SERVANT_NODE_REGISTER,
	N_CMD_SERVANT_NODE_REMOVE,
	N_CMD_SERVANT_NODE_KEEPTIMEOUT,
	N_CMD_MASTER_REGISTER_CACHE,
	N_CMD_MASTER_REMOVE_CACHE,
	N_CMD_MASTER_KEEPTIMEOUT_CACHE,
    N_CMD_MASTER_TRANSPORT,
	N_CMD_MASTER_TO_USER,
	N_CMD_MASTER_RESTART,
	N_CMD_MASTER_SHUTDOWN,
	N_CMD_MASTER_ERASE,
	N_CMD_SERVER_PREPARE,
	N_CMD_SERVER_PLAY,
	N_CMD_SERVER_STOP,
	N_CMD_SERVER_STORE,
	N_CMD_END_TRANSACTION,
	N_CMD_CHECK_SWITCH_MAPID,
	N_CMD_PRESET_TARGET_MAP,
    /////////////////////////////////
	N_CMD_PRIVATE_CHAT,
	N_CMD_APPLY_FRIENDS,
	N_CMD_RESULT_OF_APPLYING,
	N_CMD_REMOVE_FRIEND,

    N_CMD_RANK_UPDATE,
    N_CMD_RANK_REQUEST,
	N_CMD_RANK_REMOVE,
	N_CMD_RANK_REWARD,
    N_CMD_SEND_MAIL,
};

//--------------------------------------
//  Protocol CMD definition
//--------------------------------------
// C is Client, L is Login Server, A is Agent Server, T is 'To'

enum eProtocolCMD
{
  P_CMD_NIL = 0,
  P_CMD_CTL_LOGIN = 1,
  P_CMD_CTL_CREATE = 2,
  P_CMD_CTL_ENTER = 3,
  P_CMD_CTL_RELOGIN = 4,
  P_CMD_C_LOGIN = 5,
  P_CMD_S_LOGOUT = 6,
  P_CMD_C_SWITCH_NODE = 7,
  P_CMD_C_SWITCH_MAP = 8,
  P_CMD_C_CHEAT = 9,
  P_CMD_C_TIME = 10,
  P_CMD_C_KEEPALIVE = 11,
  P_CMD_C_TAKE_QUEST = 21,
  P_CMD_C_SUBMIT_QUEST = 22,
  P_CMD_C_ABORT_QUEST = 23,
  P_CMD_C_DONE_QUESTS = 24,
  P_CMD_S_UPDATE_QUEST = 25,
  P_CMD_C_UPDATE_QUEST = 26,
  P_CMD_S_ACCESSIBLE_QUEST = 27,
  P_CMD_S_AUTO_TAKE_QUEST = 28,
  P_CMD_C_DIALOG_COMPLETE = 29,
  P_CMD_C_ORGANIZE_BACKPACK = 31,
  P_CMD_C_USE_ITEM = 32,
  P_CMD_C_UNLOCK_BACKPACK = 33,
  P_CMD_S_CONSUME_ITEM = 34,
  P_CMD_C_REMOVE_TIMEOUT_ITEM = 35,
  P_CMD_C_SALE_ITEM = 36,
  P_CMD_C_PVE_BEGIN = 41,
  P_CMD_C_PVE_END = 42,
  P_CMD_S_PVE_BEGIN = 43,
  P_CMD_C_COLLECTION = 44,
  P_CMD_C_PET_REQUEST = 51,
  P_CMD_C_PET_UPGRADE_REQUEST = 52,
  P_CMD_C_PET_EVOLVE_REQUEST = 53,
  P_CMD_C_PET_TAKE_REQUEST = 54,
  P_CMD_C_PET_FREE_REQUEST = 55,
  P_CMD_C_PET_GET_BATTLE_REQUEST = 56,
  P_CMD_C_PET_SET_BATTLE_REQUEST = 57,
  P_CMD_S_PET_UPGRADE = 58,
  P_CMD_C_PET_CREATE_EQUIP = 59,
  P_CMD_C_PET_UPGRADE_EQUIP = 60,
  P_CMD_S_PET_REWARD = 61,
  P_CMD_C_PLAYER_SYNC_POS = 70,
  P_CMD_C_WORLD_LIST = 71,
  P_CMD_C_ENTER_WORLD = 72,
  P_CMD_C_MAP_STATUS = 73,
  P_CMD_S_MAP_UPDATE = 74,
  P_CMD_C_DRESS_ITEMS = 76,
  P_CMD_C_DRESS_CHANGE = 77,
  P_CMD_C_DRESS_EXPIRED = 78,
  P_CMD_C_DRESS_ACTIVE = 79,
  P_CMD_C_PLATYER_DRESS = 80,
  P_CMD_C_CHAT = 83,
  P_CMD_S_CHAT = 84,
  P_CMD_C_GET_SYS_CHATS = 85,
  P_CMD_C_GET_PRI_CHATS = 86,
  P_CMD_C_GET_PRI_LIST = 87,
  P_CMD_C_DEL_PRI_CHATS = 88,
  P_CMD_C_DEL_ALL_PRI_CHATS = 89,
  P_CMD_C_FRIEND_INFO = 90,
  P_CMD_C_RAND_PLAYER_LIST = 91,
  P_CMD_C_SEARCH_PLAYER = 92,
  P_CMD_C_APPLY_BE_FRIEND = 93,
  P_CMD_S_APPLY_BE_FRIEND = 94,
  P_CMD_C_RESULT_BE_FRIEND = 95,
  P_CMD_S_RESULT_BE_FRIEND = 96,
  P_CMD_C_CLEAR_APPLING = 97,
  P_CMD_C_NOTE = 98,
  P_CMD_C_TOP = 99,
  P_CMD_C_CANCEL_TOP = 100,
  P_CMD_C_ADD_BLACKLIST = 101,
  P_CMD_C_REMOVE_BLACKLIST = 102,
  P_CMD_C_REMOVE_FRIEND = 103,
  P_CMD_S_REMOVE_FRIEND = 104,
  P_CMD_C_BUY_SHOPITEM = 110,
  P_CMD_C_GET_RANK = 115,
  P_CMD_S_ADVERT_INCENT = 117,
  P_CMD_C_SEND_MAIL = 118,
  P_CMD_S_SEND_MAIL = 119,
  P_CMD_C_GET_MAIL_LIST = 120,
  P_CMD_C_GET_MAIL_DETAIL = 121,
  P_CMD_C_GET_MAIL_ATTACHMENT = 122,
  P_CMD_C_DEL_MAIL = 123,
  P_CMD_C_DEL_ALL_MAIL = 124,
  P_CMD_C_PLAYER_INFO = 125,
  P_CMD_C_ALTER_PLAYER_NICK = 126,
  P_CMD_C_ALTER_PLAYER_HEAD = 127,
  P_CMD_C_ALTER_PLAYER_HEADFRAME = 128,
  P_CMD_C_ALTER_PLAYER_TITLE = 129,
  P_CMD_C_PLAYER_UNLOAD_TITLE = 130,
  P_CMD_C_PLAYER_EXPIRED_TITLE = 131,
  P_CMD_S_HEAD_REWARD = 132,
  P_CMD_S_HEADFRAME_REWARD = 133,
  P_CMD_S_TITLE_REWARD = 134,
  P_CMD_C_CARD_INFO = 136,
  P_CMD_C_CARD_SCHEDULE = 137,
  P_CMD_C_CARD_COMPOSE = 138,
  P_CMD_C_CARD_SCHEDULE_COMPL = 139,
  P_CMD_S_CARD_PIECE = 140,
  P_CMD_C_PROLOGUE_PROCESS = 141,
  P_CMD_C_QUEST_LIST_TAKE = 145,
  P_CMD_C_QUEST_LIST_INFO = 146,
  P_CMD_C_QUEST_LIST_REFRESH = 147,
};

#endif	/* DEFINES_H */

