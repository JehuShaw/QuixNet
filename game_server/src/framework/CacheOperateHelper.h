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
#include "BitSignSet.h"
#include "TransferStream.h"
#include "MCResult.h"
#include "ValueStream.h"
#include "broadcast_packet.pb.h"

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
#define SetRecordNoReply(mcRecordPtr) mcRecordPtr->set_result(mcRecordPtr->result() | MCERR_NOREPLY)
#define SetRecordAttachCas(mcRecordPtr) mcRecordPtr->set_result(mcRecordPtr->result() | MCERR_ATTACH_CAS)
#define SetRecordCas(mcRecordPtr, cas) mcRecordPtr->set_cas(cas)
#define SetRecordCasAndResult(mcRecordPtr, cas) mcRecordPtr->set_result(mcRecordPtr->result() | MCERR_ATTACH_CAS); mcRecordPtr->set_cas(cas)

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
#define HasRecordKey(mcRecordRef) !mcRecordRef.key().empty()
#define HasRecordValue(mcRecordRef) !mcRecordRef.value().empty()
#define HasRecordCas(mcRecordRef) mcRecordRef.result() & MCERR_ATTACH_CAS

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

#define McReadOnlyOneRecordDirServId(serverId, strKey, inValue)\
CCacheOperate::ReadOnlyOneRecord(ROUTE_DIRECT_SERVERID, serverId, strKey, inValue)

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

#define McCheckGlobalExistDirServId(serverId, strKey)\
CCacheOperate::CheckGlobalExist(ROUTE_DIRECT_SERVERID, serverId, strKey)

#define McCheckExistEscapeStringDirServId(serverId, strKey, strNewKey)\
CCacheOperate::CheckExistEscapeString(ROUTE_DIRECT_SERVERID, serverId, strKey, strNewKey)

#define McAllDBStoredProceduresDirServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleStoredProcs(N_CMD_ALL_DB_STOREDPROCEDURES, ROUTE_DIRECT_SERVERID, serverId, mcRequest, mcResponse)

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

#define McReadOnlyOneRecordBalServId(serverId, strKey, inValue)\
CCacheOperate::ReadOnlyOneRecord(ROUTE_BALANCE_SERVERID, serverId, strKey, inValue)

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

#define McCheckGlobalExistBalServId(serverId, strKey)\
CCacheOperate::CheckGlobalExist(ROUTE_BALANCE_SERVERID, serverId, strKey)

#define McCheckExistEscapeStringBalServId(serverId, strKey, strNewKey)\
CCacheOperate::CheckExistEscapeString(ROUTE_BALANCE_SERVERID, serverId, strKey, strNewKey)

#define McAllDBStoredProceduresBalServId(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleStoredProcs(N_CMD_ALL_DB_STOREDPROCEDURES, ROUTE_BALANCE_SERVERID, serverId, mcRequest, mcResponse)

//////////////////////////////////////////////////////////////////////////
// operator by hash 32Key
#define McLoadAllHash32Key(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleMultiRows(N_CMD_LOADALL, ROUTE_HASH_32KEY, serverId, mcRequest, mcResponse)

#define McDBInsertHash32Key(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_INSERT, ROUTE_HASH_32KEY, serverId, mcRequest, mcResponse)

#define McDBSelectHash32Key(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_SELECT, ROUTE_HASH_32KEY, serverId, mcRequest, mcResponse)

#define McDBUpdateHash32Key(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_UPDATE, ROUTE_HASH_32KEY, serverId, mcRequest, mcResponse)

#define McDBDeleteHash32Key(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleSnglRow(N_CMD_DB_DELETE, ROUTE_HASH_32KEY, serverId, mcRequest, mcResponse)

#define McDBSelectAllHash32Key(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleMultiRows(N_CMD_DB_SELECTALL, ROUTE_HASH_32KEY, serverId, mcRequest, mcResponse)

#define McDBStoredProcHash32Key(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleStoredProcs(N_CMD_DB_STOREDPROCEDURES, ROUTE_HASH_32KEY, serverId, mcRequest, mcResponse)

