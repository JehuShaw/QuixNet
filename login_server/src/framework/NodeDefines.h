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

#define TIMEOUT_MAX_TIMES_REMOVE_CHANNEL 50

#define TIMEOUT_MAX_TIMES_REMOVE_SERVER 100

// 3 second
#define KEEP_REGISTER_INTERVAL 300

// 3 keep register times
#define KEEP_REGISTER_TIMEOUT (KEEP_REGISTER_INTERVAL*3)

// login expiry, 30 second
#define LOGIN_EXPIRY_TIME 3000

#define AUTH_KEY "#XueYou#!&^!!"

#define RELOGIN_TIMEOUT 60

#define MAX_CHARARER_SIZE 2

// csv file define
const static char csv_field_terminator = ',';
const static char csv_line_terminator = '\n';
const static char csv_enclosure_char = '"';

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
#define APPCONFIG_ENDPOINT "EndPoint"
#define APPCONFIG_SERVERACCEPT "ServerAccept"
#define APPCONFIG_ACCEPTMAXLINK "AcceptMaxLink"
#define APPCONFIG_ACCEPTPACKETLIMIT "AcceptPacketLimit"
#define APPCONFIG_SERVANTCONNECT "ServantConnect"
#define APPCONFIG_SERVERCONNECT "ServerConnect"
#define APPCONFIG_RPCZTHREADS "RpczThreads"
#define APPCONFIG_GAMETHREADS "GameThreads"
#define APPCONFIG_ZMQIOTHREADS "ZmqioThreads"
#define APPCONFIG_LOGINCHECKWEB "LoginCheckWeb"
#define APPCONFIG_AGENTSERVERNAME "AgentServerName"
#define APPCONFIG_TEMPLATEPATH "TemplatePath"
//////////////////////////////////////////////////

#endif  /* NODEDEFINES_H */

