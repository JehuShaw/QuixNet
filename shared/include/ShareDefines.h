/* 
 * File:   defines.h
 * Author: Jehu Shaw
 *
 * Created on 2011_1_12 AM 8:11
 */

#ifndef _DEFINES_H
#define	_DEFINES_H

#include <string>
#include <map>
#include "Common.h"
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

#define GLOBAL_CACHE_USERID 0

#define DEFAULT_USERID -1

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
#define NODE_USER_CHECK_KEY "node_user_check"
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
};

//--------------------------------------
//  WorkerService return result
//--------------------------------------
enum eWorkerServiceResult {
    WORKER_RESULT_NIL,
};

enum eMailSenderType {
    MAIL_SENDER_USER,
    MAIL_SENDER_GM,
};

enum eMailReceiverType {
    MAIL_RECEIVER_SPECIAL_USER,
    MAIL_RECEIVER_GLOBAL_USER,
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
	N_CMD_SEND_TO_CLIENT = 17,
	N_CMD_BROADCAST_TO_CLIENT = 18,
	N_CMD_CLOSE_CLIENT = 19,
	N_CMD_CLOSE_ALLCLIENT = 20,
	N_CMD_SEND_TO_WORKER = 21,
	N_CMD_KICK_LOGGED = 22,
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
	N_CMD_MASTER_RESTART,
	N_CMD_MASTER_SHUTDOWN,
	N_CMD_MASTER_ERASE,
	N_CMD_SERVER_PLAY,
	N_CMD_SERVER_STOP,
	N_CMD_SERVER_STORE,
	N_CMD_END_TRANSACTION,
	N_CMD_REMOVE_BALSERVID,
    /////////////////////////////////
    N_CMD_C_RANK_UPDATE,
    N_CMD_C_RANK_REQUEST,
    N_CMD_SEND_MAIL,
};

//--------------------------------------
//  Protocol CMD definition
//--------------------------------------
// C is Client, L is Login Server, A is Agent Server, T is 'To'

enum eProtocolCMD
{
    P_CMD_CTL_LOGIN = 0x00000001,
    P_CMD_C_LOGIN,
    P_CMD_S_LOGOUT,
	P_CMD_C_PLAYER_RENAME,
    P_CMD_C_SEND_MAIL,
    P_CMD_C_RECEIVE_MAIL,
    P_CMD_S_NOTIFY_MAIL,
    P_CMD_C_ENTER_MAIL,
    P_CMD_C_FETCH_MAIL_AFFIX,
};



#endif	/* _DEFINES_H */