#define McDBAsyncStoredProcHash32Key(serverId, mcRequest, mcResponse)\
CCacheOperate::HandleStoredProcs(N_CMD_DB_ASYNCSTOREDPROCEDURES, ROUTE_HASH_32KEY, serverId, mcRequest, mcResponse)

#define McDBEscapeStringHash32Key(serverId, inEscape, outEscape)\
CCacheOperate::HandleEscapeString(ROUTE_HASH_32KEY, serverId, inEscape, outEscape)

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

#define McReadOnlyOneRecordBalUserId(userId, strKey, inValue)\
CCacheOperate::ReadOnlyOneRecord(ROUTE_BALANCE_USERID, userId, strKey, inValue)

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

#define McCheckGlobalExistBalUserId(userId, strKey)\
CCacheOperate::CheckGlobalExist(ROUTE_BALANCE_USERID, userId, strKey)

#define McCheckExistEscapeStringBalUserId(userId, strKey, strNewKey)\
CCacheOperate::CheckExistEscapeString(ROUTE_BALANCE_USERID, userId, strKey, strNewKey)

#define McAllDBStoredProcBalUserId(userId, mcRequest, mcResponse)\
CCacheOperate::HandleStoredProcs(N_CMD_ALL_DB_STOREDPROCEDURES, ROUTE_BALANCE_USERID, userId, mcRequest, mcResponse)

#ifdef DEFAULT_ROUTE_BY_SERVERID
#define McGetForUpdate McGetForUpdateBalServId
#define McUpdate McUpdateBalServId
#define McAdd McAddBalServId
#define McGet McGetBalServId
#define McSet McSetBalServId
#define McGets McGetsBalServId
#define McCas McCasBalServId
#define McDel McDelBalServId
#define McLoad McLoadBalServId
#define McStore McStoreBalServId
#define McDBInsert McDBInsertBalServId
#define McDBSelect McDBSelectBalServId
#define McDBUpdate McDBUpdateBalServId
#define McDBDelete McDBDeleteBalServId
#define McDBEscapeString McDBEscapeStringBalServId
#define McAddOneRecord McAddOneRecordBalServId
#define McDelOneRecord McDelOneRecordBalServId
#define McCasOneRecord McCasOneRecordBalServId
#define McGetsOneRecord McGetsOneRecordBalServId
#define McReadOnlyOneRecord McReadOnlyOneRecordBalServId
#define McLoadOneRecord McLoadOneRecordBalServId
#define McStoreOneRecord McStoreOneRecordBalServId
#define McInsertOneRecordToDB McInsertOneRecordToDBBalServId
#define McSelectOneRecordFromDB McSelectOneRecordFromDBBalServId
#define McUpdateOneRecordToDB McUpdateOneRecordToDBBalServId
#define McDeleteOneRecordFromDB McDeleteOneRecordFromDBBalServId
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
#define McDBInsert McDBInsertBalUserId
#define McDBSelect McDBSelectBalUserId
#define McDBUpdate McDBUpdateBalUserId
#define McDBDelete McDBDeleteBalUserId
// select all route by server id
#define McDBEscapeString McDBEscapeStringBalUserId
#define McAddOneRecord McAddOneRecordBalUserId
#define McDelOneRecord McDelOneRecordBalUserId
#define McCasOneRecord McCasOneRecordBalUserId
#define McGetsOneRecord McGetsOneRecordBalUserId
#define McReadOnlyOneRecord McReadOnlyOneRecordBalUserId
#define McLoadOneRecord McLoadOneRecordBalUserId
#define McStoreOneRecord McStoreOneRecordBalUserId
#define McInsertOneRecordToDB McInsertOneRecordToDBBalUserId
#define McSelectOneRecordFromDB McSelectOneRecordFromDBBalUserId
#define McUpdateOneRecordToDB McUpdateOneRecordToDBBalUserId
#define McDeleteOneRecordFromDB McDeleteOneRecordFromDBBalUserId
#endif

#define McLoadAll(routeType, route, mcRequest, mcResponse)\
CCacheOperate::HandleMultiRows(N_CMD_LOADALL, routeType, route, mcRequest, mcResponse)

