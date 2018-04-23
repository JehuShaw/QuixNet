/* 
 * File:   LoggingOperateHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef _LOGGINGOPERATEHELPER_H
#define _LOGGINGOPERATEHELPER_H
#include "NodeDefines.h"
#include "LoggingModule.h"

INLINE bool TraceBehavior(uint64_t nUserId, uint64_t nAccount, const std::string& strUserName, eLogBehaviorType nActionType, 
	int64_t nParam0 = 0, int64_t nParam1 = 0, int64_t nParam2 = 0, int64_t nParam3 = 0, int64_t nParam4 = 0,
	int64_t nParam5 = 0, int64_t nParam6 = 0, int64_t nParam7 = 0, int64_t nParam8 = 0, int64_t nParam9 = 0,
	const char* szParam1 = NULL, const char* szParam2 = NULL, const char* szParam3 = NULL) 
{
	CLoggingModule::PTR_T pLoggingModule(CLoggingModule::Pointer());
	return pLoggingModule->Trace(nUserId, nAccount, LOG_TYPE_BEHAVIOR, nActionType,
		0, nParam0, nParam1, nParam2, nParam3, nParam4, nParam5, nParam6,
		nParam7, nParam8, nParam9, strUserName.c_str(), szParam1, szParam2, szParam3);
}

INLINE bool TraceBudget(uint64_t nUserId, uint64_t nAccount, eLogBudgetType nActionType, 
	eObjType nObjType, int32_t nChangeCount, int32_t nLeftCount, int64_t nParam3 = 0, int64_t nParam4 = 0,
	int64_t nParam5 = 0, int64_t nParam6 = 0, int64_t nParam7 = 0, int64_t nParam8 = 0, int64_t nParam9 = 0,
	const char* szParam0 = NULL, const char* szParam1 = NULL, const char* szParam2 = NULL, const char* szParam3 = NULL) 
{
	CLoggingModule::PTR_T pLoggingModule(CLoggingModule::Pointer());
	return pLoggingModule->Trace(nUserId, nAccount, LOG_TYPE_BUDGET, nActionType,
		0, nObjType, nChangeCount, nLeftCount, nParam3, nParam4, nParam5, nParam6,
		nParam7, nParam8, nParam9, szParam0, szParam1, szParam2, szParam3);
}

#endif /* _LOGGINGOPERATEHELPER_H */

