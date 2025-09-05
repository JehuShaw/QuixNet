#include "LoginWebRequest.h"
#include "Common.h"
#include "Log.h"
#include "ModuleOperateHelper.h"
#include "json/json.h"


CLoginWebRequest::CLoginWebRequest(
	const char *pUrl,
	uint64_t account,
	const ntwk::SocketID& socketId)

	: m_url(pUrl)
	, m_account(account)
	, m_socketId(socketId)
{

}

CLoginWebRequest::~CLoginWebRequest()
{

}

CLoginWebRequest * CLoginWebRequest::Create(
	const char *pUrl,
	uint64_t account,
	const ntwk::SocketID& socketId)
{
    return new CLoginWebRequest(pUrl, account, socketId);
}

void CLoginWebRequest::Release(CLoginWebRequest* pSession)
{
    delete pSession;
}

const char * CLoginWebRequest::GetUrl() const
{
    return m_url.c_str();
}

unsigned int CLoginWebRequest::OnData(const void *pData, unsigned int bytes)
{
	printf("CLoginWebRequest::OnData  Begin account = " I64FMTD " \n ", m_account);
	if(NULL == pData) {
		OutputDebug("NULL == pData ");
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

	CBodyBitStream body;
	Json::Value& jsonCode = jsonValue["code"];
	if(jsonCode.isInt()) {		
		Json::Value& jsonAccountId = jsonValue["accountId"];
		if(jsonAccountId.isUInt64()) {
			const uint64_t account = jsonAccountId.asUInt64();
			if(account == m_account) {
				body.WriteBool(jsonCode.asInt() == TRUE);
			} else {
				body.WriteBool(false);
				OutputError("jsonAccountId.asUInt64()[" I64FMTD
					"] != m_account[" I64FMTD "]", account, m_account);
			}
		} else {
			body.WriteBool(false);
			OutputError("!jsonAccountId.isUInt64() type = %d ", jsonAccountId.type());
		}
	} else {
		body.WriteBool(false);
		OutputError("!jsonCode.isInt() type = %d ", jsonCode.type());
	}

	body.WriteUInt64(m_account);
	body.WriteInt32(m_socketId.index);
	body.WriteUInt32(m_socketId.binaryAddress);
	body.WriteUInt16(m_socketId.port);

	printf("CLoginWebRequest::OnData end account = " I64FMTD " \n ", m_account);

	CBodyBitStream response;
	SendModuleNotification(N_CMD_LOGIN_CHECK_WEB_RESULT, body, response);
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