#define McDBSelectAll(routeType, route, mcRequest, mcResponse)\
CCacheOperate::HandleMultiRows(N_CMD_DB_SELECTALL, routeType, route, mcRequest, mcResponse)

#define SendCacheProtocol CCacheOperate::HandleProtocol

#define SendCacheCmdToClient(userId, cmd)\
CCacheOperate::SendToClient(userId, cmd, __FILE__, __LINE__)

#define SendCacheToClient(userId, cmd, message)\
CCacheOperate::SendToClient(userId, cmd, message, __FILE__, __LINE__)

#define SendCachePacketToClient(dataPacket)\
CCacheOperate::SendToClient(dataPacket, __FILE__, __LINE__)

#define BroadcastCacheCmdToClient(route, cmd, includeIds, excludeIds)\
CCacheOperate::BroadcastToClient(route, cmd, includeIds, excludeIds, __FILE__, __LINE__)

#define BroadcastCacheToClient(route, cmd, message, includeIds, excludeIds)\
CCacheOperate::BroadcastToClient(route, cmd, message, includeIds, excludeIds, __FILE__, __LINE__)

#define BroadcastCachePacketToClient(dataPacket, includeIds, excludeIds)\
CCacheOperate::BroadcastToClient(dataPacket, includeIds, excludeIds, __FILE__, __LINE__)

#define CloseCacheClient(userId)\
CCacheOperate::CloseClient(userId, __FILE__, __LINE__)

#define CloseCacheAllClient(route, includeIds, excludeIds)\
CCacheOperate::CloseAllClients(route, includeIds, excludeIds, __FILE__, __LINE__)

#define SendCacheCmdToWorker(userId, cmd)\
CCacheOperate::SendToWorker(userId, cmd, __FILE__, __LINE__)

#define SendCacheToWorker(userId, cmd, message)\
CCacheOperate::SendToWorker(userId, cmd, message, __FILE__, __LINE__)

#define SendCachePacketToWorker(dataPacket)\
CCacheOperate::SendToWorker(dataPacket, __FILE__, __LINE__)

#define KickCacheLogged(userId)\
CCacheOperate::KickLogged(userId, __FILE__, __LINE__)

#define SendCacheCmdToPlayer(route, cmd, pResponse)\
CCacheOperate::SendToPlayer(route, cmd, pResponse, __FILE__, __LINE__)

#define SendCacheToPlayer(route, cmd, message, pResponse)\
CCacheOperate::SendToPlayer(route, cmd, message, pResponse, __FILE__, __LINE__)

#define SendCachePacketToPlayer(dataRequest, dataResponse)\
CCacheOperate::SendToPlayer(dataRequest, dataResponse, __FILE__, __LINE__)

#define PostCacheCmdToPlayer(route, cmd)\
CCacheOperate::PostToPlayer(route, cmd, __FILE__, __LINE__)

#define PostCacheToPlayer(route, cmd, message)\
CCacheOperate::PostToPlayer(route, cmd, message, __FILE__, __LINE__)

