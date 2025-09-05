/* 
 * File:   LoggingModule.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */
#include "LoggingModule.h"
#include "NodeDefines.h"
#include "Log.h"
#include "SeparatedStream.h"
#include "CacheOperateHelper.h"
#include "GuidFactory.h"

#define SPN_MAX_LOG_TABLE_SIZE "game_log_max_table_num"
#define SPN_SHOW_LOG_TABLES "game_log_show_tables"
#define SPN_CREATE_LOG_TABLE "game_log_create_table"
#define SPN_INSERT_LOG "game_log_insert"


using namespace mdl;
using namespace util;

CLoggingModule::CLoggingModule() 
	: CModule(LOGGING_MODEL_NAME), m_nServerId(0) 
{
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	m_sepaTableStore.SetRawPointer(new CSepaTableStore(
		pConfig->GetInt(APPCONFIG_LOGGINGCACHEID)));
	m_nServerId = pConfig->GetInt(APPCONFIG_SERVERID);
}

void CLoggingModule::OnRegister() {
    OutputBasic("OnRegister");
}

void CLoggingModule::OnRemove(){
    OutputBasic("OnRemove");
}

std::vector<int> CLoggingModule::ListNotificationInterests()
{
	return std::vector<int>({
		N_CMD_NODE_REGISTER
	});
}

IModule::InterestList CLoggingModule::ListProtocolInterests()
{
	InterestList interests;
	return interests;
}

void CLoggingModule::HandleNotification(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
    int32_t nCmd = request->GetName();
    switch(nCmd) {
    case N_CMD_NODE_REGISTER:
        if((uint32_t)request->GetType() == m_sepaTableStore->GetCacheId()) {
			long nMaxTableSize = CSepaTableStore::LoadMaxTableSize(
				m_sepaTableStore->GetCacheId(), SPN_MAX_LOG_TABLE_SIZE);	
			m_sepaTableStore->LoadTables(nMaxTableSize, SPN_SHOW_LOG_TABLES);
        }
        break;
	default:
		break;
    }
}

bool CLoggingModule::Trace(uint64_t nUserId, uint64_t nAccount, int nLogType, int nActionType, int nSubType,
	int64_t nParam0 /*= 0*/, int64_t nParam1/*= 0*/, int64_t nParam2/*= 0*/, int64_t nParam3/*= 0*/, int64_t nParam4/*= 0*/,
	int64_t nParam5/*= 0*/, int64_t nParam6/*= 0*/, int64_t nParam7/*= 0*/, int64_t nParam8/*= 0*/, int64_t nParam9/*= 0*/,
	const char* szParam0/*= NULL*/, const char* szParam1/*= NULL*/, const char* szParam2/*= NULL*/, const char* szParam3/*= NULL*/)
{
	if(m_sepaTableStore.IsInvalid()) {
		return false;
	}

	// uint64_t nId = CGuidFactory::Pointer()->CreateGuid();
	util::CSeparatedStream stream(RECORD_SEPARATOR_CHAR);
	stream
	//	<< nId
		<< nAccount
		<< nLogType
		<< nActionType
		<< nSubType
		<< m_nServerId
		<< nUserId
		<< nParam0
		<< nParam1
		<< nParam2
		<< nParam3
		<< nParam4
		<< nParam5
		<< nParam6
		<< nParam7
		<< nParam8
		<< nParam9;
		if(NULL != szParam0) {
			stream << szParam0;
		} else {
			stream << "";
		}
		if(NULL != szParam1) {
			stream << szParam1;
		} else {
			stream << "";
		}
		if(NULL != szParam2) {
			stream << szParam2;
		} else {
			stream << "";
		}
		if(NULL != szParam3) {
			stream << szParam3;
		} else {
			stream << "";
		}
	stream.EndLine();

	std::string strContext(stream.Str());

	return m_sepaTableStore->CheckAndStore(SPN_CREATE_LOG_TABLE, SPN_INSERT_LOG,
		nUserId, strContext.c_str(), strContext.length());
}
