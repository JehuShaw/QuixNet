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

	// ��������ļ�URL
	virtual const char* GetUrl() const = 0;
	// ���ݵ����¼�������ֵ�������Ѵ������ݴ�С�������bytes���������ر���ֹ
	virtual unsigned int OnData(const void * data, unsigned int bytes) = 0;
	// Э��ͷ�����¼�������ֵ�������Ѵ������ݴ�С�������bytes���������ر���ֹ
	virtual unsigned int OnHead(const char * data, unsigned int bytes) = 0;
	//
	virtual int OnProgress(double dltotal, double dlnow) = 0;
	// �����¼�����ɻ���ʧ�ܣ�
	virtual void OnDone(unsigned int dwStatus, const char * szError) {
		EasyClearup();
	}

	// ���http��������
	virtual const char * GetMethod() const {
		return "GET";
	}
	// ��������ļ�����ʼƫ��λ�ã�-1Ϊ��ָ��ͬʱ����end (������)
	virtual long GetStartOffset() const {
		return -1;
	}
	// ��������ļ�����ƫ��λ�ã�-1Ϊ��ָ�� (������)
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