#define PostCachePacketToPlayer(dataRequest)\
CCacheOperate::PostToPlayer(dataRequest, __FILE__, __LINE__)

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
		SetRecordCasAndResult(pMcRecord, cas);
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
	inline void SetKey(const util::CValueStream& keyName) {
		m_cacheRequest.set_key(keyName.GetData(), keyName.GetLength());
	}
	inline void SetKey(const char* szKeyName) {
		util::CValueStream key;
		key.Serialize(szKeyName, true);
		m_cacheRequest.set_key(key.GetData(), key.GetLength());
	}
	// The Stored Procedures parameters.
	inline void SetParams(const util::CValueStream& params) {
		m_cacheRequest.set_data(params.GetData(), params.GetLength());
	}
	// No reply result
	inline void SetNoReply(bool bNoReply) {
		if(bNoReply) {
			m_cacheRequest.set_result(m_cacheRequest.result() | MCERR_NOREPLY);
		} else {
			m_cacheRequest.set_result(m_cacheRequest.result() & (~(MCERR_NOREPLY)));
		}
	}
	// Set escape string
	inline void SetEscapeString(bool bEscapeString) {
		if (bEscapeString) {
			m_cacheRequest.set_result(m_cacheRequest.result() | MCERR_ESCAPE_STRING);
		} else {
			m_cacheRequest.set_result(m_cacheRequest.result() & (~(MCERR_ESCAPE_STRING)));
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

		return workerResponse.result();
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
		cacheResponse.set_result(nResult);
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
							pMCRequest->set_expiry(mcGets.expiry());
							pMCRequest->set_flags(mcGets.flags());
							pMCRequest->set_result(mcGets.result());
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
		if(MCERR_OK == nResult) {
			const mc_record_t& mcRecord = GetRecord(cacheAddResponse, 0);
			inValue.SetCas(GetRecordCas(mcRecord));
		} else if(MCCHANGE_NIL != oldChgType) {
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
		SetRecordAttachCas(mcGetRecord);
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
			SetRecordCasAndResult(pMCRecord, inValue.GetCas());
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

	static inline MCResult ReadOnlyOneRecord(eRouteType routeType, uint64_t route,
		const util::CTransferStream& strKeys, ICacheValue& outValue)
	{
		mc_request_t cacheGetRequest;
		mc_record_t* mcGetRecord = SetRecord(cacheGetRequest);
		SetRecordNKey(mcGetRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());
		SetRecordAttachCas(mcGetRecord);
		mc_response_t cacheGetResponse;
		if (CCacheOperate::HandleCache(N_CMD_GETS, routeType, route,
			cacheGetRequest, cacheGetResponse) != SERVER_SUCCESS) {
			return MCERR_NOREPLY;
		}
		MCResult nResult = GetCacheFirstRecordResult(cacheGetResponse);
		if (MCERR_OK == nResult) {
			const mc_record_t& mcRecord = GetRecord(cacheGetResponse, 0);
			outValue.Parse(GetRecordValue(mcRecord));
		}
		return nResult;
	}

    static inline MCResult GetsOneRecord(eRouteType routeType, uint64_t route,
        const util::CTransferStream& strKeys, ICacheValue& outValue)
    {
        mc_request_t cacheGetRequest;
        mc_record_t* mcGetRecord = SetRecord(cacheGetRequest);
        SetRecordNKey(mcGetRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());
		SetRecordAttachCas(mcGetRecord);
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
		SetRecordCasAndResult(pMCRecord, inValue.GetCas());
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
		SetRecordAttachCas(mcGetRecord);
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
		SetRecordCasAndResult(pMCRecord, inValue.GetCas());
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

	inline static  MCResult CheckGlobalExist(
		eRouteType routeType, uint64_t route,
		const util::CTransferStream& strKeys)
	{
		mc_request_t cacheRequest;
		mc_record_t* mcRecord = SetRecord(cacheRequest);
		SetRecordNKey(mcRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());

		mc_response_t cacheResponse;
		if (CCacheOperate::HandleCache(N_CMD_DB_CHECK_GLOBAL_EXISTS, routeType, route,
			cacheRequest, cacheResponse) != SERVER_SUCCESS) {
			return MCERR_NOTSTORED;
		}
		
		return (MCResult)GetCacheFirstRecordResult(cacheResponse);
	}

	inline static  MCResult CheckExistEscapeString(
		eRouteType routeType, uint64_t route,
		const util::CTransferStream& strKeys,
		util::CTransferStream& outNewKeys)
	{
		mc_request_t cacheRequest;
		mc_record_t* mcRecord = SetRecord(cacheRequest);
		SetRecordNKey(mcRecord, strKeys.GetData(), strKeys.GetNumberOfBytesUsed());

		mc_response_t cacheResponse;
		if (CCacheOperate::HandleCache(N_CMD_DB_CHKEXIST_ESCAPESTRING, routeType, route,
			cacheRequest, cacheResponse) != SERVER_SUCCESS) {
			return MCERR_NOTSTORED;
		}

		if(cacheResponse.values_size() > 0) {
			const mc_record_t& mcRecordRespone = GetRecord(cacheResponse, 0);
			const std::string& strNewKey = mcRecordRespone.key();
			outNewKeys.WriteBytes(strNewKey.data(), strNewKey.length());
		}

		return (MCResult)GetCacheFirstRecordResult(cacheResponse);
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
		::node::DataPacket& workerResponse, util::CWeakPointer<CWrapPlayer> pPlayer, int nType = 0) {

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

	inline static util::CWeakPointer<CWrapPlayer> GetPlayer(
		const util::CWeakPointer<mdl::INotification>& request,
		const char* file, long line)
	{
		if(request.IsInvalid()) {
			PrintError("file: %s line: %u @%s request.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<CWrapPlayer>();
		}

		util::CWeakPointer<const CBodyMessage> pBodyMessage(request->GetBody());
		if(pBodyMessage.IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyMessage.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<CWrapPlayer>();
		}

		if(pBodyMessage->GetPlayer().IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyMessage->getMessage().IsInvalid()", file, line, __FUNCTION__);
		}

		return pBodyMessage->GetPlayer();
	}

	inline static util::CWeakPointer<CWrapPlayer> GetPlayerSilence(
		const util::CWeakPointer<mdl::INotification>& request)
	{
		if(request.IsInvalid()) {
			return util::CWeakPointer<CWrapPlayer>();
		}

		util::CWeakPointer<CBodyMessage> pBodyMessage(request->GetBody());
		if(pBodyMessage.IsInvalid()) {
			return util::CWeakPointer<CWrapPlayer>();
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
		if(!message.SerializeToString(pDataPacket->mutable_data())) {
			PrintError("file: %s line: %u @%s !message.SerializeToString", file, line, __FUNCTION__);
			return false;
		}
		return true;
	}

	inline static bool SerializeData(::node::DataPacket& outPacket,
		const ::google::protobuf::Message& message, const char* file, long line)
	{
		if (!message.SerializeToString(outPacket.mutable_data())) {
			PrintError("file: %s line: %u @%s !message.SerializeToString", file, line, __FUNCTION__);
			return false;
		}
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
		dataPacket.set_result(SERVER_SUCCESS);

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
		dataPacket.set_result(SERVER_SUCCESS);

		if (!message.SerializeToString(dataPacket.mutable_data())) {
			PrintError("file: %s line: %u @%s !message.SerializeToString"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_SERIALIZE_REQUEST;
		}
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

		if (!dataPacket.SerializeToString(cacheRequest.mutable_data())) {
			PrintError("file: %s line: %u @%s !dataPacket.SerializeToString"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_SERIALIZE_REQUEST;
		}

		::node::DataPacket cacheResponse;
		CCacheOperate::HandleNotification(cacheRequest, cacheResponse);

		return (eServerError)cacheResponse.result();
	}

	inline static eServerError BroadcastToClient(
		uint64_t route,
		int32_t cmd,
		const std::set<uint64_t>* includeIds,
		const std::set<uint64_t>* excludeIds,
		const char* file, long line)
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_route(route);
		dataPacket.set_result(SERVER_SUCCESS);

		return BroadcastToClient(dataPacket, includeIds, excludeIds, file, line);
	}

	inline static eServerError BroadcastToClient(
		uint64_t route,
		int32_t cmd,
		const ::google::protobuf::Message& message,
		const std::set<uint64_t>* includeIds,
		const std::set<uint64_t>* excludeIds,
		const char* file, long line)
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_route(route);
		dataPacket.set_result(SERVER_SUCCESS);

		if (!message.SerializeToString(dataPacket.mutable_data())) {
			PrintError("file: %s line: %u @%s !message.SerializeToString"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_SERIALIZE_REQUEST;
		}

		return BroadcastToClient(dataPacket, includeIds, excludeIds, file, line);
	}

	inline static eServerError BroadcastToClient(
		const util::CWeakPointer<::node::DataPacket>& pDataPacket,
		const std::set<uint64_t>* includeIds,
		const std::set<uint64_t>* excludeIds,
		const char* file, long line)
	{
		return BroadcastToClient(*pDataPacket, includeIds, excludeIds, file, line);
	}

	inline static eServerError BroadcastToClient(
		const ::node::DataPacket& dataPacket,
		const std::set<uint64_t>* includeIds,
		const std::set<uint64_t>* excludeIds,
		const char* file, long line)
	{
		::node::DataPacket cacheRequest;
		cacheRequest.set_cmd(N_CMD_BROADCAST_TO_CLIENT);
		cacheRequest.set_route_type(ROUTE_BROADCAST_USER);
		cacheRequest.set_route(dataPacket.route());

		::node::BroadcastRequest broadcastRequest;
		if (NULL == includeIds) {
			if(NULL != excludeIds) {
				std::set<uint64_t>::const_iterator itEx(excludeIds->begin());
				for (; excludeIds->end() != itEx; ++itEx) {
					broadcastRequest.add_excludeids(*itEx);
				}
			}
		} else {
			if(includeIds->empty()) {
				return SERVER_SUCCESS;
			} else {
				std::set<uint64_t>::const_iterator itIn(includeIds->begin());
				for (; includeIds->end() != itIn; ++itIn) {
					broadcastRequest.add_includeids(*itIn);
				}
			}
		}
		if (!dataPacket.SerializeToString(broadcastRequest.mutable_data())) {
			PrintError("file: %s line: %u @%s !dataPacket.SerializeToString(broadcastRequest)"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_SERIALIZE_REQUEST;
		}

		if (!broadcastRequest.SerializeToString(cacheRequest.mutable_data())) {
			PrintError("file: %s line: %u @%s !dataPacket.SerializeToString(cacheRequest)"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_SERIALIZE_REQUEST;
		}

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

	inline static eServerError CloseAllClients(
		uint64_t route,
		const std::set<uint64_t>* includeIds,
		const std::set<uint64_t>* excludeIds,
		const char* file, long line)
	{
		::node::DataPacket cacheRequest;
		cacheRequest.set_cmd(N_CMD_CLOSE_ALLCLIENTS);
		cacheRequest.set_route_type(ROUTE_BROADCAST_USER);
		cacheRequest.set_route(route);

		::node::BroadcastRequest broadcastRequest;
		if (NULL == includeIds) {
			if(NULL != excludeIds) {
				std::set<uint64_t>::const_iterator itEx(excludeIds->begin());
				for (; excludeIds->end() != itEx; ++itEx) {
					broadcastRequest.add_excludeids(*itEx);
				}
			}
		} else {
			if (includeIds->empty()) {
				return SERVER_SUCCESS;
			} else {
				std::set<uint64_t>::const_iterator itIn(includeIds->begin());
				for (; includeIds->end() != itIn; ++itIn) {
					broadcastRequest.add_includeids(*itIn);
				}
			}
		}

		if (!broadcastRequest.SerializeToString(cacheRequest.mutable_data())) {
			PrintError("file: %s line: %u @%s !broadcastRequest.SerializeToString(cacheRequest)"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_SERIALIZE_REQUEST;
		}

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

		if (!message.SerializeToString(dataPacket.mutable_data())) {
			PrintError("file: %s line: %u @%s !message.SerializeToString", file, line, __FUNCTION__);
			return CACHE_ERROR_SERIALIZE_REQUEST;
		}

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

		if (!dataPacket.SerializeToString(cacheRequest.mutable_data())) {
			PrintError("file: %s line: %u @%s !dataPacket.SerializeToString"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_SERIALIZE_REQUEST;
		}

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

	inline static eServerError SendToPlayer(
		uint64_t route,
		int32_t cmd,
		::google::protobuf::Message* pResponse,
		const char* file, long line)
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_route(route);

		::node::DataPacket dataResponse;
		eServerError nResult = SendToPlayer(dataPacket, dataResponse, file, line);
		if (SERVER_SUCCESS == nResult) {
			if (NULL != pResponse) {
				if (!pResponse->ParseFromString(dataResponse.data())) {
					PrintError("file: %s line: %u @%s !pResponse->ParseFromString(dataResponse)"
						, file, line, __FUNCTION__);
					return CACHE_ERROR_PARSE_REQUEST;
				}
			}
		}
		return nResult;
	}

	inline static eServerError SendToPlayer(
		uint64_t route,
		int32_t cmd,
		const ::google::protobuf::Message& message,
		::google::protobuf::Message* pResponse,
		const char* file, long line)
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_route(route);

		if (!message.SerializeToString(dataPacket.mutable_data())) {
			PrintError("file: %s line: %u @%s !message.SerializeToString"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_SERIALIZE_REQUEST;
		}
		::node::DataPacket dataResponse;
		eServerError nResult = SendToPlayer(dataPacket, dataResponse, file, line);
		if (SERVER_SUCCESS == nResult) {
			if (NULL != pResponse) {
				if (!pResponse->ParseFromString(dataResponse.data())) {
					PrintError("file: %s line: %u @%s !pResponse->ParseFromString(dataResponse)"
						, file, line, __FUNCTION__);
					return CACHE_ERROR_PARSE_REQUEST;
				}
			}
		}
		return nResult;
	}

	inline static eServerError SendToPlayer(
		const util::CWeakPointer<::node::DataPacket>& pDataRequest,
		util::CWeakPointer<::node::DataPacket>& pDataResponse,
		const char* file, long line)
	{
		return SendToPlayer(*pDataRequest, *pDataResponse, file, line);
	}

	inline static eServerError SendToPlayer(
		const ::node::DataPacket& dataPacket,
		::node::DataPacket& dataResponse,
		const char* file, long line)
	{
		::node::DataPacket cacheRequest;
		cacheRequest.set_cmd(N_CMD_SEND_TO_PLAYER);
		cacheRequest.set_route_type(ROUTE_BROADCAST_USER);
		cacheRequest.set_route(dataPacket.route());

		::node::ForwardRequest forwardRequest;
		forwardRequest.mutable_data()->CopyFrom(dataPacket);

		if (!forwardRequest.SerializeToString(cacheRequest.mutable_data())) {
			PrintError("file: %s line: %u @%s !dataPacket.SerializeToString(cacheRequest)"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_SERIALIZE_REQUEST;
		}

		CCacheOperate::HandleNotification(cacheRequest, dataResponse);

		return (eServerError)dataResponse.result();
	}

	inline static eServerError PostToPlayer(
		uint64_t route,
		int32_t cmd,
		const char* file, long line)
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_route(route);

		return PostToPlayer(dataPacket, file, line);
	}

	inline static eServerError PostToPlayer(
		uint64_t route,
		int32_t cmd,
		const ::google::protobuf::Message& message,
		const char* file, long line)
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_route(route);

		if (!message.SerializeToString(dataPacket.mutable_data())) {
			PrintError("file: %s line: %u @%s !message.SerializeToString"
				, file, line, __FUNCTION__);
		}

		return PostToPlayer(dataPacket, file, line);
	}

	inline static eServerError PostToPlayer(
		const util::CWeakPointer<::node::DataPacket>& pDataRequest,
		const char* file, long line)
	{
		return PostToPlayer(*pDataRequest, file, line);
	}

	inline static eServerError PostToPlayer(
		const ::node::DataPacket& dataPacket,
		const char* file, long line)
	{
		::node::DataPacket cacheRequest;
		cacheRequest.set_cmd(N_CMD_POST_TO_PLAYER);
		cacheRequest.set_route_type(ROUTE_BROADCAST_USER);
		cacheRequest.set_route(dataPacket.route());

		::node::ForwardRequest forwardRequest;
		forwardRequest.mutable_data()->CopyFrom(dataPacket);

		if (!forwardRequest.SerializeToString(cacheRequest.mutable_data())) {
			PrintError("file: %s line: %u @%s !dataPacket.SerializeToString(cacheRequest)"
				, file, line, __FUNCTION__);
		}

		::node::DataPacket dataResponse;
		CCacheOperate::HandleNotification(cacheRequest, dataResponse);
		return (eServerError)dataResponse.result();
	}

};

#endif /* CACHEOPERATE_H */

