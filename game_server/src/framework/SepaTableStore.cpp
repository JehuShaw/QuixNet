#include "SepaTableStore.h"
#include "Log.h"
#include "CacheOperateHelper.h"
#include "SmallBuffer.h"
#include "ValueStream.h"

using namespace mdl;
using namespace ntwk;
using namespace util;

CSepaTableStore::CSepaTableStore(uint16_t u16CacheId) 
	: m_u16CacheId(u16CacheId), m_nMaxTableSize(0) {
;
}

CSepaTableStore::~CSepaTableStore() {
}

int CSepaTableStore::LoadMaxTableSize(uint16_t u16CacheId, const char* szSPMaxTableSize)
{
	if(NULL == szSPMaxTableSize) {
		OutputDebug("NULL == szSPMaxTableSize");
		return 0;
	}

	CValueStream datas;
	datas.Serialize(szSPMaxTableSize);

	CRequestStoredProcs loadRequest;
	loadRequest.SetKey(datas);

	CResponseRows loadResponse;
	McDBStoredProcDirServId(u16CacheId, loadRequest, loadResponse);
	int nSize = loadResponse.GetSize();
	if(nSize < 1) {
		OutputDebug("nSize < 1");
		return 0;
	}

	int32_t nTableSize = 0;
	CValueStream value(loadResponse.GetValue(0));
	value.Parse(nTableSize);
	return nTableSize;
}

bool CSepaTableStore::LoadTables(long nMaxTableSize, const char* szSPExistTables)
{
	if(NULL == szSPExistTables) {
		OutputDebug("NULL == szSPExistTables");
		return false;
	}

	if(nMaxTableSize < 1) {
		OutputDebug("nMaxTableSize < 1");
		return false;
	}

	CValueStream datas;
	datas.Serialize(szSPExistTables);
	CRequestStoredProcs loadRequest;
	loadRequest.SetKey(datas);

	CResponseRows loadResponse;
	McDBStoredProcDirServId(GetCacheId(), loadRequest, loadResponse);
	int nSize = loadResponse.GetSize();
	if(nSize > 0) {
		if(!Empty()) {
			Clear();
		}
		for(int i = 0; i < nSize; ++i) {
			CValueStream record(loadResponse.GetValue(i));
			std::string strValue;
			record.Parse(strValue);
			int nIndex = ParseInt(strValue.c_str());
			if(nIndex >= 0 && nIndex < nMaxTableSize) {
				if(CheckReassign(nIndex)) {
					Reassign(nIndex);
				}
				SetExistence(nIndex);
			}
		}
	}
	atomic_xchg(&m_nMaxTableSize, nMaxTableSize);
	return true;
}

bool CSepaTableStore::CheckAndStore(const char* szSPCreate, const char* szSPStore, uint64_t userId, const char* szContext, int nLength)
{
	if(m_nMaxTableSize < 1) {
		OutputDebug("m_nMaxTableSize < 1");
		return false;
	}
	int nIndex = GetTableIndex(userId);
	if(nIndex < 0) {
		OutputDebug("nIndex < 0");
		return false;
	}
	if(CheckReassign(nIndex)) {
		Reassign(nIndex);
	}

	if(CheckNonexistent(nIndex) && NULL != szSPCreate) {
		
		CValueStream datas;
		datas.Serialize(szSPCreate);
		CRequestStoredProcs createRequest;
		createRequest.SetKey(datas);

		datas.Reset();
		datas.Serialize(nIndex);
		createRequest.SetParams(datas);

		CResponseRows loadResponse;
		McDBStoredProcDirServId(GetCacheId(), createRequest, loadResponse);

		eServerError nResult = loadResponse.GetServResult();
		if(SERVER_SUCCESS == nResult) {
			SetExistence(nIndex);
		} else {
			OutputDebug("%s() result = %d", szSPCreate, nResult);
		}
	} 

	if(NULL == szSPStore) {
		OutputDebug("NULL == szSPStore");
		return false;
	}

	// if don't set the data then return
	if(NULL == szContext || nLength < 1) {
		OutputDebug("NULL == szContext || nLength < 1");
		return false;
	}

	CValueStream datas;
	datas.Serialize(szSPStore);
	CRequestStoredProcs request;
	request.SetKey(datas);

	datas.Reset();
	datas.Serialize(nIndex);
	datas.Serialize(szContext);
	request.SetParams(datas);
	// set not block request
	request.SetNoReply(true);

	CResponseRows response;
	McDBStoredProcDirServId(GetCacheId(), request, response);
	return true;
}

bool CSepaTableStore::Store(const char* szSPStore, uint64_t userId, const char* szContext, int nLength)
{
	if(NULL == szSPStore) {
		OutputDebug("NULL == szSPStore");
		return false;
	}

	// if don't set the data then return
	if(NULL == szContext || nLength < 1) {
		OutputDebug("NULL == szContext || nLength < 1");
		return false;
	}

	if(m_nMaxTableSize < 1) {
		OutputDebug("m_nMaxTableSize < 1");
		return false;
	}

	int nIndex = GetTableIndex(userId);
	if(nIndex < 0) {
		OutputDebug("nIndex < 0");
		return false;
	}
	if(CheckReassign(nIndex)) {
		OutputDebug("CheckReassign(nIndex)");
		return false;
	}

	CValueStream datas;
	datas.Serialize(szSPStore);
	CRequestStoredProcs request;
	request.SetKey(datas);

	datas.Reset();
	datas.Serialize(nIndex);
	datas.Serialize(szContext);
	request.SetParams(datas);
	// set not block request
	request.SetNoReply(true);

	CResponseRows response;
	McDBStoredProcDirServId(GetCacheId(), request, response);
	return true;
}

