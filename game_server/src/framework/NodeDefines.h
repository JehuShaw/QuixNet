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
#define LOGGING_MODEL_NAME "logging_module"
#define PLAYER_MODEL_NAME "player_module"

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
#define APPCONFIG_LOGGINGCACHEID "LoggingCacheID"
////////////////////////////////////////////////

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

// ��������
enum eQuestConditionType {
    QUEST_CONDITION_NIL,
    QUEST_CONDITION_PVE,                  // �����ͨPVE����
    QUEST_CONDITION_DIFFPVE,              // ��ɾ�ӢPVE����
    QUEST_CONDITION_WARRIORS,             // �����˫����
    QUEST_CONDITION_FIEFTRADE,            // ��ɷ��ó��
    QUEST_CONDITION_FIEFMINE,             // ��ɷ�زɿ�
    QUEST_CONDITION_ACTIVITYPVE,          // ��ɻ����
    QUEST_CONDITION_SIZE,
};
//////////////////////////////////////////////////////////////////////////



#endif  /*_NODEDEFINES_H*/

