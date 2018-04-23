/*
 * File:   NodeDefines.h
 * Author: Jehu Shaw
 *
 */

// Configuration Header File

#ifndef _NODEDEFINES_H
#define _NODEDEFINES_H

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

#define CALL_DEADLINE_MS 6000

#define CALL_UNREGISTER_MS 8000

#define TIMEOUT_MAX_TIMES_REMOVE_CHANNEL 100

// 3 second
#define KEEP_REGISTER_INTERVAL 30

// 3 keep register times
#define KEEP_REGISTER_TIMEOUT (KEEP_REGISTER_INTERVAL*3)

// check CWorkerModule db delay time (0.1 second)
#define WORKERMODULE_DELAY_TIME 40


// csv file define
const static char csv_field_terminator = ',';
const static char csv_line_terminator  = '\n';
const static char csv_enclosure_char   = '"';

//--------------------------------------
//  Module name Define
//--------------------------------------
#define CONTROLCENTRE_MODULE_NAME "ctrlcentre_module"
#define RANK_MODEL_NAME "rank_module"

//--------------------------------------
//  AppConfig key name Define
//--------------------------------------
#define APPCONFIG_PROJECTNAME "ProjectName"
#define APPCONFIG_SERVERID "ServerID"
#define APPCONFIG_SERVERNAME "ServerName"
#define APPCONFIG_SERVERREGION "ServerRegion"
#define APPCONFIG_SERVERBIND "ServerBind"
#define APPCONFIG_SERVANTCONNECT "ServantConnect"
#define APPCONFIG_SERVERCONNECT "ServerConnect"
#define APPCONFIG_RPCZTHREADS "RpczThreads"
#define APPCONFIG_GAMETHREADS "GameThreads"
#define APPCONFIG_ZMQIOTHREADS "ZmqioThreads"
#define APPCONFIG_TEMPLATEPATH "TemplatePath"
#define APPCONFIG_RANKCACHEID "RankCacheID"
#define APPCONFIG_FLOORLIMIT "FloorLimit"
#define APPCONFIG_MAXSIZELIMIT "MaxSizeLimit"


// attribute
inline static long AttributeFormula1(long nBase, long nLevel, double nGrow) {
    return (long)((double)nBase + (double)(nLevel - 1) * nGrow);
}

// atkspeed attribute
inline static long AtkSpeedFormula(long nBase, long nRate) {
    return -(nRate * nBase / 100);
}

// attribute
inline static long AttributePctFormula(long nBase, long nPct) {
    return nBase + long(nBase * nPct / 100);
}

inline static uint64_t CombinedId64(uint32_t low, uint32_t high) {
	id64_t id64(low, high);
	return id64.u64;
}

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

// 任务类型
enum eQuestConditionType {
    QUEST_CONDITION_NIL,
    QUEST_CONDITION_PVE,                  // 完成普通PVE副本
    QUEST_CONDITION_DIFFPVE,              // 完成精英PVE副本
    QUEST_CONDITION_WARRIORS,             // 完成无双副本
    QUEST_CONDITION_FIEFTRADE,            // 完成封地贸易
    QUEST_CONDITION_FIEFMINE,             // 完成封地采矿
    QUEST_CONDITION_ACTIVITYPVE,          // 完成活动副本
    QUEST_CONDITION_SIZE,
};
//////////////////////////////////////////////////////////////////////////

enum ePlayerStatus
{
    PLAYER_STATUS_OFFLINE,
    PLAYER_STATUS_ONLINE,
};


enum eRequestRankType {
    REQUEST_RANK_PRIVATE,       // 玩家自己的排名
    REQUEST_RANK_TOP,           // 排行版的前几名
	REQUEST_RANK_AROUND,		// 获得玩家自己和上下玩家排名
};

#define GENERAL_SKILL_FIRST_SLOT 0
#define GENERAL_SKILL_SECOND_SLOT 1
#define GENERAL_SKILL_THIRD_SLOT 2
#define GENERAL_SKILL_SLOT_SIZE 3

//////////////////////////////////////////////////////////////////////////
#define MIN_STAGE_ID_PAD 1
#define MAX_STAGE_ID_PAD 99
#define GetCodeStageID(preId, padId) ((preId) * 100 + (padId))
//////////////////////////////////////////////////////////////////////////
#define LOAD_EACH_PAGE_SIZE 10000
#define RANK_MAX_SIZE 10000
#define REQEST_RANK_TOP_SIZE 100
#define REQEST_RANK_AROUND_SIZE 5

#endif  /*_NODEDEFINES_H*/

