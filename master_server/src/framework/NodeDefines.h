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
// If any node crash, do you want auto restart it ?
extern volatile bool g_bAutoRestart;

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


// auto play expiry, 180 second
#define AUTO_PLAY_EXPIRY_TIME 1800

// login expiry, 30 second
#define LOGIN_EXPIRY_TIME 300

#define AUTH_KEY "DianClan!&^!!"

// memory cache key
#define NODE_STATE_REGISTER "node_state_register"
#define NODE_STATE_REFRESH "node_state_refresh"
#define NODE_STATE_UNREGISTER "node_state_unregister"
#define NODE_ROUTE_CREATE "node_route_create"
#define NODE_ROUTE_DROP "node_route_drop"
#define NODE_ROUTE_INSERT "node_route_insert"
#define NODE_ROUTE_REMOVE "node_route_remove"
#define NODE_ROUTE_GET "node_route_get"
#define NODE_USER_LOGIN "node_user_login"


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
#define APPCONFIG_SERVERACCEPT "ServerAccept"
#define APPCONFIG_SERVERCONNECT "ServerConnect"
#define APPCONFIG_RPCZTHREADS "RpczThreads"
#define APPCONFIG_GAMETHREADS "GameThreads"
#define APPCONFIG_ZMQIOTHREADS "ZmqioThreads"
#define APPCONFIG_LOGINCHECKWEB "LoginCheckWeb"
/////////////////////////////////////////////////

#endif  /*_NODEDEFINES_H*/

