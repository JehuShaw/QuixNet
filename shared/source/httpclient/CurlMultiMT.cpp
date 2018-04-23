#include "CurlMultiMT.h"
#include "ScopedLock.h"
#include "curl.h"

using namespace thd;

namespace ntwk {

CCurlMultiMT::CCurlMultiMT() : m_multiHandle(NULL)
{
	CScopedLock lock(m_mutex);
	m_multiHandle = curl_multi_init();
}

CCurlMultiMT::~CCurlMultiMT()
{
	CScopedLock lock(m_mutex);
	curl_multi_cleanup(m_multiHandle);
}

int CCurlMultiMT::Fdset(fd_set* read_fd_set, fd_set* write_fd_set, fd_set* exc_fd_set, int* max_fd)
{
	CScopedLock lock(m_mutex);
	return curl_multi_fdset(m_multiHandle, read_fd_set, write_fd_set, exc_fd_set, max_fd);
}

int CCurlMultiMT::Perform(int* running_handles)
{
	CScopedLock lock(m_mutex);
	return curl_multi_perform(m_multiHandle, running_handles);
}

CURLMsg* CCurlMultiMT::InfoRead(int* msgs_in_queue)
{
	CScopedLock lock(m_mutex);
	return curl_multi_info_read(m_multiHandle, msgs_in_queue);
}

int CCurlMultiMT::AddHandle(CURLM* curl_handle)
{
	CScopedLock lock(m_mutex);
	return curl_multi_add_handle(m_multiHandle, curl_handle);
}

int CCurlMultiMT::RemoveHandle(CURLM* curl_handle)
{
	CScopedLock lock(m_mutex);
	return curl_multi_remove_handle(m_multiHandle, curl_handle);
}

int CCurlMultiMT::Timeout(long* timeout)
{
	CScopedLock lock(m_mutex);
	return curl_multi_timeout(m_multiHandle, timeout);
}

} // end namespace ntwk

