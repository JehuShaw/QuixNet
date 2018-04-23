/*
 * File:   Errors.h
 * Author: Jehu Shaw
 *
 */

#ifndef _SERVER_ERRORS_H
#define _SERVER_ERRORS_H

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
	SERVER_ERROR_ALREADY_EXIST,
	SERVER_ERROR_SERVER_STOP,
	SERVER_ERROR_NOTFOUND_CHANNEL,
	SERVER_ERROR_SERIALIZE,
	// 消息报无法解析
	PARSE_PACKAGE_FAIL,
	// 服务端和客户端的配置版本不一致
	CONFIG_VERSION_NOT_CONSISTENT,
	// 找不到玩家实例
	CANNT_FIND_PLAYER,
	//////////////////////////////////
	// 下面的位置填写业务层错误定义


	/////////////////////////////////
	// 未给错误信息的错误
	SERVER_ERROR_UNKNOW = 0,
	SERVER_SUCCESS = 1,
	// 这个位置不添加值
};

#endif

