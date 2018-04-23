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

extern volatile long g_serverStatus;

#define CALL_INFINITE_WAITING -1

#define CALL_DEADLINE_MS 6000

#define CALL_UNREGISTER_MS 10000

#define TIMEOUT_MAX_TIMES_REMOVE_CHANNEL 50

// 3 second
#define KEEP_REGISTER_INTERVAL 30

// login expiry, 30 second
#define LOGIN_EXPIRY_TIME 300

#define AUTH_KEY "qmsj2035^@@!@$"


//--------------------------------------
//  Notification Names & Types
//--------------------------------------
#define LOGIN_MODULE_NAME "login_module"

//--------------------------------------
//  AppConfig key name Define
//--------------------------------------
#define APPCONFIG_EQUIPID "EquipId"
#define APPCONFIG_CONNECT "Connect"
#define APPCONFIG_ACCEPTPACKETLIMIT "AcceptPacketLimit"
#define APPCONFIG_VERSION "Version"

#define APPCONFIG_GAMETHREADS "GameThreads"
#define APPCONFIG_LOGINWEB "LoginWeb"
//////////////////////////////////////////////////

enum WebLoginType {
	WEB_LOGIN_ANONYMOUS = 1,
	WEB_LOGIN_USERANDPWD = 2,
};

#endif  /*_NODEDEFINES_H*/

