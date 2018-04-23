#include "HttpClientManager.h"
#include "curl.h"
#include "CurlMultiMT.h"

#if defined OPEN_LOG
#include <fstream>
#include "Log.h"
#endif //   OPEN_LOG

namespace ntwk {

CHttpClientManager::CHttpClientManager() {

	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
	if (CURLE_OK != code) {
#ifdef OPEN_LOG
		Log.outLog("HttpClient.log", "curl_global_init() returns %d\n", code);
#endif
	}
	m_curlMulti = new CCurlMultiMT;
}

CHttpClientManager::~CHttpClientManager() {
	curl_global_cleanup();
	delete m_curlMulti;
	m_curlMulti = NULL;
}

bool CHttpClientManager::AddRequest(HttpRequest* pRequest) {

	if(NULL == pRequest) {
#if defined OPEN_LOG
		Log.outLog("HttpClient.log", "FatalErr, ISession is NULL");
#endif //   OPEN_LOG
		return false;
	}

	// 添加CURL
	CURL *& pCURL = pRequest->m_curlHandle;
	if(NULL == pCURL) {
		pCURL = curl_easy_init();
	} else {
#ifdef OPEN_LOG
		Log.outLog("HttpClient.log", "Re-use easy handle? 0x%x\n", (unsigned int)pCURL);
#endif
	}

	if(NULL == pCURL) {
#ifdef OPEN_LOG
		Log.outLog("HttpClient.log", "curl_easy_init returns NULL");
#endif
		return false;
	}

	if(pRequest->GetUrl()) {
		if(CURLE_OK != curl_easy_setopt(pCURL, CURLOPT_URL, pRequest->GetUrl())) {
#ifdef OPEN_LOG
			Log.outLog("HttpClient.log", "curl_easy_setopt did NOT return CURLE_OK.");
#endif
		}
	}

	const char *szStr = NULL;
	if(NULL != (szStr = pRequest->GetMethod())) {
		if (STRCASECMP(szStr, "get") == 0) {
			curl_easy_setopt(pCURL, CURLOPT_HTTPGET, 1);	// METHOD
		} else if (STRCASECMP(szStr, "head") == 0) {
			curl_easy_setopt(pCURL, CURLOPT_NOBODY, 1);
		} else if (STRCASECMP(szStr, "post") == 0) {
			curl_easy_setopt(pCURL, CURLOPT_HTTPPOST, 1);
		}
	}

	curl_easy_setopt(pCURL, CURLOPT_PRIVATE, pRequest);
	curl_easy_setopt(pCURL, CURLOPT_NOPROGRESS, 0L);								// Progress
	curl_easy_setopt(pCURL, CURLOPT_PROGRESSFUNCTION, &ProgressFunc);
	curl_easy_setopt(pCURL, CURLOPT_PROGRESSDATA, pRequest);
	curl_easy_setopt(pCURL, CURLOPT_WRITEFUNCTION, &WriteFunc);	// WRITE CALL
	curl_easy_setopt(pCURL, CURLOPT_WRITEDATA, pRequest);
	curl_easy_setopt(pCURL, CURLOPT_HEADERFUNCTION, &HeadFunc);	// HEAD CALL
	curl_easy_setopt(pCURL, CURLOPT_HEADERDATA, pRequest);
	curl_easy_setopt(pCURL, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(pCURL, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)");
	curl_easy_setopt(pCURL, CURLOPT_ERRORBUFFER, pRequest->m_error);
	curl_easy_setopt(pCURL, CURLOPT_FAILONERROR, 1L);
	curl_easy_setopt(pCURL, CURLOPT_CONNECTTIMEOUT, 10L);
	curl_easy_setopt(pCURL, CURLOPT_DNS_CACHE_TIMEOUT, -1);

#if 0 && defined _DEBUG
	//	curl_easy_setopt(pCURL, CURLOPT_HEADER, 1);
	curl_easy_setopt(pCURL, CURLOPT_VERBOSE, 1);
	curl_easy_setopt(pCURL, CURLOPT_DEBUGFUNCTION, &SessionManager::debugFunc);
	curl_easy_setopt(pCURL, CURLOPT_DEBUGDATA, pSession);
#endif //   _DEBUG

	long begin = pRequest->GetStartOffset();
	long end = pRequest->GetEndOffset();
	if (begin >= 0) {
		char buffer[128] = {0};
		if (end > 0) {
			snprintf(buffer, sizeof(buffer), "%ld-%ld", begin, end);
		} else {
			snprintf(buffer, sizeof(buffer), "%ld-", begin);
		}
		buffer[127] = 0;
		curl_easy_setopt(pCURL, CURLOPT_RANGE, buffer);
	}

	// 加入multi handle
	CURLMcode code = (CURLMcode)m_curlMulti->AddHandle(pCURL);
	if(CURLM_OK != code) {
#ifdef OPEN_LOG
		Log.outLog("HttpClient.log", "FatalErr, AddHandle return error:[%s]", curl_multi_strerror(code));
#endif //   OPEN_LOG
		return false;
	}
	return true;
}

size_t CHttpClientManager::WriteFunc(void *buffer, size_t size, size_t count , void* pUserData) {
	HttpRequest* pRequest = (HttpRequest*)pUserData;
	if(NULL == pRequest) {
		return 0;
	}
	return static_cast<size_t>(pRequest->OnData(buffer, (unsigned int)(size * count)));
//	return value should be 'count' * 'size' if succeed.
}

size_t CHttpClientManager::HeadFunc(void *ptr, size_t size, size_t nmemb , void* pUserData) {
	HttpRequest* pRequest = (HttpRequest*)pUserData;
	if(NULL == pRequest) {
		return 0;
	}
	return static_cast<size_t>(pRequest->OnHead(reinterpret_cast<const char*>(ptr), (unsigned int)(size * nmemb)));
	// should be 'nmemb' * 'size' if succeed.
}

int CHttpClientManager::DebugFunc(CURL *curl, int infotype, char *data, size_t data_size, void *pVSession) {

	if(infotype > CURLINFO_HEADER_OUT) {
		return 0;
	}

#if defined OPEN_LOG
	static const char *curl_infotype_map[] = {
		"CURLINFO_TEXT",
		"CURLINFO_HEADER_IN",
		"CURLINFO_HEADER_OUT",
		"CURLINFO_DATA_IN",
		"CURLINFO_DATA_OUT",
		"CURLINFO_SSL_DATA_IN",
		"CURLINFO_SSL_DATA_OUT",
		"CURLINFO_END",
		NULL
	};

	HttpRequest* pSession = (HttpRequest*)pVSession;
	if(NULL == pSession) {
		return 0;
	}

	// 日志记录打开
	std::ofstream log;
	log.open(L"SpeedGetLog.log", std::ios::app);
	if (log.is_open()) {
		time_t t = time(NULL);
		struct tm tms;
		localtime_r(&t, &tms);
		log << "Time stamp: " << tms.tm_year + 1900 << "-" << tms.tm_mon + 1 << "-" << tms.tm_mday << " " << tms.tm_hour << ":" << tms.tm_min << ":" << tms.tm_sec << "\n";
		log << "Log session info: url = \"" << pSession->GetUrl() << "\", block begin: " << pSession->GetStartOffset() << ", block end: " << pSession->GetEndOffset() << "\n";
		log << "curl_infotype: " << curl_infotype_map[infotype] << "\n";
		if (infotype <= CURLINFO_HEADER_OUT) {
			char *datacpy = new char[data_size + 1];
			memcpy_s(datacpy, data_size + 1, data, data_size);
			datacpy[data_size] = 0;
			log << datacpy << "\n";
			delete [] datacpy;
		} else if (infotype <= CURLINFO_SSL_DATA_OUT) {
			log << "Data size: " << data_size << "\n";
		}
		log << "====================================================================\n\n";
		log.close();
	}
#endif //   OPEN_LOG
	return 0;	// must return 0.
}

int CHttpClientManager::ProgressFunc(void *pVSession, double dltotal, double dlnow, double ultotal, double ulnow) {

	HttpRequest* pSession = (HttpRequest*)pVSession;
	if(NULL == pSession) {
		return 0;
	}
	return pSession->OnProgress(dltotal, dlnow);
}

bool CHttpClientManager::Run() {

	int nRunningHandles = 0;
	CURLMcode code = CURLM_OK;
	do {
		code = (CURLMcode)m_curlMulti->Perform(&nRunningHandles);
		if(code == CURLM_CALL_MULTI_PERFORM) {
			Sleep(1);
			continue;
		}
	} while(false);

	
	timeval tv;
	int max_fd = -1;


	fd_set fd_read, fd_write, fd_except; 
	FD_ZERO(&fd_read); 
	FD_ZERO(&fd_write); 
	FD_ZERO(&fd_except); 

	m_curlMulti->Fdset(&fd_read, &fd_write, &fd_except, &max_fd);

	// max file descriptor is -1, so curl is busy and we need to wait
	if(-1 != max_fd) {

		long timeout; // will store the curl timeout
		m_curlMulti->Timeout(&timeout);
		if(timeout == -1) { // no timeout set?
			timeout = 100; // default to 100ms
		}
		// convert timeout to timeval to use it in select
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;

		if(select(max_fd + 1, &fd_read, &fd_write, &fd_except, &tv) < 0) {
#ifdef OPEN_LOG
			Log.outLog("HttpClient.log", "main: failed to select file descriptor %d : %",
				(int)max_fd + 1, strerror(errno));
#endif
		}
	}

	CURLMsg *info = NULL;
	int msgs_in_queue = 0;
	unsigned int uiReadCount = 0;
	do {
		info = m_curlMulti->InfoRead(&msgs_in_queue);
		if(NULL != info) {
			if(CURLMSG_DONE == info->msg) {
				HttpRequest* pRequest = NULL;
				curl_easy_getinfo(info->easy_handle, CURLINFO_PRIVATE, &pRequest);
				if(NULL != pRequest) {
					CURLcode result = info->data.result;
					m_curlMulti->RemoveHandle(info->easy_handle);
					pRequest->OnDone(result, curl_easy_strerror(result));
				} else {
					m_curlMulti->RemoveHandle(info->easy_handle);
					curl_easy_cleanup(info->easy_handle);
#ifdef OPEN_LOG
					Log.outLog("HttpClient.log", "Could not find session matched session");
#endif
				}
			}
		}
	} while (msgs_in_queue > 0 && ++uiReadCount < 10);

	return true;
}

} // end namespace ntwk

