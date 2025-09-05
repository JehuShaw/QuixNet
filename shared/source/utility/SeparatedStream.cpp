#include "SeparatedStream.h"
#include <stdexcept>

util::CSeparatedStream::CSeparatedStream(
	char delim,
	char enclosure_delim /*= ','*/,
	SeparatedType sepdType /*= SEPDTYPE_FIRSTANDLAST_EMPTY*/,
	int allocBytes /*= StackAllocationSize*/)

	: m_data(m_stackData)
	, m_size(SEPDSTREAM_STACK_ALLOC_SIZE)
	, m_writeOffset(0)
	, m_readOffset(0)
	, m_bCopyData(true)
	, m_delim(delim)
	, m_enclosure_delim(enclosure_delim)
	, m_sepdType(sepdType)
	, m_moreData(false)
{
	if(delim == '\"' || m_enclosure_delim == '\"'
		|| delim == '-' || m_enclosure_delim == '-'
		|| delim == '+' || m_enclosure_delim == '+'
		|| delim == '.' || m_enclosure_delim == '.'
		|| (delim >= '0' && delim <= '9')
		|| (m_enclosure_delim >= '0' && m_enclosure_delim <= '9')
		|| (delim >= 'a' && delim <= 'z')
		|| (m_enclosure_delim >= 'a' && m_enclosure_delim <= 'z')
		|| (delim >= 'A' && delim <= 'Z')
		|| (m_enclosure_delim >= 'A' && m_enclosure_delim <= 'Z') )
	{
		throw std::invalid_argument("pass argument error !");
		return;
	}

	if(allocBytes > SEPDSTREAM_STACK_ALLOC_SIZE) {
		m_data = (char*) malloc(allocBytes);
		if(NULL == m_data) {
			m_size = 0;
		} else {
			m_data[0] = '\0';
			m_size = allocBytes;
		}
	} else {
		m_data[0] = '\0';
	}
#ifdef _DEBUG
	assert(m_data);
#endif
}

util::CSeparatedStream::CSeparatedStream(
	const char* szData,
	int nBytesLength,
	bool bCopyData,
	char delim,
	char enclosure_delim /*= ','*/,
	SeparatedType sepdType /*= SEPDTYPE_FIRSTANDLAST_EMPTY*/)

	: m_data(m_stackData)
	, m_size(SEPDSTREAM_STACK_ALLOC_SIZE)
	, m_writeOffset(0)
	, m_readOffset(0)
	, m_bCopyData(bCopyData)
	, m_delim(delim)
	, m_enclosure_delim(enclosure_delim)
	, m_sepdType(sepdType)
	, m_moreData(false)
{
	if(delim == '\"' || m_enclosure_delim == '\"'
		|| delim == '-' || m_enclosure_delim == '-'
		|| delim == '+' || m_enclosure_delim == '+'
		|| delim == '.' || m_enclosure_delim == '.'
		|| (delim >= '0' && delim <= '9')
		|| (m_enclosure_delim >= '0' && m_enclosure_delim <= '9')
		|| (delim >= 'a' && delim <= 'z')
		|| (m_enclosure_delim >= 'a' && m_enclosure_delim <= 'z')
		|| (delim >= 'A' && delim <= 'Z')
		|| (m_enclosure_delim >= 'A' && m_enclosure_delim <= 'Z') )
	{
		throw std::invalid_argument("pass argument error !");
		return;
	}

	if(m_bCopyData) {
		if(nBytesLength > 0 && NULL != szData) {
			int nNewSize = nBytesLength + 1;
			if(nNewSize > SEPDSTREAM_STACK_ALLOC_SIZE) {
				m_data = (char*) malloc(nNewSize);
				if(NULL == m_data) {
					m_size = 0;
				} else {
					m_size = nNewSize;
				}
			}
#ifdef _DEBUG
			assert(m_data);
#endif
			memcpy(m_data, szData, nBytesLength);
			m_data[nBytesLength] = '\0';
			m_writeOffset = nBytesLength;
			m_moreData = true;
		} else {
			m_stackData[0] = '\0';
		}
	} else {
		if(nBytesLength > 0 && NULL != szData) {
			m_data = const_cast<char*>(szData);
			m_size = nBytesLength + 1;
			m_writeOffset = nBytesLength;
			m_moreData = true;
		}
	}
}

