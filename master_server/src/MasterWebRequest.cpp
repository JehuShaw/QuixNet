#include "MasterWebRequest.h"
#include "Common.h"
#include "Log.h"
#include "json/json.h"
#include "SessionEventManager.h"

using namespace util;


CMasterWebRequest::CMasterWebRequest(const char *pUrl,
	unsigned int socketIdx, unsigned int account)

	: m_url(pUrl), m_socketIdx(socketIdx), m_account(account)
{

}

CMasterWebRequest::~CMasterWebRequest()
{

}

CMasterWebRequest * CMasterWebRequest::Create(const char *pUrl,
	unsigned int socketIdx, unsigned int account)
{
    return new CMasterWebRequest(pUrl, socketIdx, account);
}

void CMasterWebRequest::Release(CMasterWebRequest* pSession )
{
    delete pSession;
}

const char * CMasterWebRequest::GetUrl() const
{
    return m_url.c_str();
}

unsigned int CMasterWebRequest::OnData(const void *pData, unsigned int bytes)
{
	if(NULL == pData) {
		OutputDebug("NULL == pData");
		return bytes;
	}

	std::string strJson((char*)pData);

	Json::Value jsonValue;
	Json::Reader jsonReader;
	jsonReader.parse(strJson, jsonValue, false);

	if(!jsonValue.isObject()) {
		OutputDebug("!jsonValue.isObject() [%s]", strJson.c_str());
		return bytes;
	}

	CArgBitStream argment;
	Json::Value& jsonCode = jsonValue["code"];
	if(jsonCode.isInt()) {
		argment.WriteBool(jsonCode.asInt() == TRUE);
	} else {
		argment.WriteBool(false);
	}

	argment.WriteUInt32(m_account);
    argment.WriteInt32(m_socketIdx);

    CAutoPointer<CArgBitStream> pArg(&argment, false);

    CSessionEventManager::PTR_T pSessionEventMgr(CSessionEventManager::Pointer());
    pSessionEventMgr->DispatchEvent(SESSION_EVENT_LOGIN_CHECK, pArg);

    return bytes;
}

unsigned int CMasterWebRequest::OnHead(const char *pHead, unsigned int bytes)
{
    return bytes;
}

int CMasterWebRequest::OnProgress(double dltotal, double dlnow)
{
    return 0;
}

void CMasterWebRequest::OnDone(unsigned int dwStatus, const char *)
{
    EasyClearup();
    Release(this);
}


