/*
 * File:   CacheOperate.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef CACHEOPERATE_H
#define CACHEOPERATE_H

#include "ModuleManager.h"
#include "cache.pb.h"
#include "cache.rpcz.h"
#include "BodyMessage.h"
#include "NodeDefines.h"
#include "ICacheValue.h"
#include "Log.h"
#include "ChannelManager.h"
#include "SmallBuffer.h"
#include "BitSignSet.h"
#include "TransferStream.h"
#include "MCResult.h"
#include "ValueStream.h"


// array string delimit
#define ARRAY_SEPARATOR ","
#define ARRAY_SEPARATOR_CHAR ','
// memcached value field delimit
#define RECORD_SEPARATOR "|"
#define RECORD_SEPARATOR_CHAR '|'
typedef std::vector<std::string> RECORD_PARAMS_T;

typedef ::node::CacheRequest mc_request_t;
typedef ::node::CacheResponse mc_response_t;
typedef ::node::MCRequest mc_record_t;
typedef ::google::protobuf::RepeatedPtrField< ::node::MCRequest > mc_record_set_t;
// record set operator
#define RecordSetAdd(mcRecordSetPtr) mcRecordSetPtr->Add()
// set record operator
#define SetRecord(mcRequest) mcRequest.add_values()
#define SetRecordKey(mcRecordPtr, key) mcRecordPtr->set_key(key)
#define SetRecordNKey(mcRecordPtr, key, size) mcRecordPtr->set_key(key, size)
#define SetRecordValue(mcRecordPtr, value) mcRecordPtr->set_value(value)
#define SetRecordNValue(mcRecordPtr, value, size) mcRecordPtr->set_value(value, size)
#define SetRecordFlags(mcRecordPtr, flags) mcRecordPtr->set_flags(flags)
#define SetRecordExpiry(mcRecordPtr, expiry) mcRecordPtr->set_expiry(expiry)
#define SetRecordResult(mcRecordPtr, result) mcRecordPtr->set_result(result)
#define SetRecordNoReply(mcRecordPtr) mcRecordPtr->set_result(MCERR_NOREPLY)
#define SetRecordCas(mcRecordPtr, cas) mcRecordPtr->set_cas(cas)

// get record operator
#define GetRecordSize(mcResponse) mcResponse.values_size()
#define GetRecord(mcResponse, index) mcResponse.values(index)
#define GetRecordKey(mcRecordRef) mcRecordRef.key()
#define GetRecordValue(mcRecordRef) mcRecordRef.value()
#define GetRecordFlags(mcRecordRef) mcRecordRef.flags()
#define GetRecordExpiry(mcRecordRef) mcRecordRef.expiry()
#define GetRecordResult(mcRecordRef) mcRecordRef.result()
#define GetRecordCas(mcRecordRef) mcRecordRef.cas()

// check record operator
#define HasRecordKey(mcRecordRef) mcRecordRef.has_key()
#define HasRecordValue(mcRecordRef) mcRecordRef.has_value()
#define HasRecordFlags(mcRecordRef) mcRecordRef.has_flags()
#define HasRecordExpiry(mcRecordRef) mcRecordRef.has_expiry()
#define HasRecordResult(mcRecordRef) mcRecordRef.has_result()
#define HasRecordCas(mcRecordRef) mcRecordRef.has_cas()

// reset record operator
#define ResetRecordValue(mcRecordRef, value) const_cast<mc_record_t&>(mcRecordRef).set_value(value)


// get result
#define GetCacheResult(mcResponse) mcResponse.result()

#define GetCacheRecordResult(mcResponse, index)\
CCacheOperate::GetRecordResultByIdx(mcResponse, index)

#define GetCacheFirstRecordResult(mcResponse)\
CCacheOperate::GetRecordResultByIdx(mcResponse, 0)

//////////////////////////////////////////////////////////////////////////
// operator by direct serverId
#define McGetForUpdateDirServId(serverId, getsRequest, getsResponse, casResponse)\
for(int i = 0; i < 6; ++i) {\
    if(i > 0) { getsResponse.Clear(); }\
    CCacheOperate::HandleGets(ROUTE_DIRECT_SERVERID, serverId, getsRequest, getsResponse)


#define McUpdateDirServId(serverId, getsRequest, casResponse)\
    if(!CCacheOperate::HandleCAS(ROUTE_DIRECT_SERVERID, serverId, getsRequest, casResponse)) {\
        continue;\
    }\
    break;\
}

#define McAddDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_ADD, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McGetDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_GET, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McSetDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_SET, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McGetsDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_GETS, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McCasDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_CAS, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McDelDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DEL, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McLoadDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_LOAD, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McStoreDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_STORE, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McLoadAllDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleMultiRows(N_CMD_LOADALL, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McDBInsertDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_INSERT, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McDBSelectDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_SELECT, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McDBUpdateDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_UPDATE, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McDBDeleteDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_DELETE, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McDBSelectAllDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleMultiRows(N_CMD_DB_SELECTALL, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McDBStoredProcDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleStoredProcs(N_CMD_DB_STOREDPROCEDURES, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McDBAsyncStoredProcDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleStoredProcs(N_CMD_DB_ASYNCSTOREDPROCEDURES, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

#define McDBEscapeStringDirServId(serverId, inEscape, outEscape)\
CCacheOperate::HandleEscapeString(ROUTE_DIRECT_SERVERID, serverId, inEscape, outEscape)

#define McAddOneRecordDirServId(serverId, strKey, inValue)\
CCacheOperate::AddOneRecord(ROUTE_DIRECT_SERVERID, serverId, strKey, inValue)

#define McDelOneRecordDirServId(serverId, strKey)\
CCacheOperate::DelOneRecord(ROUTE_DIRECT_SERVERID, serverId, strKey)

#define McCasOneRecordDirServId(serverId, strKey, inValue)\
CCacheOperate::CasOneRecord(ROUTE_DIRECT_SERVERID, serverId, strKey, inValue)

#define McGetsOneRecordDirServId(serverId, strKey, inValue)\
CCacheOperate::GetsOneRecord(ROUTE_DIRECT_SERVERID, serverId, strKey, inValue)

#define McLoadOneRecordDirServId(serverId, strKey, inValue)\
CCacheOperate::LoadOneRecord(ROUTE_DIRECT_SERVERID, serverId, strKey, inValue)

#define McStoreOneRecordDirServId(serverId, strKey, inValue)\
CCacheOperate::StoreOneRecord(ROUTE_DIRECT_SERVERID, serverId, strKey, inValue)


#define McInsertOneRecordToDBDirServId(serverId, strKey, inValue)\
CCacheOperate::InsertOneRecordToDB(ROUTE_DIRECT_SERVERID, serverId, strKey, inValue)

#define McSelectOneRecordFromDBDirServId(serverId, strKey, inValue)\
CCacheOperate::SelectOneRecordFromDB(ROUTE_DIRECT_SERVERID, serverId, strKey, inValue)

#define McUpdateOneRecordToDBDirServId(serverId, strKey, inValue)\
CCacheOperate::UpdateOneRecordToDB(ROUTE_DIRECT_SERVERID, serverId, strKey, inValue)

#define McDeleteOneRecordFromDBDirServId(serverId, strKey)\
CCacheOperate::DeleteOneRecordFromDB(ROUTE_DIRECT_SERVERID, serverId, strKey)

//////////////////////////////////////////////////////////////////////////
// operator by balance serverId
#define McGetForUpdateBalServId(serverId, getsRequest, getsResponse, casResponse)\
for(int i = 0; i < 6; ++i) {\
	if(i > 0) { getsResponse.Clear(); }\
	CCacheOperate::HandleGets(ROUTE_BALANCE_SERVERID, serverId, getsRequest, getsResponse)


#define McUpdateBalServId(serverId, getsRequest, casResponse)\
	if(!CCacheOperate::HandleCAS(ROUTE_BALANCE_SERVERID, serverId, getsRequest, casResponse)) {\
		continue;\
	}\
	break;\
}

#define McAddBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_ADD, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McGetBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_GET, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McSetBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_SET, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McGetsBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_GETS, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McCasBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_CAS, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McDelBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DEL, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McLoadBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_LOAD, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McStoreBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_STORE, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McLoadAllBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleMultiRows(N_CMD_LOADALL, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McDBInsertBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_INSERT, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McDBSelectBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_SELECT, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McDBUpdateBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_UPDATE, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McDBDeleteBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_DELETE, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McDBSelectAllBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleMultiRows(N_CMD_DB_SELECTALL, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McDBStoredProcBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleStoredProcs(N_CMD_DB_STOREDPROCEDURES, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McDBAsyncStoredProcBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleStoredProcs(N_CMD_DB_ASYNCSTOREDPROCEDURES, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

#define McDBEscapeStringBalServId(serverId, inEscape, outEscape)\
CCacheOperate::HandleEscapeString(ROUTE_BALANCE_SERVERID, serverId, inEscape, outEscape)

#define McAddOneRecordBalServId(serverId, strKey, inValue)\
CCacheOperate::AddOneRecord(ROUTE_BALANCE_SERVERID, serverId, strKey, inValue)

#define McDelOneRecordBalServId(serverId, strKey)\
CCacheOperate::DelOneRecord(ROUTE_BALANCE_SERVERID, serverId, strKey)

#define McCasOneRecordBalServId(serverId, strKey, inValue)\
CCacheOperate::CasOneRecord(ROUTE_BALANCE_SERVERID, serverId, strKey, inValue)

#define McGetsOneRecordBalServId(serverId, strKey, inValue)\
CCacheOperate::GetsOneRecord(ROUTE_BALANCE_SERVERID, serverId, strKey, inValue)

#define McLoadOneRecordBalServId(serverId, strKey, inValue)\
CCacheOperate::LoadOneRecord(ROUTE_BALANCE_SERVERID, serverId, strKey, inValue)

#define McStoreOneRecordBalServId(serverId, strKey, inValue)\
CCacheOperate::StoreOneRecord(ROUTE_BALANCE_SERVERID, serverId, strKey, inValue)


#define McInsertOneRecordToDBBalServId(serverId, strKey, inValue)\
CCacheOperate::InsertOneRecordToDB(ROUTE_BALANCE_SERVERID, serverId, strKey, inValue)

#define McSelectOneRecordFromDBBalServId(serverId, strKey, inValue)\
CCacheOperate::SelectOneRecordFromDB(ROUTE_BALANCE_SERVERID, serverId, strKey, inValue)

#define McUpdateOneRecordToDBBalServId(serverId, strKey, inValue)\
CCacheOperate::UpdateOneRecordToDB(ROUTE_BALANCE_SERVERID, serverId, strKey, inValue)

#define McDeleteOneRecordFromDBBalServId(serverId, strKey)\
CCacheOperate::DeleteOneRecordFromDB(ROUTE_BALANCE_SERVERID, serverId, strKey)

//////////////////////////////////////////////////////////////////////////
// operator by balance userId
#define McGetForUpdateBalUserId(userId, getsRequest, getsResponse, casResponse)\
for(int i = 0; i < 6; ++i) {\
    if(i > 0) { getsResponse.Clear(); }\
    CCacheOperate::HandleGets(ROUTE_BALANCE_USERID, userId, getsRequest, getsResponse)

#define McUpdateBalUserId(userId, getsRequest, casResponse)\
    if(!CCacheOperate::HandleCAS(ROUTE_BALANCE_USERID, userId, getsRequest, casResponse)) {\
        continue;\
    }\
    break;\
}

#define McAddBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_ADD, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McGetBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_GET, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McSetBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_SET, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McGetsBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_GETS, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McCasBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_CAS, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McDelBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DEL, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McLoadBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_LOAD, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McStoreBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_STORE, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McLoadAllBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleMultiRows(N_CMD_LOADALL, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McDBInsertBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_INSERT, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McDBSelectBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_SELECT, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McDBUpdateBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_UPDATE, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McDBDeleteBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_DELETE, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McDBSelectAllBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleMultiRows(N_CMD_DB_SELECTALL, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McDBStoredProcBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleStoredProcs(N_CMD_DB_STOREDPROCEDURES, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McDBAsyncStoredProcBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleStoredProcs(N_CMD_DB_ASYNCSTOREDPROCEDURES, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#define McDBEscapeStringBalUserId(userId, inEscape, outEscape)\
CCacheOperate::HandleEscapeString(ROUTE_BALANCE_USERID, userId, inEscape, outEscape)

#define McAddOneRecordBalUserId(userId, strKey, inValue)\
CCacheOperate::AddOneRecord(ROUTE_BALANCE_USERID, userId, strKey, inValue)

#define McDelOneRecordBalUserId(userId, strKey)\
CCacheOperate::DelOneRecord(ROUTE_BALANCE_USERID, userId, strKey)

#define McCasOneRecordBalUserId(userId, strKey, inValue)\
CCacheOperate::CasOneRecord(ROUTE_BALANCE_USERID, userId, strKey, inValue)

#define McGetsOneRecordBalUserId(userId, strKey, inValue)\
CCacheOperate::GetsOneRecord(ROUTE_BALANCE_USERID, userId, strKey, inValue)

#define McLoadOneRecordBalUserId(userId, strKey, inValue)\
CCacheOperate::LoadOneRecord(ROUTE_BALANCE_USERID, userId, strKey, inValue)

#define McStoreOneRecordBalUserId(userId, strKey, inValue)\
CCacheOperate::StoreOneRecord(ROUTE_BALANCE_USERID, userId, strKey, inValue)


#define McInsertOneRecordToDBBalUserId(userId, strKey, inValue)\
CCacheOperate::InsertOneRecordToDB(ROUTE_BALANCE_USERID, userId, strKey, inValue)

#define McSelectOneRecordFromDBBalUserId(userId, strKey, inValue)\
CCacheOperate::SelectOneRecordFromDB(ROUTE_BALANCE_USERID, userId, strKey, inValue)

#define McUpdateOneRecordToDBBalUserId(userId, strKey, inValue)\
CCacheOperate::UpdateOneRecordToDB(ROUTE_BALANCE_USERID, userId, strKey, inValue)

#define McDeleteOneRecordFromDBBalUserId(userId, strKey)\
CCacheOperate::DeleteOneRecordFromDB(ROUTE_BALANCE_USERID, userId, strKey)

#ifdef DEFAULT_ROUTE_BY_SERVERID
#define McGetForUpdate McGetForUpdateDirServId
#define McUpdate McUpdateDirServId
#define McAdd McAddDirServId
#define McGet McGetDirServId
#define McSet McSetDirServId
#define McGets McGetsDirServId
#define McCas McCasDirServId
#define McDel McDelDirServId
#define McLoad McLoadDirServId
#define McStore McStoreDirServId
#define McLoadAll McLoadAllDirServId
#define McDBInsert McDBInsertDirServId
#define McDBSelect McDBSelectDirServId
#define McDBUpdate McDBUpdateDirServId
#define McDBDelete McDBDeleteDirServId
#define McDBSelectAll McDBSelectAllDirServId
#define McDBEscapeString McDBEscapeStringDirServId
#define McAddOneRecord McAddOneRecordDirServId
#define McDelOneRecord McDelOneRecordDirServId
#define McCasOneRecord McCasOneRecordDirServId
#define McGetsOneRecord McGetsOneRecordDirServId
#define McLoadOneRecord McLoadOneRecordDirServId
#define McStoreOneRecord McStoreOneRecordDirServId
#define McInsertOneRecordToDB McInsertOneRecordToDBDirServId
#define McSelectOneRecordFromDB McSelectOneRecordFromDBDirServId
#define McUpdateOneRecordToDB McUpdateOneRecordToDBDirServId
#define McDeleteOneRecordFromDB McDeleteOneRecordFromDBDirServId
#else
#define McGetForUpdate McGetForUpdateBalUserId
#define McUpdate McUpdateBalUserId
#define McAdd McAddBalUserId
#define McGet McGetBalUserId
#define McSet McSetBalUserId
#define McGets McGetsBalUserId
#define McCas McCasBalUserId
#define McDel McDelBalUserId
#define McLoad McLoadBalUserId
#define McStore McStoreBalUserId
// load all route by server id
#define McLoadAll McLoadAllDirServId
#define McDBInsert McDBInsertBalUserId
#define McDBSelect McDBSelectBalUserId
#define McDBUpdate McDBUpdateBalUserId
#define McDBDelete McDBDeleteBalUserId
// select all route by server id
#define McDBSelectAll McDBSelectAllDirServId
#define McDBEscapeString McDBEscapeStringBalUserId
#define McAddOneRecord McAddOneRecordBalUserId
#define McDelOneRecord McDelOneRecordBalUserId
#define McCasOneRecord McCasOneRecordBalUserId
#define McGetsOneRecord McGetsOneRecordBalUserId
#define McLoadOneRecord McLoadOneRecordBalUserId
#define McStoreOneRecord McStoreOneRecordBalUserId
#define McInsertOneRecordToDB McInsertOneRecordToDBBalUserId
#define McSelectOneRecordFromDB McSelectOneRecordFromDBBalUserId
#define McUpdateOneRecordToDB McUpdateOneRecordToDBBalUserId
#define McDeleteOneRecordFromDB McDeleteOneRecordFromDBBalUserId
#endif

#define SendCacheProtocol CCacheOperate::HandleProtocol

#define SendCacheCmdToClient(userId, cmd)\
CCacheOperate::SendToClient(userId, cmd, __FILE__, __LINE__)

#define SendCacheToClient(userId, cmd, message)\
CCacheOperate::SendToClient(userId, cmd, message, __FILE__, __LINE__)

#define SendCachePacketToClient(dataPacket)\
CCacheOperate::SendToClient(dataPacket, __FILE__, __LINE__)

#define BroadcastCacheCmdToClient(cmd, excludeId)\
CCacheOperate::BroadcastToClient(cmd, excludeId, __FILE__, __LINE__)

#define BroadcastCacheToClient(cmd, message, excludeId)\
CCacheOperate::BroadcastToClient(cmd, message, excludeId, __FILE__, __LINE__)

#define BroadcastCachePacketToClient(dataPacket, excludeId)\
CCacheOperate::BroadcastToClient(dataPacket, excludeId, __FILE__, __LINE__)

#define CloseCacheClient(userId)\
CCacheOperate::CloseClient(userId, __FILE__, __LINE__)

#define CloseCacheAllClient()\
CCacheOperate::CloseAllClient(__FILE__, __LINE__)

#define SendCacheCmdToWorker(userId, cmd)\
CCacheOperate::SendToWorker(userId, cmd, __FILE__, __LINE__)

#define SendCacheToWorker(userId, cmd, message)\
CCacheOperate::SendToWorker(userId, cmd, message, __FILE__, __LINE__)

#define SendCachePacketToWorker(dataPacket)\
CCacheOperate::SendToWorker(dataPacket, __FILE__, __LINE__)

#define KickCacheLogged(userId)\
CCacheOperate::KickLogged(userId, __FILE__, __LINE__)

#define GetCacheRequestPacket(request)\
CCacheOperate::GetRequestPacket(request, __FILE__, __LINE__)

#define GetCacheResponsePacket(reply)\
CCacheOperate::GetResponsePacket(reply, __FILE__, __LINE__)

#define GetCachePlayer(request)\
CCacheOperate::GetPlayer(request, __FILE__, __LINE__)

#define GetCachePlayerSilence(request)\
CCacheOperate::GetPlayerSilence(request)

#define SerializeCacheData(outPacket, message)\
CCacheOperate::SerializeData(outPacket, message, __FILE__, __LINE__)

#define ParseCacheData(outMessage, packet)\
CCacheOperate::ParseData(outMessage, packet, __FILE__, __LINE__)

class CResponseRows {
public:
	inline int GetSize() const {
		return m_cacheResponse.values_size();
	}

	inline util::CValueStream GetKey(int index) const {
		return util::CValueStream(m_cacheResponse.values(index).key(), false);
	}

	inline util::CValueStream GetValue(int index) const {
		return util::CValueStream(m_cacheResponse.values(index).value(), false);
	}

	inline uint64_t GetCas(int index) const {
		return m_cacheResponse.values(index).cas();
	}

	inline MCResult GetResult(int index) const {
		return (MCResult)m_cacheResponse.values(index).result();
	}

	inline MCResult GetFirstRecordResult() const {
		if(0 < m_cacheResponse.values_size()) {
			return (MCResult)m_cacheResponse.values(0).result();
		}
		return MCERR_NOREPLY;
	}

	inline eServerError GetServResult() const {
		return (eServerError)m_cacheResponse.result();
	}

	inline mc_response_t& GetResponse() {
		return m_cacheResponse;
	}

	inline void Clear() {
		m_cacheResponse.Clear();
	}

private:
	mc_response_t m_cacheResponse;
};

class CRequestSnglRow {
public:
	inline mc_record_t* AddRecord() {
		return SetRecord(m_cacheRequest);
	}
	// The key contain "name" and "key_columns".
	inline static void SetKey(mc_record_t* pMcRecord, const util::CValueStream& key) {
		if(NULL == pMcRecord) {
			OutputError("NULL == pMcRecord");
			return;
		}
		SetRecordNKey(pMcRecord, key.GetData(), key.GetLength());
	}
	// The value only contain "value_columns" field which is the field of "containers".
	inline static void SetValue(mc_record_t* pMcRecord, const util::CValueStream& value) {
		if(NULL == pMcRecord) {
			OutputError("NULL == pMcRecord");
			return;
		}
		SetRecordNValue(pMcRecord, value.GetData(), value.GetLength());
	}

	inline static void SetCas(mc_record_t* pMcRecord, uint64_t cas) {
		if(NULL == pMcRecord) {
			OutputError("NULL == pMcRecord");
			return;
		}
		SetRecordCas(pMcRecord, cas);
	}

	inline mc_request_t& GetRequest() {
		return m_cacheRequest;
	}

private:
	mc_request_t m_cacheRequest;
};

class CRequestMultiRows {
public:
	// The key contain "name" and "key_columns".
	inline void SetKey(const util::CValueStream& key) {
		m_cacheRequest.set_key(key.GetData(), key.GetLength());
	}
	// The database row offset, start by zero
	inline void SetOffset(uint32_t nOffset) {
		m_cacheRequest.set_offset(nOffset);
	}
	// The database number of rows which you want.
	inline void SetCount(uint32_t nCount) {
		m_cacheRequest.set_count(nCount);
	}

	inline mc_request_t& GetRequest() {
		return m_cacheRequest;
	}

private:
	mc_request_t m_cacheRequest;
};

class CRequestStoredProcs {
public:
	// The key only contain "name" field which is the field of "containers".
	inline void SetKey(const util::CValueStream& key) {
		m_cacheRequest.set_key(key.GetData(), key.GetLength());
	}
	// The Stored Procedures parameters.
	inline void SetParams(const util::CValueStream& params) {
		m_cacheRequest.set_data(params.GetData(), params.GetLength());
	}
	// AsyncStoredProcedures use
	inline void SetNoReply(bool bNoReply) {
		if(bNoReply) {
			m_cacheRequest.set_result(MCERR_NOREPLY);
		} else {
			m_cacheRequest.clear_result();
		}
	}

	inline mc_request_t& GetRequest() {
		return m_cacheRequest;
	}

private:
	mc_request_t m_cacheRequest;
};

class CCacheOperate {
public:

	inline static int HandleNotification(const ::node::DataPacket& workerRequest,
		::node::DataPacket& workerResponse, int nType = 0) {

			util::CAutoPointer<::node::DataPacket> pDspRequest(&workerRequest, false);
			CBodyMessage requestBody(pDspRequest);
			util::CAutoPointer<CBodyMessage> pRequestBody(&requestBody, false);
			mdl::CNotification notification(workerRequest.cmd(), pRequestBody, nType);
			util::CAutoPointer<mdl::CNotification> pRequest(&notification, false);

			util::CAutoPointer<::node::DataPacket> pDspResponse(&workerResponse, false);
			CBodyMessage responseBody(pDspResponse);
			util::CAutoPointer<CBodyMessage> pResponseBody(&responseBody, false);
			mdl::CResponse mdlResponse(pResponseBody);
			util::CAutoPointer<mdl::CResponse> pReply(&mdlResponse, false);

			mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
			pFacade->NotifyObservers(pRequest, pReply, false);

			return pReply->GetResult();
	}

	static eServerError HandleCache(int cmd, eRouteType routeType, uint64_t route,
        mc_request_t& cacheRequest, mc_response_t& cacheResponse)
    {
		::node::DataPacket dpRequest;
		if(!SerializeCacheData(dpRequest, cacheRequest)) {
			return CACHE_ERROR_PARSE_REQUEST;
		}
		dpRequest.set_cmd(cmd);
		dpRequest.set_route_type(routeType);
		dpRequest.set_route(route);

		::node::DataPacket dpResponse;
		HandleNotification(dpRequest, dpResponse, 0);
		eServerError nResult = (eServerError)dpResponse.result();
		if(SERVER_SUCCESS == nResult) {
			ParseCacheData(cacheResponse, dpResponse);
		}
		return nResult;
	}

	INLINE static eServerError HandleSnglRow(int cmd, eRouteType routeType, uint64_t route,
		CRequestSnglRow& cacheRequest, CResponseRows& cacheResponse)
	{
		return HandleCache(cmd, routeType, route, cacheRequest.GetRequest(), cacheResponse.GetResponse());
	}

	INLINE static eServerError HandleMultiRows(int cmd, eRouteType routeType, uint64_t route,
		CRequestMultiRows& cacheRequest, CResponseRows& cacheResponse)
	{
		return HandleCache(cmd, routeType, route, cacheRequest.GetRequest(), cacheResponse.GetResponse());
	}

	INLINE static eServerError HandleStoredProcs(int cmd, eRouteType routeType, uint64_t route,
		CRequestStoredProcs& cacheRequest, CResponseRows& cacheResponse)
	{
		return HandleCache(cmd, routeType, route, cacheRequest.GetRequest(), cacheResponse.GetResponse());
	}

	static void HandleGets(eRouteType routeType, uint64_t route,
        CRequestSnglRow& cacheRequest, CResponseRows& cacheResponse)
	{
		mc_request_t& getsRequest = cacheRequest.GetRequest();
		mc_response_t& getsResponse = cacheResponse.GetResponse();

		HandleCache(N_CMD_GETS, routeType, route, getsRequest, getsResponse);

		int nSize = getsResponse.values_size() > getsRequest.values_size() ?
			getsRequest.values_size() : getsResponse.values_size();
		for(int j = 0; j < nSize; ++j) {
			const ::node::MCRequest& mcReply = getsResponse.values(j);
			::node::MCRequest& mcRequest = const_cast<::node::MCRequest&>(getsRequest.values(j));
			mcRequest.set_cas(mcReply.cas());
		}
	}

	static bool HandleCAS(eRouteType routeType, uint64_t route,
        CRequestSnglRow& cacheRequest, CResponseRows& cacheResponse)
	{
		mc_request_t& getsRequest = cacheRequest.GetRequest();
		mc_response_t& casResponse = cacheResponse.GetResponse();

		int nResponseSize = casResponse.values_size();
		if(nResponseSize > 0) {
			mc_request_t regetsRequest;

			int nRequestSize = getsRequest.values_size();
			for(int i = 0; i < nRequestSize; ++i) {
				const mc_record_t& mcGets = getsRequest.values(i);
				for(int j = 0; j < nResponseSize; ++j) {
					const mc_record_t& mcCas = casResponse.values(j);
					if(mcGets.key() == mcCas.key()
						&& mcGets.cas() != mcCas.cas()
						&& mcCas.result() == MCERR_EXISTS) {
							mc_record_t* pMCRequest = regetsRequest.add_values();
							assert(pMCRequest);
							pMCRequest->set_key(mcGets.key());
							pMCRequest->set_value(mcGets.value());
							pMCRequest->set_cas(mcGets.cas());
							if(mcGets.has_expiry()) {
								pMCRequest->set_expiry(mcGets.expiry());
							}
							if(mcGets.has_flags()) {
								pMCRequest->set_flags(mcGets.flags());
							}
							if(mcGets.has_result()) {
								pMCRequest->set_result(mcGets.result());
							}
					}
				}
			}
			casResponse.Clear();
			HandleCache(N_CMD_CAS, routeType, route, regetsRequest, casResponse);
		} else {
			HandleCache(N_CMD_CAS, routeType, route, getsRequest, casResponse);
		}

        std::string strKey;
        int nRequestSize = getsRequest.values_size();
        for(int i = 0; i < nRequestSize; ++i) {
            mc_record_t& mcGets = const_cast<mc_record_t&>(getsRequest.values(i));
            strKey = mcGets.key();
            mcGets.Clear();
            mcGets.set_key(strKey);
        }

		nResponseSize = casResponse.values_size();
		for(int j = 0; j < nResponseSize; ++j) {
			const mc_record_t& mcRequest = casResponse.values(j);
			if(mcRequest.result() == MCERR_EXISTS) {
				return false;
			}
		}
		return true;
	}

	static inline bool CheckCAS(util::CAutoPointer<mc_response_t>& cacheResponse){

		for(int j = 0; j < cacheResponse->values_size(); ++j) {
			const mc_record_t& mcRequest = cacheResponse->values(j);
			if(mcRequest.result() == MCERR_EXISTS) {
				return true;
			}
		}
		return false;
	}

    static inline MCResult GetRecordResultByIdx(const mc_response_t& mcResponse, int index) {
        if(index < mcResponse.values_size()) {
            const mc_record_t& mcRecordRespone = GetRecord(mcResponse, index);
            return (MCResult)GetRecordResult(mcRecordRespone);
        }
        return MCERR_NOREPLY;
    }

    //////////////////////////////////////////////////////////////////////////
    static inline MCResult AddOneRecord(eRouteType routeType, uint64_t route,
        const util::CTransferStream& strKeys, ICacheValue& inValue)
    {
        mc_request_t cacheAddRequest;
        mc_record_t* mcAddRecord = SetRecord(cacheAddRequest);
        SetRecordNKey(mcAddRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());
		util::CTransferStream strValues;
		util::BitSignSet oldBitSigns;
		uint8_t oldChgType = MCCHANGE_NIL;
		inValue.Serialize(strValues, oldBitSigns, oldChgType);
        SetRecordNValue(mcAddRecord, strValues.GetData(), strValues.GetNumberOfBytesUsed());

        mc_response_t cacheAddResponse;
        if(CCacheOperate::HandleCache(N_CMD_ADD, routeType, route,
            cacheAddRequest, cacheAddResponse) != SERVER_SUCCESS) {
			if(MCCHANGE_NIL != oldChgType) {
				inValue.RecoverChgType(oldBitSigns, oldChgType);
			}
			return MCERR_NOTSTORED;
		}
		MCResult nResult = (MCResult)GetCacheFirstRecordResult(cacheAddResponse);
		if(MCERR_OK != nResult && MCCHANGE_NIL != oldChgType) {
			inValue.RecoverChgType(oldBitSigns, oldChgType);
		}
        return nResult;
    }

    static inline MCResult LoadOneRecord(eRouteType routeType, uint64_t route,
        const util::CTransferStream& strKeys, ICacheValue& outValue)
    {
        mc_request_t cacheLoadRequest;
        mc_record_t* mcGetRecord = SetRecord(cacheLoadRequest);
        SetRecordNKey(mcGetRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());
        mc_response_t cacheLoadResponse;
        if(CCacheOperate::HandleCache(N_CMD_LOAD, routeType, route,
            cacheLoadRequest, cacheLoadResponse) != SERVER_SUCCESS) {
			return MCERR_NOREPLY;
		}
        MCResult nResult = GetCacheFirstRecordResult(cacheLoadResponse);
        if(MCERR_OK == nResult) {
            const mc_record_t& mcRecord = GetRecord(cacheLoadResponse, 0);
            outValue.Parse(GetRecordValue(mcRecord), GetRecordCas(mcRecord));
        }
        return nResult;
    }

    static inline MCResult StoreOneRecord(eRouteType routeType, uint64_t route,
        const util::CTransferStream& strKeys, ICacheValue& inValue)
    {
        mc_request_t cacheStoreRequest;
        mc_record_t* pMCRecord = SetRecord(cacheStoreRequest);
        SetRecordNKey(pMCRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());
		util::BitSignSet oldBitSigns;
		uint8_t oldChgType = MCCHANGE_NIL;
		if(inValue.ChangeType() != MCCHANGE_NIL) {
            SetRecordCas(pMCRecord, inValue.GetCas());
			util::CTransferStream strValues;
			inValue.Serialize(strValues, oldBitSigns, oldChgType);
            SetRecordNValue(pMCRecord, strValues.GetData(), strValues.GetNumberOfBytesUsed());
        }

        mc_response_t cacheStoreResponse;
		if(CCacheOperate::HandleCache(N_CMD_STORE, routeType, route,
            cacheStoreRequest, cacheStoreResponse) != SERVER_SUCCESS) {
			if(MCCHANGE_NIL != oldChgType) {
				inValue.RecoverChgType(oldBitSigns, oldChgType);
			}
			return MCERR_NOTSTORED;
		}
		MCResult nResult = (MCResult)GetCacheFirstRecordResult(cacheStoreResponse);
		if(MCERR_OK != nResult && MCCHANGE_NIL != oldChgType) {
			inValue.RecoverChgType(oldBitSigns, oldChgType);
		}
        return nResult;
    }

    static inline MCResult GetsOneRecord(eRouteType routeType, uint64_t route,
        const util::CTransferStream& strKeys, ICacheValue& outValue)
    {
        mc_request_t cacheGetRequest;
        mc_record_t* mcGetRecord = SetRecord(cacheGetRequest);
        SetRecordNKey(mcGetRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());
        mc_response_t cacheGetResponse;
        if(CCacheOperate::HandleCache(N_CMD_GETS, routeType, route,
            cacheGetRequest, cacheGetResponse) != SERVER_SUCCESS) {
			return MCERR_NOREPLY;
		}
        MCResult nResult = GetCacheFirstRecordResult(cacheGetResponse);
        if(MCERR_OK == nResult) {
            const mc_record_t& mcRecord = GetRecord(cacheGetResponse, 0);
            outValue.Parse(GetRecordValue(mcRecord), GetRecordCas(mcRecord));
        }
        return nResult;
    }

    static inline MCResult CasOneRecord(eRouteType routeType, uint64_t route,
        const util::CTransferStream& strKeys, ICacheValue& inValue)
    {
        mc_request_t cacheCasRequest;
        mc_record_t* pMCRecord = SetRecord(cacheCasRequest);
        SetRecordNKey(pMCRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());
        SetRecordCas(pMCRecord, inValue.GetCas());
		util::CTransferStream strValues;
		util::BitSignSet oldBitSigns;
		uint8_t oldChgType = MCCHANGE_NIL;
		inValue.Serialize(strValues, oldBitSigns, oldChgType);
        SetRecordNValue(pMCRecord, strValues.GetData(), strValues.GetNumberOfBytesUsed());
        mc_response_t cacheCasResponse;
        if(CCacheOperate::HandleCache(N_CMD_CAS, routeType, route,
            cacheCasRequest, cacheCasResponse) != SERVER_SUCCESS) {
			if(MCCHANGE_NIL != oldChgType){
				inValue.RecoverChgType(oldBitSigns, oldChgType);
			}
			return MCERR_NOTSTORED;
		}
		MCResult nResult = (MCResult)GetCacheFirstRecordResult(cacheCasResponse);
		if(MCERR_OK == nResult || MCERR_EXISTS == nResult) {
			const mc_record_t& mcRecord = GetRecord(cacheCasResponse, 0);
			inValue.SetCas(GetRecordCas(mcRecord));
			if(MCERR_EXISTS == nResult && MCCHANGE_NIL != oldChgType) {
				inValue.RecoverChgType(oldBitSigns, oldChgType);
			}
		} else if(MCCHANGE_NIL != oldChgType){
			inValue.RecoverChgType(oldBitSigns, oldChgType);
		}
        return nResult;
    }

    static inline MCResult DelOneRecord(eRouteType routeType, uint64_t route,
		const util::CTransferStream& strKeys)
    {
        mc_request_t cacheDelRequest;
        mc_record_t* pMCRecord = SetRecord(cacheDelRequest);
        SetRecordNKey(pMCRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());
        mc_response_t cacheDelResponse;
        if(CCacheOperate::HandleCache(N_CMD_DEL, routeType, route,
            cacheDelRequest, cacheDelResponse) != SERVER_SUCCESS) {
			return MCERR_NOTSTORED;
		}
        return GetCacheFirstRecordResult(cacheDelResponse);
    }

    static inline MCResult InsertOneRecordToDB(eRouteType routeType, uint64_t route,
        const util::CTransferStream& strKeys, ICacheValue& inValue)
    {
        mc_request_t dbInsertRequest;
        mc_record_t* mcAddRecord = SetRecord(dbInsertRequest);
        SetRecordNKey(mcAddRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());
		util::CTransferStream strValues;
		util::BitSignSet oldBitSigns;
		uint8_t oldChgType = MCCHANGE_NIL;
		inValue.Serialize(strValues, oldBitSigns, oldChgType);
        SetRecordNValue(mcAddRecord, strValues.GetData(), strValues.GetNumberOfBytesUsed());

        mc_response_t dbInsertResponse;
        if(CCacheOperate::HandleCache(N_CMD_DB_INSERT, routeType, route,
            dbInsertRequest, dbInsertResponse) != SERVER_SUCCESS) {
			if(MCCHANGE_NIL != oldChgType){
				inValue.RecoverChgType(oldBitSigns, oldChgType);
			}
			return MCERR_NOTSTORED;
		}
		MCResult nResult = (MCResult)GetCacheFirstRecordResult(dbInsertResponse);
		if(MCERR_OK != nResult && MCCHANGE_NIL != oldChgType) {
			inValue.RecoverChgType(oldBitSigns, oldChgType);
		}
        return nResult;
    }

    static inline MCResult SelectOneRecordFromDB(eRouteType routeType, uint64_t route,
        const util::CTransferStream& strKeys, ICacheValue& outValue)
    {
        mc_request_t dbSelectRequest;
        mc_record_t* mcGetRecord = SetRecord(dbSelectRequest);
        SetRecordNKey(mcGetRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());
        mc_response_t dbSelectResponse;
        if(CCacheOperate::HandleCache(N_CMD_DB_SELECT, routeType, route,
            dbSelectRequest, dbSelectResponse) != SERVER_SUCCESS) {
			return MCERR_NOREPLY;
		}
        MCResult nResult = GetCacheFirstRecordResult(dbSelectResponse);
        if(MCERR_OK == nResult) {
            const mc_record_t& mcRecord = GetRecord(dbSelectResponse, 0);
            outValue.Parse(GetRecordValue(mcRecord), GetRecordCas(mcRecord));
        }
        return nResult;
    }

    static inline MCResult UpdateOneRecordToDB(eRouteType routeType, uint64_t route,
        const util::CTransferStream& strKeys, ICacheValue& inValue)
    {
        mc_request_t dbUpdateRequest;
        mc_record_t* pMCRecord = SetRecord(dbUpdateRequest);
        SetRecordNKey(pMCRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());
        SetRecordCas(pMCRecord, inValue.GetCas());
		util::CTransferStream strValues;
		util::BitSignSet oldBitSigns;
		uint8_t oldChgType = MCCHANGE_NIL;
		inValue.Serialize(strValues, oldBitSigns, oldChgType);
        SetRecordNValue(pMCRecord, strValues.GetData(), strValues.GetNumberOfBytesUsed());
        mc_response_t dbUpdateResponse;
        if(CCacheOperate::HandleCache(N_CMD_DB_UPDATE, routeType, route,
            dbUpdateRequest, dbUpdateResponse) != SERVER_SUCCESS) {
			if(MCCHANGE_NIL != oldChgType){
				inValue.RecoverChgType(oldBitSigns, oldChgType);
			}
			return MCERR_NOTSTORED;
		}
		MCResult nResult = (MCResult)GetCacheFirstRecordResult(dbUpdateResponse);
		if(MCERR_OK != nResult && MCCHANGE_NIL != oldChgType) {
			inValue.RecoverChgType(oldBitSigns, oldChgType);
		}
        return nResult;
    }

    static inline MCResult DeleteOneRecordFromDB(eRouteType routeType, uint64_t route,
		const util::CTransferStream& strKeys)
    {
        mc_request_t dbDeleteRequest;
        mc_record_t* pMCRecord = SetRecord(dbDeleteRequest);
        SetRecordNKey(pMCRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());
        mc_response_t dbDeleteResponse;
        if(CCacheOperate::HandleCache(N_CMD_DB_DELETE, routeType, route,
            dbDeleteRequest, dbDeleteResponse) != SERVER_SUCCESS) {
			return MCERR_NOTSTORED;
		}
        return GetCacheFirstRecordResult(dbDeleteResponse);
    }

    inline static eServerError HandleEscapeString(eRouteType routeType,
		uint64_t route, const std::string& inEscape, std::string& outEscape)
    {
		ASSERT(routeType != ROUTE_BROADCAST_USER);

        ::node::DataPacket dpRequest;
		dpRequest.set_cmd(N_CMD_DB_ESCAPESTRING);
		dpRequest.set_route_type(routeType);
		dpRequest.set_route(route);
        dpRequest.set_data(inEscape);

        ::node::DataPacket dpResponse;
        CCacheOperate::HandleNotification(dpRequest, dpResponse);

		eServerError nResult = (eServerError)dpResponse.result();
		if(SERVER_SUCCESS == nResult) {
			outEscape = dpResponse.data();
		}
        return nResult;
    }

	inline static int HandleProtocol(const ::node::DataPacket& workerRequest,
		::node::DataPacket& workerResponse, int nType = 0) {

			util::CAutoPointer<::node::DataPacket> pDspRequest(&workerRequest, false);
			CBodyMessage requestBody(pDspRequest);
			util::CAutoPointer<CBodyMessage> pRequestBody(&requestBody, false);
			mdl::CNotification notification(workerRequest.cmd(), pRequestBody, nType);
			util::CAutoPointer<mdl::CNotification> pRequest(&notification, false);

			util::CAutoPointer<::node::DataPacket> pDspResponse(&workerResponse, false);
			CBodyMessage responseBody(pDspResponse);
			util::CAutoPointer<CBodyMessage> pResponseBody(&responseBody, false);
			mdl::CResponse mdlResponse(pResponseBody);
			util::CAutoPointer<mdl::CResponse> pReply(&mdlResponse, false);

			mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
			pFacade->NotifyProtocol(pRequest, pReply, false);

			return pReply->GetResult();
	}

	inline static int HandleProtocol(const ::node::DataPacket& workerRequest,
		::node::DataPacket& workerResponse, util::CWeakPointer<CPlayerBase> pPlayer, int nType = 0) {

			util::CAutoPointer<::node::DataPacket> pDspRequest(&workerRequest, false);
			CBodyMessage requestBody(pDspRequest);
			requestBody.SetPlayer(pPlayer);
			util::CAutoPointer<CBodyMessage> pRequestBody(&requestBody, false);
			mdl::CNotification notification(workerRequest.cmd(), pRequestBody, nType);
			util::CAutoPointer<mdl::CNotification> pRequest(&notification, false);

			util::CAutoPointer<::node::DataPacket> pDspResponse(&workerResponse, false);
			CBodyMessage responseBody(pDspResponse);
			util::CAutoPointer<CBodyMessage> pResponseBody(&responseBody, false);
			mdl::CResponse mdlResponse(pResponseBody);
			util::CAutoPointer<mdl::CResponse> pReply(&mdlResponse, false);

			mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
			pFacade->NotifyProtocol(pRequest, pReply, false);

			return pReply->GetResult();
	}

	inline static util::CWeakPointer<::node::DataPacket> GetRequestPacket(
		const util::CWeakPointer<mdl::INotification>& request,
		const char* file, long line)
	{
		if(request.IsInvalid()) {
			PrintError("file: %s line: %u @%s request.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<::node::DataPacket>();
		}

		util::CWeakPointer<CBodyMessage> pBodyMessage(request->GetBody());
		if(pBodyMessage.IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyMessage.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<::node::DataPacket>();
		}

		if(pBodyMessage->GetMessage().IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyMessage->getMessage().IsInvalid()", file, line, __FUNCTION__);
		}

		return pBodyMessage->GetMessage();
	}

	inline static util::CWeakPointer<::node::DataPacket> GetResponsePacket(
		const util::CWeakPointer<mdl::IResponse>& reply,
		const char* file, long line)
	{
		if(reply.IsInvalid()) {
			PrintError("file: %s line: %u @%s reply.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<::node::DataPacket>();
		}

		util::CWeakPointer<const CBodyMessage> pBodyReply(reply->GetBody());
		if(pBodyReply.IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyReply.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<::node::DataPacket>();
		}

		if(pBodyReply->GetMessage().IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyReply->getMessage().IsInvalid()", file, line, __FUNCTION__);
		}
		return pBodyReply->GetMessage();
	}

	inline static util::CWeakPointer<CPlayerBase> GetPlayer(
		const util::CWeakPointer<mdl::INotification>& request,
		const char* file, long line)
	{
		if(request.IsInvalid()) {
			PrintError("file: %s line: %u @%s request.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<CPlayerBase>();
		}

		util::CWeakPointer<const CBodyMessage> pBodyMessage(request->GetBody());
		if(pBodyMessage.IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyMessage.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<CPlayerBase>();
		}

		if(pBodyMessage->GetPlayer().IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyMessage->getMessage().IsInvalid()", file, line, __FUNCTION__);
		}

		return pBodyMessage->GetPlayer();
	}

	inline static util::CWeakPointer<CPlayerBase> GetPlayerSilence(
		const util::CWeakPointer<mdl::INotification>& request)
	{
		if(request.IsInvalid()) {
			return util::CWeakPointer<CPlayerBase>();
		}

		util::CWeakPointer<CBodyMessage> pBodyMessage(request->GetBody());
		if(pBodyMessage.IsInvalid()) {
			return util::CWeakPointer<CPlayerBase>();
		}

		return pBodyMessage->GetPlayer();
	}

	inline static bool SerializeData(util::CWeakPointer<::node::DataPacket>& pDataPacket,
		const ::google::protobuf::Message& message, const char* file, long line)
	{
		if(pDataPacket.IsInvalid()) {
			PrintError("file: %s line: %u @%s pDataPacket.IsInvalid()", file, line, __FUNCTION__);
			return false;
		}
		int nByteSize = message.ByteSize();
		ntwk::SmallBuffer smallbuf(nByteSize);
		if(!message.SerializeToArray((char*)smallbuf, nByteSize)) {
			PrintError("file: %s line: %u @%s !message.SerializeToArray", file, line, __FUNCTION__);
			return false;
		}
		pDataPacket->set_data((char*)smallbuf, nByteSize);
		return true;
	}

	inline static bool SerializeData(::node::DataPacket& outPacket,
		const ::google::protobuf::Message& message, const char* file, long line)
	{
		int nByteSize = message.ByteSize();
		ntwk::SmallBuffer smallbuf(nByteSize);
		if(!message.SerializeToArray((char*)smallbuf, nByteSize)) {
			PrintError("file: %s line: %u @%s !message.SerializeToArray", file, line, __FUNCTION__);
			return false;
		}
		outPacket.set_data((char*)smallbuf, nByteSize);
		return true;
	}

	inline static bool ParseData(::google::protobuf::Message& outMessage,
		const util::CWeakPointer<::node::DataPacket>& pDataPacket, const char* file, long line)
	{
		if(pDataPacket.IsInvalid()) {
			PrintError("file: %s line: %u @%s pDataPacket.IsInvalid()", file, line, __FUNCTION__);
			return false;
		}
		const std::string& bytes = pDataPacket->data();

		if(bytes.empty()) {
			PrintError("file: %s line: %u @%s bytes.empty()", file, line, __FUNCTION__);
			return false;
		}

		if(!outMessage.ParseFromArray(bytes.data(), bytes.length())) {
			PrintError("file: %s line: %u @%s !outMessage.ParseFromArray", file, line, __FUNCTION__);
			return false;
		}
		return true;
	}

	inline static bool ParseData(::google::protobuf::Message& outMessage,
		const ::node::DataPacket& packet, const char* file, long line)
	{
		const std::string& bytes = packet.data();

		if(bytes.empty()) {
			PrintError("file: %s line: %u @%s bytes.empty()", file, line, __FUNCTION__);
			return false;
		}

		if(!outMessage.ParseFromArray(bytes.data(), bytes.length())) {
			PrintError("file: %s line: %u @%s !outMessage.ParseFromArray", file, line, __FUNCTION__);
			return false;
		}
		return true;
	}

	inline static eServerError SendToClient(
		uint64_t userId, int32_t cmd,
		const char* file, long line)
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_route(userId);

		return SendToClient(dataPacket, file, line);
	}

	inline static eServerError SendToClient(
		uint64_t userId, int32_t cmd,
		const ::google::protobuf::Message& message,
		const char* file, long line)
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_route(userId);

		int nByteSize = message.ByteSize();
		ntwk::SmallBuffer smallbuf(nByteSize);
		if(!message.SerializeToArray((char*)smallbuf, nByteSize)) {
			PrintError("file: %s line: %u @%s !message.SerializeToArray"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_PARSE_REQUEST;
		}
		dataPacket.set_data((char*)smallbuf, nByteSize);
		return SendToClient(dataPacket, file, line);
	}

	inline static eServerError SendToClient(
		const util::CWeakPointer<::node::DataPacket>& pDataPacket,
		const char* file, long line)
	{
		return SendToClient(*pDataPacket, file, line);
	}

	inline static eServerError SendToClient(
		const ::node::DataPacket& dataPacket,
		const char* file, long line)
	{
		::node::DataPacket cacheRequest;
		cacheRequest.set_cmd(N_CMD_SEND_TO_CLIENT);
		cacheRequest.set_route_type(ROUTE_BALANCE_USERID);
		cacheRequest.set_route(dataPacket.route());

		int nByteSize = dataPacket.ByteSize();
		ntwk::SmallBuffer smallbuf(nByteSize);
		if(!dataPacket.SerializeToArray((char*)smallbuf, nByteSize)) {
			PrintError("file: %s line: %u @%s !dataPacket.SerializeToArray"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_PARSE_REQUEST;
		}
		cacheRequest.set_data((char*)smallbuf, nByteSize);

		::node::DataPacket cacheResponse;
		CCacheOperate::HandleNotification(cacheRequest, cacheResponse);

		return (eServerError)cacheResponse.result();
	}

	inline static eServerError BroadcastToClient(
		int32_t cmd,
		uint64_t excludeId,
		const char* file, long line)
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);

		return BroadcastToClient(dataPacket, excludeId, file, line);
	}

	inline static eServerError BroadcastToClient(
		int32_t cmd,
		const ::google::protobuf::Message& message,
		uint64_t excludeId,
		const char* file, long line)
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);

		int nByteSize = message.ByteSize();
		ntwk::SmallBuffer smallbuf(nByteSize);
		if(!message.SerializeToArray((char*)smallbuf, nByteSize)) {
			PrintError("file: %s line: %u @%s !message.SerializeToArray"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_PARSE_REQUEST;
		}
		dataPacket.set_data((char*)smallbuf, nByteSize);

		return BroadcastToClient(dataPacket, excludeId, file, line);
	}

	inline static eServerError BroadcastToClient(
		const util::CWeakPointer<::node::DataPacket>& pDataPacket,
		uint64_t excludeId,
		const char* file, long line)
	{
		return BroadcastToClient(*pDataPacket, excludeId, file, line);
	}

	inline static eServerError BroadcastToClient(
		const ::node::DataPacket& dataPacket,
		uint64_t excludeId,
		const char* file, long line)
	{
		::node::DataPacket cacheRequest;
		cacheRequest.set_cmd(N_CMD_BROADCAST_TO_CLIENT);
		cacheRequest.set_route_type(ROUTE_BROADCAST_USER);
		cacheRequest.set_route(dataPacket.route());

		int nByteSize = dataPacket.ByteSize();
		ntwk::SmallBuffer smallbuf(nByteSize);
		if(!dataPacket.SerializeToArray((char*)smallbuf, nByteSize)) {
			PrintError("file: %s line: %u @%s !dataPacket.SerializeToArray"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_PARSE_REQUEST;
		}
		cacheRequest.set_data((char*)smallbuf, nByteSize);

		::node::DataPacket cacheResponse;
		CCacheOperate::HandleNotification(cacheRequest, cacheResponse);

		return (eServerError)cacheResponse.result();
	}

	inline static eServerError CloseClient(uint64_t userId, const char* file, long line)
	{
		::node::DataPacket cacheRequest;
		cacheRequest.set_cmd(N_CMD_CLOSE_CLIENT);
		cacheRequest.set_route_type(ROUTE_BALANCE_USERID);
		cacheRequest.set_route(userId);

		::node::DataPacket cacheResponse;
		CCacheOperate::HandleNotification(cacheRequest, cacheResponse);

		return (eServerError)cacheResponse.result();
	}

	inline static eServerError CloseAllClient(const char* file, long line)
	{
		::node::DataPacket cacheRequest;
		cacheRequest.set_cmd(N_CMD_CLOSE_ALLCLIENT);
		cacheRequest.set_route_type(ROUTE_BROADCAST_USER);

		::node::DataPacket cacheResponse;
		CCacheOperate::HandleNotification(cacheRequest, cacheResponse);

		return (eServerError)cacheResponse.result();
	}

	inline static eServerError SendToWorker(
		uint64_t userId, int32_t cmd,
		const char* file, long line)
	{
		::node::DataPacket dataPacket;
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_route(userId);
		dataPacket.set_cmd(cmd);

		return SendToWorker(dataPacket, file, line);
	}

	inline static eServerError SendToWorker(
		uint64_t userId, int32_t cmd,
		const ::google::protobuf::Message& message,
		const char* file, long line)
	{
		::node::DataPacket dataPacket;
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_route(userId);
		dataPacket.set_cmd(cmd);

		int nByteSize = message.ByteSize();
		ntwk::SmallBuffer smallbuf(nByteSize);
		if(!message.SerializeToArray((char*)smallbuf, nByteSize)) {
			PrintError("file: %s line: %u @%s !message.SerializeToArray", file, line, __FUNCTION__);
			return CACHE_ERROR_PARSE_REQUEST;
		}
		dataPacket.set_data((char*)smallbuf, nByteSize);

		return SendToWorker(dataPacket, file, line);
	}

	inline static eServerError SendToWorker(
		const util::CWeakPointer<::node::DataPacket>& pDataPacket,
		const char* file, long line)
	{
		return SendToWorker(*pDataPacket, file, line);
	}

	inline static eServerError SendToWorker(
		const ::node::DataPacket& dataPacket,
		const char* file, long line)
	{
		::node::DataPacket cacheRequest;
		cacheRequest.set_cmd(N_CMD_SEND_TO_WORKER);
		cacheRequest.set_route_type(ROUTE_BALANCE_USERID);
		cacheRequest.set_route(dataPacket.route());

		int nByteSize = dataPacket.ByteSize();
		ntwk::SmallBuffer smallbuf(nByteSize);
		if(!dataPacket.SerializeToArray((char*)smallbuf, nByteSize)) {
			PrintError("file: %s line: %u @%s !dataPacket.SerializeToArray"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_PARSE_REQUEST;
		}
		cacheRequest.set_data((char*)smallbuf, nByteSize);

		::node::DataPacket cacheResponse;
		CCacheOperate::HandleNotification(cacheRequest, cacheResponse);

		return (eServerError)cacheResponse.result();
	}

	inline static eServerError KickLogged(uint64_t userId, const char* file, long line)
	{
		::node::DataPacket cacheRequest;
		cacheRequest.set_cmd(N_CMD_KICK_LOGGED);
		cacheRequest.set_route_type(ROUTE_BALANCE_USERID);
		cacheRequest.set_route(userId);

		::node::DataPacket cacheResponse;
		CCacheOperate::HandleNotification(cacheRequest, cacheResponse);

		return (eServerError)cacheResponse.result();
	}
};

#endif /* CACHEOPERATE_H */