util::CSeparatedStream::CSeparatedStream(
	const std::string& strData,
	bool bCopyData,
	char delim,
	char enclosure_delim /*= ','*/,
	SeparatedType sepdType /*= SEPDTYPE_FIRSTANDLAST_EMPTY*/)

	: m_data(m_stackData)
	, m_size(SEPDSTREAM_STACK_ALLOC_SIZE)
	, m_writeOffset(0)
	, m_readOffset(0)
	, m_bCopyData(bCopyData)
	, m_delim(delim)
	, m_enclosure_delim(enclosure_delim)
	, m_sepdType(sepdType)
	, m_moreData(false)
{
	if(delim == '\"' || m_enclosure_delim == '\"'
		|| delim == '-' || m_enclosure_delim == '-'
		|| delim == '+' || m_enclosure_delim == '+'
		|| delim == '.' || m_enclosure_delim == '.'
		|| (delim >= '0' && delim <= '9')
		|| (m_enclosure_delim >= '0' && m_enclosure_delim <= '9')
		|| (delim >= 'a' && delim <= 'z')
		|| (m_enclosure_delim >= 'a' && m_enclosure_delim <= 'z')
		|| (delim >= 'A' && delim <= 'Z')
		|| (m_enclosure_delim >= 'A' && m_enclosure_delim <= 'Z') )
	{
		throw std::invalid_argument("pass argument error !");
		return;
	}

	const char* szData = strData.c_str();
	int nBytesLength = (int)strData.length();

	if(bCopyData) {
		if(nBytesLength > 0 && NULL != szData) {
			int nNewSize = nBytesLength + 1;
			if(nNewSize > SEPDSTREAM_STACK_ALLOC_SIZE) {
				m_data = (char*) malloc(nNewSize);
				if(NULL == m_data) {
					m_size = 0;
				} else {
					m_size = nNewSize;
				}
			}
#ifdef _DEBUG
			assert(m_data);
#endif
			memcpy(m_data, szData, nBytesLength);
			m_data[nBytesLength] = '\0';
			m_writeOffset = nBytesLength;
			m_moreData = true;
		} else {
			m_stackData[0] = '\0';
		}
	} else {
		if(nBytesLength > 0 && NULL != szData) {
			m_data = const_cast<char*>(szData);
			m_size = nBytesLength + 1;
			m_writeOffset = nBytesLength;
			m_moreData = true;
		}
	}
}


util::CSeparatedStream::~CSeparatedStream() {
	if(m_bCopyData && m_stackData != m_data) {
		free(m_data);
		m_data = NULL;
	}
}

bool util::CSeparatedStream::AddBytesAndReallocate(const int numberOfBytes)
{
	if(numberOfBytes <= 0) {
		return false;
	}

	if(!m_bCopyData) {
#ifdef _DEBUG
		// If this assert hits then we need to specify true for the third parameter in the constructor
		// It needs to reallocate to hold all the data and can't do it unless we allocated to begin with
		assert(false);
#endif
		return false;
	}

	int newBytesAllocated = numberOfBytes + m_writeOffset;
	if(newBytesAllocated > 0 && m_size < newBytesAllocated) {
		// Less memory efficient but saves on news and deletes
		newBytesAllocated = (numberOfBytes + m_writeOffset) * 2;

		if(m_stackData == m_data) {
			if(newBytesAllocated > SEPDSTREAM_STACK_ALLOC_SIZE) {
				char* pTemp = (char*)malloc(newBytesAllocated);
				if(NULL != pTemp) {
					m_data = pTemp;
					// Need to copy the stack data to our new memory area
					memcpy((void *)m_data, (void *)m_stackData, m_size);
					m_size = newBytesAllocated;
					m_stackData[0] = '\0';
				} else {
#ifdef _DEBUG
					assert(false);
#endif
					return false;
				}
			}
		} else {
			char* pTemp = (char*)realloc(m_data, newBytesAllocated);
			if(NULL != pTemp) {
				m_data = pTemp;
				if(newBytesAllocated > m_size) {
					m_size = newBytesAllocated;
				}
			} else {
#ifdef _DEBUG
				assert(false);
#endif
				return false;
			}
		}
	}
	return true;
}

void util::CSeparatedStream::GetLine(const char*& szData, int& nBytesLength)
{
	m_moreData = false;
	if(m_readOffset >= m_writeOffset) {
		szData = m_data + m_readOffset;
		nBytesLength = 0;
		return;
	}
	bool bEnclosure = false;
	bool bHasDelim = false;

	const char* pStart = m_data + m_readOffset;
	if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
		|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
	{
		if(pStart == m_data && *pStart == m_delim) {
			++pStart;
			++m_readOffset;
		}
	}

	if(*pStart == m_delim) {
		// Fist check
		if(++m_readOffset == m_writeOffset) {
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_EMPTY == m_sepdType)
			{
				m_moreData = true;
			}
		} else {
			m_moreData = true;
		}
		szData = pStart;
		nBytesLength = 0;
		return;
	}

	if(*pStart == '\"') {
		bEnclosure = true;
	}

	int i = 1;
	int nUnreadSize = m_writeOffset - m_readOffset;
	if(nUnreadSize > 1 && pStart[i] == '\"') {
		bHasDelim = true;
		++i;
	}
	for(; i < nUnreadSize; ++i) {

		if(!bHasDelim && IsDelimChar(pStart[i])){
			bHasDelim = true;
		}
		if(pStart[i] == m_delim) {
			if(bEnclosure && (1 == i || pStart[i-1] != '\"')) {
				continue;
			}
			break;
		}
	}

	m_readOffset = (m_readOffset + i + 1);

	if(pStart[i] == m_delim) {
		if((i + 1) >= nUnreadSize) {
			// last check
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_EMPTY == m_sepdType)
			{
				m_moreData = true;
			} else {
				if((pStart[i + 1] == m_delim)) {
					m_moreData = true;
				}
			}
		} else {
			m_moreData = true;
		}
	}

	if(bEnclosure && bHasDelim) {
		szData = pStart + 1;
		if(i < 2) {
			nBytesLength = 0;
		} else {
			nBytesLength = i - 2;
		}
	} else {
		szData = pStart;
		nBytesLength = i;
	}
}





