/*
 * File:   HttpRequest.h
 * Author: Jehu Shaw
 *
 */

#include "HttpRequest.h"
#include "curl.h"

namespace ntwk {

	int HttpRequest::GetServerReturnCode() {
		long ret = 0;
		if (NULL == m_curlHandle) {
			return ret;
		}
		curl_easy_getinfo(m_curlHandle, CURLINFO_RESPONSE_CODE, &ret);
		return ret;
	}

	double HttpRequest::GetContentLength() {
		double ret = 0.0;
		if (NULL == m_curlHandle) {
			return ret;
		}
		curl_easy_getinfo(m_curlHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &ret);
		return ret;
	}

	void HttpRequest::EasyClearup() {
		if(NULL == m_curlHandle) {
			return;
		}
		curl_easy_cleanup(m_curlHandle);
		m_curlHandle = NULL;
	}

} // end namespace ntwk

