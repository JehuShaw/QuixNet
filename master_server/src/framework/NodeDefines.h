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
// If any node crash, do you want auto restart it ?
extern volatile bool g_bAutoRestart;

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

// auto play expiry, 180 second
#define AUTO_PLAY_EXPIRY_TIME 18000

// Time interval between preparation and play, 0.3 second
#define PREPARE_PLAY_INTERVAL 30

// login expiry, 30 second
#define LOGIN_EXPIRY_TIME 300

// 10 second
#define AUTH_TIMESTAMP_SPAN 10

// memory cache key
#define NODE_STATE_REGISTER "node_state_register"
#define NODE_STATE_REFRESH "node_state_refresh"
#define NODE_STATE_UNREGISTER "node_state_unregister"
//#define NODE_ROUTE_CREATE "node_route_create"
//#define NODE_ROUTE_DROP "node_route_drop"
//#define NODE_ROUTE_INSERT "node_route_insert"
//#define NODE_ROUTE_REMOVE "node_route_remove"
//#define NODE_ROUTE_GET "node_route_get"
#define NODE_USER_GET "node_user_get"
#define NODE_USER_CREATE "node_user_create"
#define NODE_USER_CHECK "node_user_check"
#define NODE_USER_UPDATE "node_user_update"
#define NODE_USER_DELETE "node_user_delete"
#define NODE_SEIZE_SERVER "node_seize_server"
#define NODE_FREE_SERVER "node_free_server"
#define NODE_ADMIN_CHECK "node_admin_check"


//--------------------------------------
//  Module name Define
//--------------------------------------
#define MASTER_MODULE_PERFIX "Master"

//--------------------------------------
//  AppConfig key name Define
//--------------------------------------
#define APPCONFIG_PROJECTNAME "ProjectName"
#define APPCONFIG_SERVERID "ServerID"
#define APPCONFIG_SERVERNAME "ServerName"
#define APPCONFIG_SERVERBIND "ServerBind"
#define APPCONFIG_ENDPOINT "EndPoint"
#define APPCONFIG_SERVERACCEPT "ServerAccept"
#define APPCONFIG_ACCEPTMAXLINK "AcceptMaxLink"
#define APPCONFIG_ACCEPTPACKETLIMIT "AcceptPacketLimit"
#define APPCONFIG_SERVERCONNECT "ServerConnect"
#define APPCONFIG_RPCZTHREADS "RpczThreads"
#define APPCONFIG_GAMETHREADS "GameThreads"
#define APPCONFIG_ZMQIOTHREADS "ZmqioThreads"
#define APPCONFIG_AGENTSERVERNAME "AgentServerName"
/////////////////////////////////////////////////

// BKDR Hash Function
static unsigned int BKDRHash(const char *str)
{
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;

	while (*str)
	{
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}

#endif  /* NODEDEFINES_H */

