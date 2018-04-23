/*
 * File:   CurlMultiMT.h
 * Author: Jehu Shaw
 *
 */

#ifndef _CURLMULTIMT_H
#define _CURLMULTIMT_H

#include "SpinLock.h"

typedef void CURLM;

struct CURLMsg;

namespace ntwk {

class CCurlMultiMT
{
public:

	CCurlMultiMT();

	~CCurlMultiMT();

	int Fdset(fd_set* read_fd_set, fd_set* write_fd_set, fd_set* exc_fd_set, int* max_fd);

	int Perform(int* running_handles);

	CURLMsg* InfoRead(int* msgs_in_queue);

	int AddHandle(CURLM* curl_handle);

	int RemoveHandle(CURLM* curl_handle);

	int Timeout(long* timeout);

private:

	thd::CSpinLock m_mutex;
	CURLM* m_multiHandle;
};

}

#endif /* _CURLMULTIMT_H */