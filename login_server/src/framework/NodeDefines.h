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

#define TIMEOUT_MAX_TIMES_REMOVE_CHANNEL 50

// 3 second
#define KEEP_REGISTER_INTERVAL 30

// 3 keep register times
#define KEEP_REGISTER_TIMEOUT (KEEP_REGISTER_INTERVAL*3)

// check CWorkerModule db delay time (0.1 second)
#define WORKERMODULE_DELAY_TIME 40


// login expiry, 30 second
#define LOGIN_EXPIRY_TIME 300

#define AUTH_KEY "qmsj2035^@@!@$"

//--------------------------------------
//  Notification Names & Types
//--------------------------------------
#define CONTROLCENTRE_MODULE_NAME "ctrlcentre_module"
#define LOGIN_MODULE_NAME "login_module"

//--------------------------------------
//  AppConfig key name Define
//--------------------------------------
#define APPCONFIG_PROJECTNAME "ProjectName"
#define APPCONFIG_SERVERID "ServerID"
#define APPCONFIG_SERVERNAME "ServerName"
#define APPCONFIG_SERVERREGION "ServerRegion"
#define APPCONFIG_SERVERBIND "ServerBind"
#define APPCONFIG_SERVERACCEPT "ServerAccept"
#define APPCONFIG_ACCEPTMAXLINK "AcceptMaxLink"
#define APPCONFIG_ACCEPTPACKETLIMIT "AcceptPacketLimit"
#define APPCONFIG_SERVANTCONNECT "ServantConnect"
#define APPCONFIG_SERVERCONNECT "ServerConnect"
#define APPCONFIG_RPCZTHREADS "RpczThreads"
#define APPCONFIG_GAMETHREADS "GameThreads"
#define APPCONFIG_ZMQIOTHREADS "ZmqioThreads"
#define APPCONFIG_LOGINCHECKWEB "LoginCheckWeb"
#define APPCONFIG_PROXYSERVERNAME "ProxyServerName"
//////////////////////////////////////////////////

#endif  /*_NODEDEFINES_H*/

