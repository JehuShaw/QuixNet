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

	// 数据获取回来回调
	static size_t WriteFunc(void *buffer, size_t size, size_t count, void *user_p);
	// 头信息返回回调
	static size_t HeadFunc(void *ptr, size_t size, size_t nmemb, void *userdata);
	// 调试信息数据
	static int DebugFunc(CURL *, int, char *, size_t, void *);
	// 进度回调
	static int ProgressFunc(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

	CCurlMultiMT* m_curlMulti;
};

}

#endif /* _HTTPCLIENTMANAGER_H */