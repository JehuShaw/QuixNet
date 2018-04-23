#include "LoginWebRequest.h"
#include "Common.h"
#include "Log.h"
#include "json/json.h"
#include "GameClient.h"


CLoginWebRequest::CLoginWebRequest(const char *pUrl)
	: m_url(pUrl)
{

}

CLoginWebRequest::~CLoginWebRequest()
{

}

CLoginWebRequest * CLoginWebRequest::Create(const char *pUrl)
{
    return new CLoginWebRequest(pUrl);
}

void CLoginWebRequest::Release(CLoginWebRequest* pSession )
{
    delete pSession;
}

const char * CLoginWebRequest::GetUrl() const
{
    return m_url.c_str();
}

unsigned int CLoginWebRequest::OnData(const void *pData, unsigned int bytes)
{
	if(NULL == pData){
		OutputDebug("NULL == pData");
		return bytes;
	}

	std::string strJson((char*)pData);

	Json::Value jsonValue;
	Json::Reader jsonReader;
	jsonReader.parse(strJson, jsonValue, false);

	if(!jsonValue.isObject()) {
		OutputDebug("OnData is FAIL [%s] ", strJson.c_str());
		return bytes;
	}

	Json::Value& jsonCode = jsonValue["code"];
	if(jsonCode.isInt() && jsonCode.asInt() == TRUE) {
		CGameClient::PTR_T pGameClient(CGameClient::Pointer());

		Json::Value& jsonAccountId = jsonValue["accountId"];
		uint64_t u64Account = jsonAccountId.asUInt64();
		Json::Value& jsonSessionKey = jsonValue["sessionKey"];
		std::string strSessionKey(jsonSessionKey.asString());
		Json::Value& jsonLoginIp = jsonValue["loginIp"];
		std::string strLoginIp(jsonLoginIp.asString());
		pGameClient->LoginWebSuccess(strLoginIp.c_str(), u64Account, strSessionKey.c_str());
	} else {
		CGameClient::PTR_T pGameClient(CGameClient::Pointer());
		pGameClient->LoginWebFail();
	}
    return bytes;
}

unsigned int CLoginWebRequest::OnHead(const char *pHead, unsigned int bytes)
{
    return bytes;
}

int CLoginWebRequest::OnProgress(double dltotal, double dlnow)
{
    return 0;
}

void CLoginWebRequest::OnDone(unsigned int dwStatus, const char *)
{
    EasyClearup();
    Release(this);
}


