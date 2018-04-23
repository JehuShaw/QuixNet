/* 
 * File:   CacheMySqlAdpt.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_8_4, 16:00
 */

#pragma once

#ifndef _CACHEMYSQLADPT_H
#define	_CACHEMYSQLADPT_H

#include "Common.h"
#include "CacheDBType.h"

// 
#define SQL_SP_CONFIG_OPTIONS "CALL SP_SELECT_CONFIG_OPTIONS();"
#define SQL_SP_CONTAINERS "CALL SP_SELECT_CONTAINERS(\'%s\');"
#define SQL_SP_COLUMNS_TYPE "CALL SP_COLUMNS_TYPE(\'%s\',\'%s\');"
#define SQL_SP_DBSERVERS "CALL SP_SELECT_DBSERVERS(\'%s\');"

#define SQL_INSERT_ROUTE "INSERT INTO `%s`.`%s` (`%s`) VALUES ("I64FMTD");"

#define SQL_FIELD_START "`"
#define SQL_FIELD_END "`"
#define SQL_BEGIN_TRAN "BEGIN"
#define SQL_END_TRAN "COMMIT"
#define SQL_ROLLBACK "ROLLBACK"

/**
* If MS Sql Server 
* #define SQL_UPDATE_GETCAS_BEFORE_UPDATE "DECLARE @Output TABLE (cas int NULL);"
* #define SQL_UPDATE_GETCAS_IN_SET " OUTPUT inserted.mc_cas INTO @Output"
* #define SQL_UPDATE_GETCAS_IN_WHERE_BEFORE_CAS_FIELD ""
* #define SQL_UPDATE_GETCAS_IN_WHERE_AFTER_CAS_FIELD ""
* #define SQL_UPDATE_GETCAS_AFTER_UPDATE "SELECT TOP 1 cas FROM @Output;"
*/
#define SQL_UPDATE_GETCAS_BEFORE_UPDATE "SET @cas := NULL;"
#define SQL_UPDATE_GETCAS_IN_SET ""
#define SQL_UPDATE_GETCAS_IN_WHERE_BEFORE_CAS_FIELD "(SELECT @cas := "
#define SQL_UPDATE_GETCAS_IN_WHERE_AFTER_CAS_FIELD ")"
#define SQL_UPDATE_GETCAS_AFTER_UPDATE "SELECT @cas;"

#define SQL_PROCEDURE_CALL "CALL"
#define SQL_PROCEDURE_BEFORE_PARAMS "("
#define SQL_PROCEDURE_AFTER_PARAMS ");"

eDBType GetCacheDBTypeFromMySql(const std::string& strType);
#define GetCacheDBType GetCacheDBTypeFromMySql

#endif /* _CACHEMYSQLADPT_H */



