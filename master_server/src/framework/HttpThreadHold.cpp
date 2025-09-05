/*
 * File:   HttpThreadHold.cpp
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */
#include "HttpThreadHold.h"
#include "HttpClientManager.h"

using namespace ntwk;

bool CHttpThreadHold::OnRun() {

	CHttpClientManager::PTR_T pHttpClientMgr(CHttpClientManager::Pointer());

	while(m_isStarted) {
		pHttpClientMgr->Run();
		Sleep(100);
	}
	return false;
}
