/*
 * File:   HttpRequest.h
 * Author: Jehu Shaw
 *
 */

#ifndef _HTTPREQUEST_H
#define _HTTPREQUEST_H

#include "Common.h"

#ifndef CURL_ERROR_SIZE
#define CURL_ERROR_SIZE 256
#endif

typedef void CURL;

namespace ntwk {

class SHARED_DLL_DECL HttpRequest
{
public:
	HttpRequest() : m_curlHandle(NULL) {
		memset(m_error, 0, sizeof(m_error));
	}

	virtual ~HttpRequest() {}

	// 获得下载文件URL
	virtual const char* GetUrl() const = 0;
	// 数据到达事件，返回值代表本次已处理数据大小，需等于bytes，否则下载被中止
	virtual unsigned int OnData(const void * data, unsigned int bytes) = 0;
	// 协议头返回事件，返回值代表本次已处理数据大小，需等于bytes，否则下载被中止
	virtual unsigned int OnHead(const char * data, unsigned int bytes) = 0;
	//
	virtual int OnProgress(double dltotal, double dlnow) = 0;
	// 结束事件（完成或者失败）
	virtual void OnDone(unsigned int dwStatus, const char * szError) {
		EasyClearup();
	}

	// 获得http交互方法
	virtual const char * GetMethod() const {
		return "GET";
	}
	// 获得下载文件的起始偏移位置，-1为不指定同时忽略end (闭区间)
	virtual long GetStartOffset() const {
		return -1;
	}
	// 获得下载文件结束偏移位置，-1为不指定 (闭区间)
	virtual long GetEndOffset() const {
		return -1;
	}

protected:
	int GetServerReturnCode();

	double GetContentLength();

	void EasyClearup();

	inline const char* GetErrorInfo() const {
		return m_error;
	}

private:
	friend class CHttpClientManager;
	CURL* m_curlHandle;
	char m_error[CURL_ERROR_SIZE];
};

}

#endif /* _HTTPREQUEST_H */
