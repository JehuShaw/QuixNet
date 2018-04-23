/* 
 * File:   LoggingModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef _LOGGINGMODULE_H
#define	_LOGGINGMODULE_H

#include "ModuleManager.h"
#include "Singleton.h"
#include "SepaTableStore.h"

class CLoggingModule
	: public mdl::CModule
	, public util::Singleton<CLoggingModule>
{
public:
	CLoggingModule();

	bool Trace(uint64_t nUserId, uint64_t nAccount, int nLogType, int nActionType, int nSubType,
		int64_t nParam0 = 0, int64_t nParam1 = 0, int64_t nParam2 = 0, int64_t nParam3 = 0, int64_t nParam4 = 0,
		int64_t nParam5 = 0, int64_t nParam6 = 0, int64_t nParam7 = 0, int64_t nParam8 = 0, int64_t nParam9 = 0,
		const char* szParam0 = NULL, const char* szParam1 = NULL, const char* szParam2 = NULL, const char* szParam3 = NULL);

private:
	virtual void OnRegister();

	virtual void OnRemove();

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual void HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

private:
    util::CAutoPointer<CSepaTableStore> m_sepaTableStore;
	int m_nServerId;
};

#endif	/* _LOGGINGMODULE_H */

