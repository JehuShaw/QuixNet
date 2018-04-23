/*
 * File:   HttpClientManager.h
 * Author: Jehu Shaw
 *
 */

#ifndef _HTTPCLIENTMANAGER_H
#define _HTTPCLIENTMANAGER_H

#include "HttpRequest.h"
#include "Singleton.h"

namespace ntwk {

class CCurlMultiMT;

class SHARED_DLL_DECL CHttpClientManager
	: public util::Singleton<CHttpClientManager>
{
public:

	CHttpClientManager();

	~CHttpClientManager();

	bool AddRequest(HttpRequest* request);

	bool Run();

private:

	// ���ݻ�ȡ�����ص�
	static size_t WriteFunc(void *buffer, size_t size, size_t count, void *user_p);
	// ͷ��Ϣ���ػص�
	static size_t HeadFunc(void *ptr, size_t size, size_t nmemb, void *userdata);
	// ������Ϣ����
	static int DebugFunc(CURL *, int, char *, size_t, void *);
	// ���Ȼص�
	static int ProgressFunc(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

	CCurlMultiMT* m_curlMulti;
};

}

#endif /* _HTTPCLIENTMANAGER_H */