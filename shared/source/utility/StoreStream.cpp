#include "StoreStream.h"
#include "TransferStream.h"

namespace util
{
	size_t GetStoreStreamSize(uint8_t nType) {
		if(STREAM_DATA_NIL < nType
			&& nType < STREAM_DATA_INT16)
		{
			return 1;
		} else if(STREAM_DATA_INT16 <= nType
			&& nType < STREAM_DATA_INT32)
		{
			return 2;
		} else if(STREAM_DATA_INT32 <= nType
			&& nType < STREAM_DATA_INT64)
		{
			return 4;
		} else if(STREAM_DATA_INT64 <= nType
			&& nType < STREAM_DATA_STD_STRING)
		{
			return 8;
		} else if(STREAM_DATA_STD_STRING <= nType
			&& nType < STREAM_DATA_SIZE)
		{
			return sizeof(CStoreObject);
		}
		return 0;
	}
}
//////////////////////////////////////////////////////////////////////////

util::CStoreObject::CStoreObject(int allocBytes /*= STOREOBJECT_MIN_ALLOC_SIZE*/)
	: m_length(0)
	, m_preOffset(-1)
	, m_nxtOffset(-1)
{
	if(allocBytes < STOREOBJECT_MIN_ALLOC_SIZE) {
		allocBytes = STOREOBJECT_MIN_ALLOC_SIZE;
	}
	m_data = (char*) malloc(allocBytes);
	if(NULL == m_data) {
		m_size = 0;
	} else {
		m_size = allocBytes;
	}
	assert(m_data);
}

void util::CStoreObject::WriteBytes(const char* input, const int numberOfBytes)
{
	if(NULL == input || numberOfBytes < 1) {
		m_length = 0;
		return;
	}
	AddBytesAndReallocate(numberOfBytes);
	memcpy(m_data, input, numberOfBytes);
	m_length = numberOfBytes;
}

void util::CStoreObject::AddBytesAndReallocate(const int numberOfBytes)
{
	if(numberOfBytes < 1) {
		assert(false);
		return;
	}

	int newBytesAllocated = numberOfBytes;
	if(newBytesAllocated > 0 && m_size < newBytesAllocated) {
		// Less memory efficient but saves on news and deletes
		 //newBytesAllocated = numberOfBytes * 2;

		char* pTemp = (char*)realloc(m_data, newBytesAllocated);
		if(NULL != pTemp) {
			m_data = pTemp;
			m_size = newBytesAllocated;
		} else {
			assert(false);
		}
	}
}

util::CStoreObject::~CStoreObject()
{
	free(m_data);
	m_data = NULL;
}

//////////////////////////////////////////////////////////////////////////

util::CStoreStream::CStoreStream(
	int allocBytes /*= STORESTREAM_STACK_ALLOC_SIZE*/)

	: m_data(m_stackData)
	, m_size(STORESTREAM_STACK_ALLOC_SIZE)
	, m_writeOffset(0)
	, m_readOffset(0)
	, m_bCopyData(true)
	, m_objOffset(-1)
{
	if(allocBytes > STORESTREAM_STACK_ALLOC_SIZE) {
		m_data = (char*) malloc(allocBytes);
		if(NULL == m_data) {
			m_size = 0;
		} else {
			m_size = allocBytes;
		}
	}
	assert(m_data);
}

util::CStoreStream::CStoreStream(
	const char* szData,
	int nBytesLength,
	bool bCopyData)

	: m_data(m_stackData)
	, m_size(STORESTREAM_STACK_ALLOC_SIZE)
	, m_writeOffset(0)
	, m_readOffset(0)
	, m_bCopyData(bCopyData)
	, m_objOffset(-1)
{
	if(m_bCopyData) {
		if(nBytesLength > 0 && NULL != szData) {

			if(nBytesLength > STORESTREAM_STACK_ALLOC_SIZE) {
				m_data = (char*) malloc(nBytesLength);
				if(NULL == m_data) {
					m_size = 0;
				} else {
					m_size = nBytesLength;
				}
			}
			assert(m_data);
			memcpy(m_data, szData, nBytesLength);
			m_writeOffset = nBytesLength;
		}
	} else {
		if(nBytesLength > 0 && NULL != szData) {
			m_data = const_cast<char*>(szData);
			m_size = nBytesLength;
			m_writeOffset = nBytesLength;
		}
	}
}

util::CStoreStream::CStoreStream(
	const std::string& strData,
	bool bCopyData)

	: m_data(m_stackData)
	, m_size(STORESTREAM_STACK_ALLOC_SIZE)
	, m_writeOffset(0)
	, m_readOffset(0)
	, m_bCopyData(bCopyData)
	, m_objOffset(-1)
{
	const char* szData = strData.c_str();
	int nBytesLength = (int)strData.length();
	if(m_bCopyData) {
		if(nBytesLength > 0) {

			if(nBytesLength > STORESTREAM_STACK_ALLOC_SIZE) {
				m_data = (char*) malloc(nBytesLength);
				if(NULL == m_data) {
					m_size = 0;
				} else {
					m_size = nBytesLength;
				}
			}
#ifdef _DEBUG
			assert(m_data);
#endif
			memcpy(m_data, szData, nBytesLength);
			m_writeOffset = nBytesLength;
		}
	} else {
		if(nBytesLength > 0) {
			m_data = const_cast<char*>(szData);
			m_size = nBytesLength;
			m_writeOffset = nBytesLength;
		}
	}
}

util::CStoreStream::CStoreStream(const util::CStoreStream & orig)

	: m_data(m_stackData)
	, m_size(STORESTREAM_STACK_ALLOC_SIZE)
	, m_writeOffset(0)
	, m_readOffset(0)
	, m_bCopyData(orig.m_bCopyData)
	, m_objOffset(-1)

{
	if(this == &orig) {
		return;
	}

	if(0 == orig.m_writeOffset) {
		return;
	}

	if(orig.m_bCopyData) {

		AddBytesAndReallocate(orig.m_writeOffset);
		memcpy(this->m_data, orig.m_data, orig.m_writeOffset);
		this->m_writeOffset = orig.m_writeOffset;

		CStoreObject* pObject = NULL;
		int curOffset = orig.m_objOffset;
		CStoreObject* curPtr = orig.ReadObjectType(curOffset);
		while(NULL != curPtr) {
			pObject = new (this->m_data + curOffset) CStoreObject();
			pObject->m_preOffset = curPtr->m_preOffset;
			pObject->m_nxtOffset = curPtr->m_nxtOffset;
			pObject->WriteBytes(curPtr->GetData(), curPtr->GetLength());

			curOffset = curPtr->m_preOffset;
			curPtr = orig.ReadObjectType(curOffset);
		}
		this->m_objOffset = orig.m_objOffset;
	} else {
		m_data = orig.m_data;
		m_size = orig.m_size;
		m_writeOffset = orig.m_writeOffset;
	}
}

util::CStoreStream::CStoreStream(CTransferStream& inStream)

	: m_data(m_stackData)
	, m_size(STORESTREAM_STACK_ALLOC_SIZE)
	, m_writeOffset(0)
	, m_readOffset(0)
	, m_bCopyData(true)
	, m_objOffset(-1)
{
	uint8_t nDestType = STREAM_DATA_NIL;

	while(inStream.GetNumberOfUnreadBits() > 7)
	{
		inStream >> nDestType;

		if(nDestType > STREAM_DATA_NIL && nDestType < STREAM_DATA_SIZE) {
			*(uint8_t*)(m_data + m_writeOffset) = nDestType;
			m_writeOffset += sizeof(uint8_t);
			(this->*s_typeOperators[nDestType].m_pWriteByType)(inStream, nDestType);
		} else {
			assert(false);
		}
	}
}

util::CStoreStream::~CStoreStream() {
	CStoreObject* curPtr = ReadObjectType(m_objOffset);
	while(NULL != curPtr) {
		curPtr->~CStoreObject();
		curPtr = ReadObjectType(curPtr->m_preOffset);
	}
	m_objOffset = -1;

	if(m_bCopyData && m_stackData != m_data) {
		free(m_data);
		m_data = NULL;
	}
}

util::CStoreStream & util::CStoreStream::operator = (const util::CStoreStream & right) {
	if(this == &right) {
		return *this;
	}

	// clear first
	Clear();

	if(0 == right.m_writeOffset) {
		return *this;
	}
	// copy
	if(right.m_bCopyData) {

		AddBytesAndReallocate(right.m_writeOffset);
		memcpy(this->m_data, right.m_data, right.m_writeOffset);
		this->m_writeOffset = right.m_writeOffset;

		CStoreObject* pObject = NULL;
		int curOffset = right.m_objOffset;
		CStoreObject* curPtr = right.ReadObjectType(curOffset);
		while(NULL != curPtr) {
			pObject = new (this->m_data + curOffset) CStoreObject();
			pObject->m_preOffset = curPtr->m_preOffset;
			pObject->m_nxtOffset = curPtr->m_nxtOffset;
			pObject->WriteBytes(curPtr->GetData(), curPtr->GetLength());

			curOffset = curPtr->m_preOffset;
			curPtr = right.ReadObjectType(curOffset);
		}
		this->m_objOffset = right.m_objOffset;
	} else {
		m_data = right.m_data;
		m_size = right.m_size;
		m_writeOffset = right.m_writeOffset;
	}
	return *this;
}

void util::CStoreStream::Reset(const char* szData, int nBytesLength, bool bCopyData)
{
	// The length can set zero

	CStoreObject* curPtr = ReadObjectType(m_objOffset);
	while(NULL != curPtr) {
		curPtr->~CStoreObject();
		curPtr = ReadObjectType(curPtr->m_preOffset);
	}
	m_objOffset = -1;
	if(m_bCopyData && m_stackData != m_data) {
		free(m_data);
	}
	m_data = m_stackData;
	m_size = STORESTREAM_STACK_ALLOC_SIZE;
	m_writeOffset = 0;
	m_readOffset = 0;
	m_bCopyData = true;

	if(NULL == szData || nBytesLength < 1) {
		return;
	}

	if(bCopyData) {
		AddBytesAndReallocate(nBytesLength);
		memcpy(m_data, szData, nBytesLength);
		m_writeOffset = nBytesLength;
	} else {
		m_data = const_cast<char*>(szData);
		m_size = nBytesLength;
		m_writeOffset = nBytesLength;
	}

}

// stream like operators for storing data
util::CStoreStream& util::CStoreStream::operator<<(char input)
{
#ifdef SS_DANGER_AREA_CHECKING
	CheckDangerArea(m_writeOffset);
#endif
	int nSize = sizeof(char);
	AddBytesAndReallocate(nSize);
	*(char*)(m_data + m_writeOffset) = input;
	m_writeOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator<<(bool input)
{
#ifdef SS_DANGER_AREA_CHECKING
	CheckDangerArea(m_writeOffset);
#endif
	int nSize = sizeof(bool);
	AddBytesAndReallocate(nSize);
	*(bool*)(m_data + m_writeOffset) = input;
	m_writeOffset += nSize;
	return *this;
}
// unsigned
util::CStoreStream& util::CStoreStream::operator<<(uint8_t input)
{
#ifdef SS_DANGER_AREA_CHECKING
	CheckDangerArea(m_writeOffset);
#endif
	int nSize = sizeof(uint8_t);
	AddBytesAndReallocate(nSize);
	*(uint8_t*)(m_data + m_writeOffset) = input;
	m_writeOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator<<(uint16_t input)
{
#ifdef SS_DANGER_AREA_CHECKING
	CheckDangerArea(m_writeOffset);
#endif
	int nSize = sizeof(uint16_t);
	AddBytesAndReallocate(nSize);
	*(uint16_t*)(m_data + m_writeOffset) = input;
	m_writeOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator<<(uint32_t input)
{
#ifdef SS_DANGER_AREA_CHECKING
	CheckDangerArea(m_writeOffset);
#endif
	int nSize = sizeof(uint32_t);
	AddBytesAndReallocate(nSize);
	*(uint32_t*)(m_data + m_writeOffset) = input;
	m_writeOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator<<(uint64_t input)
{
#ifdef SS_DANGER_AREA_CHECKING
	CheckDangerArea(m_writeOffset);
#endif
	int nSize = sizeof(uint64_t);
	AddBytesAndReallocate(nSize);
	*(uint64_t*)(m_data + m_writeOffset) = input;
	m_writeOffset += nSize;
	return *this;
}
// signed as in 2e complement
util::CStoreStream& util::CStoreStream::operator<<(int8_t input)
{
#ifdef SS_DANGER_AREA_CHECKING
	CheckDangerArea(m_writeOffset);
#endif
	int nSize = sizeof(int8_t);
	AddBytesAndReallocate(nSize);
	*(int8_t*)(m_data + m_writeOffset) = input;
	m_writeOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator<<(int16_t input)
{
#ifdef SS_DANGER_AREA_CHECKING
	CheckDangerArea(m_writeOffset);
#endif
	int nSize = sizeof(int16_t);
	AddBytesAndReallocate(nSize);
	*(int16_t*)(m_data + m_writeOffset) = input;
	m_writeOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator<<(int32_t input)
{
#ifdef SS_DANGER_AREA_CHECKING
	CheckDangerArea(m_writeOffset);
#endif
	int nSize = sizeof(int32_t);
	AddBytesAndReallocate(nSize);
	*(int32_t*)(m_data + m_writeOffset) = input;
	m_writeOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator<<(int64_t input)
{
#ifdef SS_DANGER_AREA_CHECKING
	CheckDangerArea(m_writeOffset);
#endif
	int nSize = sizeof(int64_t);
	AddBytesAndReallocate(nSize);
	*(int64_t*)(m_data + m_writeOffset) = input;
	m_writeOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator<<(float input)
{
#ifdef SS_DANGER_AREA_CHECKING
	CheckDangerArea(m_writeOffset);
#endif
	int nSize = sizeof(float);
	AddBytesAndReallocate(nSize);
	*(float*)(m_data + m_writeOffset) = input;
	m_writeOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator<<(double input)
{
#ifdef SS_DANGER_AREA_CHECKING
	CheckDangerArea(m_writeOffset);
#endif
	int nSize = sizeof(double);
	AddBytesAndReallocate(nSize);
	*(double*)(m_data + m_writeOffset) = input;
	m_writeOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator<<(const std::string& input)
{
	uint16_t length = (uint16_t)input.size();
	*this << length;

	if(length > 0){
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif
		AddBytesAndReallocate(length);
		memcpy((m_data + m_writeOffset), input.data(), length);
		m_writeOffset += length;
	}
	return *this;
}

void util::CStoreStream::WriteObject()
{
	int nSize = sizeof(CStoreObject);
	AddBytesAndReallocate(nSize);

	if(m_writeOffset > this->m_objOffset) {
		CStoreObject* pObject = new (m_data + m_writeOffset) CStoreObject();
		pObject->m_preOffset = this->m_objOffset;
		CStoreObject* prePtr = ReadObjectType(this->m_objOffset);
		if(NULL != prePtr) {
			prePtr->m_nxtOffset = m_writeOffset;
		}
		this->m_objOffset = m_writeOffset;
	} else {
		if(!IsObjectType(m_writeOffset)) {
			assert(false);
			return;
		}
	}

	m_writeOffset += nSize;
}

void util::CStoreStream::WriteObject(const util::CStoreStream& input)
{
	int nSize = sizeof(CStoreObject);

	CStoreObject* pObject = NULL;
	if(m_writeOffset > this->m_objOffset) {
		AddBytesAndReallocate(nSize);
		pObject = new (m_data + m_writeOffset) CStoreObject();
		pObject->m_preOffset = this->m_objOffset;
		CStoreObject* prePtr = ReadObjectType(this->m_objOffset);
		if(NULL != prePtr) {
			prePtr->m_nxtOffset = m_writeOffset;
		}
		this->m_objOffset = m_writeOffset;
	} else {
		if(!IsObjectType(m_writeOffset)) {
			assert(false);
			return;
		}
		pObject = (CStoreObject*)(m_data + m_writeOffset);
	}

	pObject->WriteBytes(input.GetData(), input.GetLength());

	m_writeOffset += nSize;
}

// stream like operators for reading data
util::CStoreStream& util::CStoreStream::operator>>(std::vector<bool>::reference output)
{
    int nSize = sizeof(bool);
	if(nSize + m_readOffset > m_writeOffset) {
		return *this;
	}
	output = *(bool*)(m_data + m_readOffset);
	m_readOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator>>(bool& output)
{
	int nSize = sizeof(bool);
	if(nSize + m_readOffset > m_writeOffset) {
		return *this;
	}
	output = *(bool*)(m_data + m_readOffset);
	m_readOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator>>(char& output)
{
	int nSize = sizeof(char);
	if(nSize + m_readOffset > m_writeOffset) {
		return *this;
	}
	output = *(char*)(m_data + m_readOffset);
	m_readOffset += nSize;
	return *this;
}
//unsigned
util::CStoreStream& util::CStoreStream::operator>>(uint8_t& output)
{
	int nSize = sizeof(uint8_t);
	if(nSize + m_readOffset > m_writeOffset) {
		return *this;
	}
	output = *(uint8_t*)(m_data + m_readOffset);
	m_readOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator>>(uint16_t& output)
{
	int nSize = sizeof(uint16_t);
	if(nSize + m_readOffset > m_writeOffset) {
		return *this;
	}
	output = *(uint16_t*)(m_data + m_readOffset);
	m_readOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator>>(uint32_t& output)
{
	int nSize = sizeof(uint32_t);
	if(nSize + m_readOffset > m_writeOffset) {
		return *this;
	}
	output = *(uint32_t*)(m_data + m_readOffset);
	m_readOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator>>(uint64_t& output)
{
	int nSize = sizeof(uint64_t);
	if(nSize + m_readOffset > m_writeOffset) {
		return *this;
	}
	output = *(uint64_t*)(m_data + m_readOffset);
	m_readOffset += nSize;
	return *this;
}
//signed as in 2e complement
util::CStoreStream& util::CStoreStream::operator>>(int8_t& output)
{
	int nSize = sizeof(int8_t);
	if(nSize + m_readOffset > m_writeOffset) {
		return *this;
	}
	output = *(int8_t*)(m_data + m_readOffset);
	m_readOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator>>(int16_t& output)
{
	int nSize = sizeof(int16_t);
	if(nSize + m_readOffset > m_writeOffset) {
		return *this;
	}
	output = *(int16_t*)(m_data + m_readOffset);
	m_readOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator>>(int32_t& output)
{
	int nSize = sizeof(int32_t);
	if(nSize + m_readOffset > m_writeOffset) {
		return *this;
	}
	output = *(int32_t*)(m_data + m_readOffset);
	m_readOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator>>(int64_t& output)
{
	int nSize = sizeof(int64_t);
	if(nSize + m_readOffset > m_writeOffset) {
		return *this;
	}
	output = *(int64_t*)(m_data + m_readOffset);
	m_readOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator>>(float& output)
{
	int nSize = sizeof(float);
	if(nSize + m_readOffset > m_writeOffset) {
		return *this;
	}
	output = *(float*)(m_data + m_readOffset);
	m_readOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator>>(double& output)
{
	int nSize = sizeof(double);
	if(nSize + m_readOffset > m_writeOffset) {
		return *this;
	}
	output = *(double*)(m_data + m_readOffset);
	m_readOffset += nSize;
	return *this;
}
util::CStoreStream& util::CStoreStream::operator>>(std::string& output)
{
	if(!output.empty()) {
		output.clear();
	}

	uint16_t length = 0;
	*this >> length;

	if(length < 1) {
		return *this;
	}

	if(length + m_readOffset > m_writeOffset) {
		return *this;
	}

	output.append((m_data + m_readOffset), length);
	m_readOffset += length;
	return *this;
}

void util::CStoreStream::ReadObject(int& readOffset, std::string& output) const
{
	int nSize = sizeof(util::CStoreObject);
	if(nSize + readOffset > m_writeOffset) {
		return;
	}
	
	if (IsObjectType(readOffset)) {
		CStoreObject* pObject = (CStoreObject*)(m_data + readOffset);
		readOffset += nSize;
		CStoreStream object(pObject->GetData(), pObject->GetLength(), false);
		object >> output;
	} else {
		assert(false);
	}
}

void util::CStoreStream::ReadObject(int& readOffset, util::CStoreStream& output) const
{
	int nSize = sizeof(util::CStoreObject);
	if(nSize + readOffset > m_writeOffset) {
		return;
	}

	if (IsObjectType(readOffset)) {
	
		CStoreObject* pObject = (CStoreObject*)(m_data + readOffset);
		readOffset += nSize;
		output.Reset(pObject->GetData(), pObject->GetLength(), true);
	} else {
		assert(false);
	}
}

void util::CStoreStream::AddBytesAndReallocate(const int numberOfBytes)
{
	if(numberOfBytes < 1) {
		assert(false);
		return;
	}

	if(!m_bCopyData) {
		// If this assert hits then we need to specify true for the third parameter in the constructor
		// It needs to reallocate to hold all the data and can't do it unless we allocated to begin with
		assert(false);
		return;
	}

	int newBytesAllocated = numberOfBytes + m_writeOffset;
	if(newBytesAllocated > 0 && m_size < newBytesAllocated) {
		// Less memory efficient but saves on news and deletes
		newBytesAllocated = (numberOfBytes + m_writeOffset) * 2;

		if(m_stackData == m_data) {
			if(newBytesAllocated > STORESTREAM_STACK_ALLOC_SIZE) {
				char* pTemp = (char*)malloc(newBytesAllocated);
				if(NULL != pTemp) {
					m_data = pTemp;
					// Need to copy the stack data to our new memory area
					memcpy((void *)m_data, (void *)m_stackData, m_size);
					m_size = newBytesAllocated;
					m_stackData[0] = '\0';
				} else {
					assert(false);
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
				assert(false);
			}
		}
	}
}

bool util::CStoreStream::IsObjectArea(int pos)
{
	int nCurOffset = m_objOffset;
	CStoreObject* curPtr = ReadObjectType(nCurOffset);
	while(NULL != curPtr) {
		if(pos >= nCurOffset && pos
			< (nCurOffset + (int)sizeof(CStoreObject)))
		{
			return true;
		}
		nCurOffset = curPtr->m_preOffset;
		curPtr = ReadObjectType(nCurOffset);
	}
	return false;
}

void util::CStoreStream::Parse(CTransferStream& inStream)
{
	uint8_t nThisType = STREAM_DATA_NIL;
	uint8_t nDestType = STREAM_DATA_NIL;
	int nWriteOffset = this->GetWriteOffset();
	this->SetWriteOffset(0);
	inStream.SetReadOffset(0);
	while(inStream.GetNumberOfUnreadBits() > 7)
	{
		inStream >> nDestType;
		nThisType = *(uint8_t*)(m_data + m_writeOffset);

		if((m_writeOffset + (int)sizeof(uint8_t)) <= nWriteOffset && IsFixedSizeType(nThisType)) {
			m_writeOffset += sizeof(uint8_t);
			(this->*s_typeOperators[nThisType].m_pWriteByType)(inStream, nDestType);
		} else {
			if(nDestType > STREAM_DATA_NIL && nDestType < STREAM_DATA_SIZE) {
				*this << nDestType;
				(this->*s_typeOperators[nDestType].m_pWriteByType)(inStream, nDestType);
			} else {
				assert(false);
			}
		}
	}
}

void util::CStoreStream::Serialize(CTransferStream& outStream) const
{
	outStream.SetWriteOffset(0);
	uint8_t nThisType = STREAM_DATA_NIL;
	int readOffset = 0;
	while(readOffset < m_writeOffset)
	{
		if(readOffset + (int)sizeof(uint8_t) > m_writeOffset) {
			assert(false);
			nThisType = STREAM_DATA_NIL;
		} else {
			nThisType = *(uint8_t*)(m_data + readOffset);
			readOffset += sizeof(uint8_t);
		}
		if(nThisType > STREAM_DATA_NIL && nThisType < STREAM_DATA_SIZE) {
			(this->*s_typeOperators[nThisType].m_pReadByType)(readOffset, outStream, true);
		} else {
			assert(false);
		}
	}
}

void util::CStoreStream::Serialize(CTransferStream& outStream, const std::vector<bool>& updateFlags) const
{
	outStream.SetWriteOffset(0);
	int nIndex = 0;
	bool bUpdate = false;
	uint8_t nThisType = STREAM_DATA_NIL;
	int readOffset = 0;

	while(readOffset < m_writeOffset)
	{
		if(nIndex < (int)updateFlags.size()) {
			bUpdate = updateFlags[nIndex++];
		} else {
			bUpdate = false;
		}
		if(readOffset + (int)sizeof(uint8_t) > m_writeOffset) {
			assert(false);
			nThisType = STREAM_DATA_NIL;
		} else {
			nThisType = *(uint8_t*)(m_data + readOffset);
			readOffset += sizeof(uint8_t);
		}
		if(nThisType > STREAM_DATA_NIL && nThisType < STREAM_DATA_SIZE) {
			(this->*s_typeOperators[nThisType].m_pReadByType)(readOffset, outStream, bUpdate);
		} else {
			assert(false);
		}
	}
}

void util::CStoreStream::ParseResetUpdate(CTransferStream& inStream)
{
	uint8_t nThisType = STREAM_DATA_NIL;
	uint8_t nDestType = STREAM_DATA_NIL;
	int nWriteOffset = this->GetWriteOffset();
	this->SetWriteOffset(0);
	inStream.SetReadOffset(0);
	while(inStream.GetNumberOfUnreadBits() > 7)
	{
		inStream >> nDestType;
		nThisType = *(uint8_t*)(m_data + m_writeOffset);

		if((m_writeOffset + (int)sizeof(uint8_t)) <= nWriteOffset && IsFixedSizeType(nThisType)) {
			*this << ToIgnoreType(nThisType);
			(this->*s_typeOperators[nThisType].m_pWriteByType)(inStream, nDestType);
		} else {
			if(nDestType > STREAM_DATA_NIL && nDestType < STREAM_DATA_SIZE) {
				*this << ToIgnoreType(nDestType);
				(this->*s_typeOperators[nDestType].m_pWriteByType)(inStream, nDestType);
			} else {
				assert(false);
			}
		}
	}
}

void util::CStoreStream::ParseSetUpdate(CTransferStream& inStream)
{
	uint8_t nThisType = STREAM_DATA_NIL;
	uint8_t nDestType = STREAM_DATA_NIL;
	int nWriteOffset = this->GetWriteOffset();
	this->SetWriteOffset(0);
	inStream.SetReadOffset(0);
	while(inStream.GetNumberOfUnreadBits() > 7)
	{
		inStream >> nDestType;
		nThisType = *(uint8_t*)(m_data + m_writeOffset);

		if((m_writeOffset + (int)sizeof(uint8_t)) <= nWriteOffset && IsFixedSizeType(nThisType)) {
			if(IsUpdateType(nDestType)) {
				*this << ToUpdateType(nThisType);
			} else {
				m_writeOffset += sizeof(uint8_t);
			}
			(this->*s_typeOperators[nThisType].m_pWriteByType)(inStream, nDestType);
		} else {
			if(nDestType > STREAM_DATA_NIL && nDestType < STREAM_DATA_SIZE) {
				*this << nDestType;
				(this->*s_typeOperators[nDestType].m_pWriteByType)(inStream, nDestType);
			} else {
				assert(false);
			}
		}
	}
}

void util::CStoreStream::SerializeResetUpdate(std::vector<CTypeString>& outStrings) const
{
	uint8_t nThisType = STREAM_DATA_NIL;
	int readOffset = 0;
	while(readOffset < m_writeOffset)
	{
		outStrings.resize(outStrings.size() + 1);
		CTypeString& typeString = outStrings.back();

		int typeOffset = readOffset;
		if(readOffset + (int)sizeof(uint8_t) > m_writeOffset) {
			assert(false);
			nThisType = STREAM_DATA_NIL;
		} else {
			nThisType = *(uint8_t*)(m_data + readOffset);
			readOffset += sizeof(uint8_t);
		}
		typeString.m_u8Type = nThisType;

		if(IsIgnoreType(nThisType)) {
			size_t nIgnoreSize = GetStoreStreamSize(nThisType);
			assert(0 != nIgnoreSize);
			readOffset += nIgnoreSize;
			continue;
		}

		if(nThisType > STREAM_DATA_NIL && nThisType < STREAM_DATA_SIZE) {
			(this->*s_typeOperators[nThisType].m_pReadToString)(readOffset, typeString.m_str);
		} else {
			assert(false);
		}

		*(uint8_t*)(m_data + typeOffset) = ToIgnoreType(nThisType);
	}
}

void util::CStoreStream::RecoverUpdate(const std::vector<CTypeString>& inStrings)
{
	uint8_t nThisType = STREAM_DATA_NIL;
	uint8_t nDestType = STREAM_DATA_NIL;
	int nWriteOffset = 0;

	int nSize = (int)inStrings.size();
	for(int i = 0; i < nSize && nWriteOffset < m_writeOffset; ++i)
	{
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(nWriteOffset);
#endif
		nThisType = *(uint8_t*)(m_data + nWriteOffset);
		nDestType = inStrings[i].GetType();

		if(IsUpdateType(nDestType)) {
			*(uint8_t*)(m_data + nWriteOffset) = nDestType;
		}

		size_t nIgnoreSize = GetStoreStreamSize(nThisType);
		assert(0 != nIgnoreSize);

		nWriteOffset = nWriteOffset + sizeof(uint8_t) + nIgnoreSize;
	}
}

void util::CStoreStream::ReadByType(CTransferStream& outStream) const
{
	uint8_t nThisType = STREAM_DATA_NIL;
	int readOffset = 0;
	while(readOffset < m_writeOffset)
	{
		if(readOffset + (int)sizeof(uint8_t) > m_writeOffset) {
			assert(false);
			nThisType = STREAM_DATA_NIL;
		} else {
			nThisType = *(uint8_t*)(m_data + readOffset);
			readOffset += sizeof(uint8_t);
		}
		if(nThisType > STREAM_DATA_NIL && nThisType < STREAM_DATA_SIZE) {
			(this->*s_typeOperators[nThisType].m_pReadByType)(readOffset, outStream, true);
		} else {
			assert(false);
		}
	}
}

//void util::CStoreStream::WriteFromTypeString(const std::vector<CTypeString>& inStrings)
//{
//	uint8_t nDestType = STREAM_DATA_NIL;
//
//	int nSize = (int)inStrings.size();
//	for(int i = 0; i < nSize; ++i)
//	{
//		nDestType = inStrings[i].GetType();
//		const std::string& strInput = inStrings[i].GetString();
//
//		if(nDestType > STREAM_DATA_NIL && nDestType < STREAM_DATA_SIZE) {
//			*this << nDestType;
//			(this->*s_typeOperators[nDestType].m_pWriteFromString)(strInput.data(), strInput.length(), nDestType);
//		} else {
//			assert(false);
//		}
//	}
//}

//void util::CStoreStream::ReadToTypeString(std::vector<CTypeString>& outStrings)
//{
//	uint8_t nThisType = STREAM_DATA_NIL;
//	while(this->GetUnreadSize() > 0)
//	{
//		outStrings.resize(outStrings.size() + 1);
//		CTypeString& typeString = outStrings.back();
//
//		*this >> nThisType;
//		typeString.m_u8Type = nThisType;
//
//		if(nThisType > STREAM_DATA_NIL && nThisType < STREAM_DATA_SIZE) {
//			(this->*s_typeOperators[nThisType].m_pReadToString)(typeString.m_str);
//		} else {
//			assert(false);
//		}
//	}
//}

//void util::CStoreStream::WriteFromTypeString(const char* szInput, int length, uint8_t nDestType)
//{
//	if(nDestType > STREAM_DATA_NIL && nDestType < STREAM_DATA_SIZE) {
//		*this << nDestType;
//		(this->*s_typeOperators[nDestType].m_pWriteFromString)(szInput, length, nDestType);
//	} else {
//		assert(false);
//	}
//}

//void util::CStoreStream::WriteFromTypeString(const CTypeString& inString)
//{
//	uint8_t nDestType = inString.GetType();
//	const std::string& strInput = inString.GetString();
//
//	if(nDestType > STREAM_DATA_NIL && nDestType < STREAM_DATA_SIZE) {
//		*this << nDestType;
//		(this->*s_typeOperators[nDestType].m_pWriteFromString)(strInput.data(), strInput.length(), nDestType);
//	} else {
//		assert(false);
//	}
//}

//void util::CStoreStream::ReadToTypeString(CTypeString& outString)
//{
//	uint8_t nThisType = STREAM_DATA_NIL;
//	*this >> nThisType;
//	outString.m_u8Type = nThisType;
//
//	if(nThisType > STREAM_DATA_NIL && nThisType < STREAM_DATA_SIZE) {
//		(this->*s_typeOperators[nThisType].m_pReadToString)(outString.m_str);
//	} else {
//		assert(false);
//	}
//}
//////////////////////////////////////////////////////////////////////////
void util::CStoreStream::WriteCharByType(CTransferStream& stream, uint8_t nType)
{
	int nSize = sizeof(char);
	if(IsUpdateType(nType)) {
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif
		AddBytesAndReallocate(nSize);
		stream.ReadByType(*(char*)(m_data + m_writeOffset), nType);
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteBoolByType(CTransferStream& stream, uint8_t nType)
{
	int nSize = sizeof(bool);
	if(IsUpdateType(nType)) {
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif
		AddBytesAndReallocate(nSize);
		stream.ReadByType(*(bool*)(m_data + m_writeOffset), nType);
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteUInt8ByType(CTransferStream& stream, uint8_t nType)
{
	int nSize = sizeof(uint8_t);
	if(IsUpdateType(nType)) {
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif
		AddBytesAndReallocate(nSize);
		stream.ReadByType(*(uint8_t*)(m_data + m_writeOffset), nType);
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteUInt16ByType(CTransferStream& stream, uint8_t nType)
{
	int nSize = sizeof(uint16_t);
	if(IsUpdateType(nType)) {
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif
		AddBytesAndReallocate(nSize);
		stream.ReadByType(*(uint16_t*)(m_data + m_writeOffset), nType);
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteUInt32ByType(CTransferStream& stream, uint8_t nType)
{
	int nSize = sizeof(uint32_t);
	if(IsUpdateType(nType)) {
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif
		AddBytesAndReallocate(nSize);
		stream.ReadByType(*(uint32_t*)(m_data + m_writeOffset), nType);
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteUInt64ByType(CTransferStream& stream, uint8_t nType)
{
	int nSize = sizeof(uint64_t);
	if(IsUpdateType(nType)) {
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif
		AddBytesAndReallocate(nSize);
		stream.ReadByType(*(uint64_t*)(m_data + m_writeOffset), nType);
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteInt8ByType(CTransferStream& stream, uint8_t nType)
{
	int nSize = sizeof(int8_t);
	if(IsUpdateType(nType)) {
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif
		AddBytesAndReallocate(nSize);
		stream.ReadByType(*(int8_t*)(m_data + m_writeOffset), nType);
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteInt16ByType(CTransferStream& stream, uint8_t nType)
{
	int nSize = sizeof(int16_t);
	if(IsUpdateType(nType)) {
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif
		AddBytesAndReallocate(nSize);
		stream.ReadByType(*(int16_t*)(m_data + m_writeOffset), nType);
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteInt32ByType(CTransferStream& stream, uint8_t nType)
{
	int nSize = sizeof(int32_t);
	if(IsUpdateType(nType)) {
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif
		AddBytesAndReallocate(nSize);
		stream.ReadByType(*(int32_t*)(m_data + m_writeOffset), nType);
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteInt64ByType(CTransferStream& stream, uint8_t nType)
{
	int nSize = sizeof(int64_t);
	if(IsUpdateType(nType)) {
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif
		AddBytesAndReallocate(nSize);
		stream.ReadByType(*(int64_t*)(m_data + m_writeOffset), nType);
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteFloatByType(CTransferStream& stream, uint8_t nType)
{
	int nSize = sizeof(float);
	if(IsUpdateType(nType)) {
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif
		AddBytesAndReallocate(nSize);
		stream.ReadByType(*(float*)(m_data + m_writeOffset), nType);
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteDoubleByType(CTransferStream& stream, uint8_t nType)
{
	int nSize = sizeof(double);
	if(IsUpdateType(nType)) {
#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif
		AddBytesAndReallocate(nSize);
		stream.ReadByType(*(double*)(m_data + m_writeOffset), nType);
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteStringByType(CTransferStream& stream, uint8_t nType)
{
	assert(IsStringType(nType));

	if(IsUpdateType(nType)) {

		uint16_t len = 0;
		stream >> len;

		CStoreStream object(len + sizeof(uint16_t));
		object << len;

		if(len > 0) {
			stream.ReadBytes((char*)(object.GetData() + object.GetWriteOffset()), len);
			object.m_writeOffset += len;
		}

		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteBoolSetByType(CTransferStream& stream, uint8_t nType) {

	assert(IsContainerType(nType));

	if(IsUpdateType(nType)) {

		int32_t length = 0;
		stream >> length;
		if(length < 0) {
			length = 0;
		}

		uint8_t nInnerType = GetInnerType(nType);

		CStoreStream object(sizeof(bool) * length + sizeof(int32_t));
		object << length;

		for(int i = 0; i < length; ++i) {
			object.WriteBoolByType(stream, nInnerType);
		}
		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteUInt8SetByType(CTransferStream& stream, uint8_t nType) {

	assert(IsContainerType(nType));

	if(IsUpdateType(nType)) {

		int32_t length = 0;
		stream >> length;
		if(length < 0) {
			length = 0;
		}

		uint8_t nInnerType = GetInnerType(nType);

		CStoreStream object(sizeof(uint8_t) * length + sizeof(int32_t));
		object << length;

		for(int i = 0; i < length; ++i) {
			object.WriteUInt8ByType(stream, nInnerType);
		}
		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteInt8SetByType(CTransferStream& stream, uint8_t nType) {

	assert(IsContainerType(nType));

	if(IsUpdateType(nType)) {

		int32_t length = 0;
		stream >> length;
		if(length < 0) {
			length = 0;
		}

		uint8_t nInnerType = GetInnerType(nType);

		CStoreStream object(sizeof(int8_t) * length + sizeof(int32_t));
		object << length;

		for(int i = 0; i < length; ++i) {
			object.WriteInt8ByType(stream, nInnerType);
		}
		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteUInt16SetByType(CTransferStream& stream, uint8_t nType) {

	assert(IsContainerType(nType));

	if(IsUpdateType(nType)) {

		int32_t length = 0;
		stream >> length;
		if(length < 0) {
			length = 0;
		}

		uint8_t nInnerType = GetInnerType(nType);

		CStoreStream object(sizeof(uint16_t) * length + sizeof(int32_t));
		object << length;

		for(int i = 0; i < length; ++i) {
			object.WriteUInt16ByType(stream, nInnerType);
		}
		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteInt16SetByType(CTransferStream& stream, uint8_t nType) {

	assert(IsContainerType(nType));

	if(IsUpdateType(nType)) {

		int32_t length = 0;
		stream >> length;
		if(length < 0) {
			length = 0;
		}

		uint8_t nInnerType = GetInnerType(nType);

		CStoreStream object(sizeof(int16_t) * length + sizeof(int32_t));
		object << length;

		for(int i = 0; i < length; ++i) {
			object.WriteInt16ByType(stream, nInnerType);
		}
		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteUInt32SetByType(CTransferStream& stream, uint8_t nType) {

	assert(IsContainerType(nType));

	if(IsUpdateType(nType)) {

		int32_t length = 0;
		stream >> length;
		if(length < 0) {
			length = 0;
		}

		uint8_t nInnerType = GetInnerType(nType);

		CStoreStream object(sizeof(uint32_t) * length + sizeof(int32_t));
		object << length;

		for(int i = 0; i < length; ++i) {
			object.WriteUInt32ByType(stream, nInnerType);
		}
		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteInt32SetByType(CTransferStream& stream, uint8_t nType) {

	assert(IsContainerType(nType));

	if(IsUpdateType(nType)) {

		int32_t length = 0;
		stream >> length;
		if(length < 0) {
			length = 0;
		}

		uint8_t nInnerType = GetInnerType(nType);

		CStoreStream object(sizeof(int32_t) * length + sizeof(int32_t));
		object << length;

		for(int i = 0; i < length; ++i) {
			object.WriteInt32ByType(stream, nInnerType);
		}
		WriteObject(object);
	} else {
		WriteObject();
	}
}

#ifndef NO_INT64
void util::CStoreStream::WriteUInt64SetByType(CTransferStream& stream, uint8_t nType) {

	assert(IsContainerType(nType));

	if(IsUpdateType(nType)) {

		int32_t length = 0;
		stream >> length;
		if(length < 0) {
			length = 0;
		}

		uint8_t nInnerType = GetInnerType(nType);

		CStoreStream object(sizeof(uint64_t) * length + sizeof(int32_t));
		object << length;

		for(int i = 0; i < length; ++i) {
			object.WriteUInt64ByType(stream, nInnerType);
		}
		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteInt64SetByType(CTransferStream& stream, uint8_t nType) {

	assert(IsContainerType(nType));

	if(IsUpdateType(nType)) {

		int32_t length = 0;
		stream >> length;
		if(length < 0) {
			length = 0;
		}

		uint8_t nInnerType = GetInnerType(nType);

		CStoreStream object(sizeof(int64_t) * length + sizeof(int32_t));
		object << length;

		for(int i = 0; i < length; ++i) {
			object.WriteInt64ByType(stream, nInnerType);
		}
		WriteObject(object);
	} else {
		WriteObject();
	}
}
#endif

void util::CStoreStream::WriteFloatSetByType(CTransferStream& stream, uint8_t nType) {

	assert(IsContainerType(nType));

	if(IsUpdateType(nType)) {

		int32_t length = 0;
		stream >> length;
		if(length < 0) {
			length = 0;
		}

		uint8_t nInnerType = GetInnerType(nType);

		CStoreStream object(sizeof(float) * length + sizeof(int32_t));
		object << length;

		for(int i = 0; i < length; ++i) {
			object.WriteFloatByType(stream, nInnerType);
		}
		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteDoubleSetByType(CTransferStream& stream, uint8_t nType) {

	assert(IsContainerType(nType));

	if(IsUpdateType(nType)) {

		int32_t length = 0;
		stream >> length;
		if(length < 0) {
			length = 0;
		}

		uint8_t nInnerType = GetInnerType(nType);

		CStoreStream object(sizeof(double) * length + sizeof(int32_t));
		object << length;

		for(int i = 0; i < length; ++i) {
			object.WriteDoubleByType(stream, nInnerType);
		}
		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteStringSetByType(CTransferStream& stream, uint8_t nType) {

	assert(IsContainerType(nType));
	uint8_t nInnerType = GetInnerType(nType);
	assert(IsStringType(nInnerType));

	if(IsUpdateType(nType)) {

		int32_t length = 0;
		stream >> length;
		if(length < 0) {
			length = 0;
		}

		uint32_t offset = stream.GetReadOffset();
		int32_t sumLen = 0;
		uint16_t len = 0;
		for(int j = 0; j < length; ++j) {
			stream >> len;
			sumLen += sizeof(uint16_t);
			sumLen += len;
			stream.IgnoreBits(TS_BYTES_TO_BITS(len));
		}
		stream.SetReadOffset(offset);

		CStoreStream object(sumLen * length + sizeof(int32_t));
		object << length;

		for(int i = 0; i < length; ++i) {
			stream >> len;
			object << len;
			object.AddBytesAndReallocate(len);
			stream.ReadBytes((char*)(object.m_data + object.m_writeOffset), len);
			object.m_writeOffset += len;
		}

		WriteObject(object);
	} else {
		WriteObject();
	}
}

//////////////////////////////////////////////////////////////////////////
void util::CStoreStream::ReadCharByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	int nSize = sizeof(char);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}
	stream.Serialize(*(char*)(m_data + readOffset), bChange);
	readOffset += nSize;
}

void util::CStoreStream::ReadBoolByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	int nSize = sizeof(bool);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}
	stream.Serialize(*(bool*)(m_data + readOffset), bChange);
	readOffset += nSize;
}

void util::CStoreStream::ReadUInt8ByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	int nSize = sizeof(uint8_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}
	stream.Serialize(*(uint8_t*)(m_data + readOffset), bChange);
	readOffset += nSize;
}

void util::CStoreStream::ReadUInt16ByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	int nSize = sizeof(uint16_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}
	stream.Serialize(*(uint16_t*)(m_data + readOffset), bChange);
	readOffset += nSize;
}

void util::CStoreStream::ReadUInt32ByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	int nSize = sizeof(uint32_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}
	stream.Serialize(*(uint32_t*)(m_data + readOffset), bChange);
	readOffset += nSize;
}

void util::CStoreStream::ReadUInt64ByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	int nSize = sizeof(uint64_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}
	stream.Serialize(*(uint64_t*)(m_data + readOffset), bChange);
	readOffset += nSize;
}

void util::CStoreStream::ReadInt8ByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	int nSize = sizeof(int8_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}
	stream.Serialize(*(int8_t*)(m_data + readOffset), bChange);
	readOffset += nSize;
}

void util::CStoreStream::ReadInt16ByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	int nSize = sizeof(int16_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}
	stream.Serialize(*(int16_t*)(m_data + readOffset), bChange);
	readOffset += nSize;
}

void util::CStoreStream::ReadInt32ByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	int nSize = sizeof(int32_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}
	stream.Serialize(*(int32_t*)(m_data + readOffset), bChange);
	readOffset += nSize;
}

void util::CStoreStream::ReadInt64ByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	int nSize = sizeof(int64_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}
	stream.Serialize(*(int64_t*)(m_data + readOffset), bChange);
	readOffset += nSize;
}

void util::CStoreStream::ReadFloatByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	int nSize = sizeof(float);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}
	stream.Serialize(*(float*)(m_data + readOffset), bChange);
	readOffset += nSize;
}

void util::CStoreStream::ReadDoubleByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	int nSize = sizeof(double);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}
	stream.Serialize(*(double*)(m_data + readOffset), bChange);
	readOffset += nSize;
}

void util::CStoreStream::ReadStringByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	std::string str;
	ReadObject(readOffset, str);
	stream.Serialize(str, bChange);
}

void util::CStoreStream::ReadBoolSetByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	std::vector<bool> object;
	ReadObject(readOffset, object);
	stream.Serialize(object, bChange);
}

void util::CStoreStream::ReadUInt8SetByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	std::vector<uint8_t> object;
	ReadObject(readOffset, object);
	stream.Serialize(object, bChange);
}

void util::CStoreStream::ReadInt8SetByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	std::vector<int8_t> object;
	ReadObject(readOffset, object);
	stream.Serialize(object, bChange);
}

void util::CStoreStream::ReadUInt16SetByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	std::vector<uint16_t> object;
	ReadObject(readOffset, object);
	stream.Serialize(object, bChange);
}

void util::CStoreStream::ReadInt16SetByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	std::vector<int16_t> object;
	ReadObject(readOffset, object);
	stream.Serialize(object, bChange);
}

void util::CStoreStream::ReadUInt32SetByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	std::vector<uint32_t> object;
	ReadObject(readOffset, object);
	stream.Serialize(object, bChange);
}

void util::CStoreStream::ReadInt32SetByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	std::vector<int32_t> object;
	ReadObject(readOffset, object);
	stream.Serialize(object, bChange);
}

#ifndef NO_INT64
void util::CStoreStream::ReadUInt64SetByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	std::vector<uint64_t> object;
	ReadObject(readOffset, object);
	stream.Serialize(object, bChange);
}

void util::CStoreStream::ReadInt64SetByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	std::vector<int64_t> object;
	ReadObject(readOffset, object);
	stream.Serialize(object, bChange);
}
#endif

void util::CStoreStream::ReadFloatSetByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	std::vector<float> object;
	ReadObject(readOffset, object);
	stream.Serialize(object, bChange);
}

void util::CStoreStream::ReadDoubleSetByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	std::vector<double> object;
	ReadObject(readOffset, object);
	stream.Serialize(object, bChange);
}

void util::CStoreStream::ReadStringSetByType(int& readOffset, CTransferStream& stream, bool bChange) const
{
	std::vector<std::string> object;
	ReadObject(readOffset, object);
	stream.Serialize(object, bChange);
}
//////////////////////////////////////////////////////////////////////////
void util::CStoreStream::WriteCharFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsSingleType(nType));

	int nSize = sizeof(char);

	if(NULL != szInput) {

#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif

		AddBytesAndReallocate(nSize);

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		switch(nType) {
		case STREAM_DATA_CHAR:
		case STREAM_DATA_CHAR_NULL:
			{
				separated >> *(char*)(m_data + m_writeOffset);
			}
			break;
		case STREAM_DATA_BOOL:
		case STREAM_DATA_BOOL_NULL:
			{
				bool temp = false;
				separated >> temp;
				*(char*)(m_data + m_writeOffset) = (char)temp;
			}
			break;
		case STREAM_DATA_INT8:
		case STREAM_DATA_INT8_NULL:
			{
				int8_t temp = 0;
				separated >> temp;
				*(char*)(m_data + m_writeOffset) = (char)temp;
			}
			break;
		case STREAM_DATA_UINT8:
		case STREAM_DATA_UINT8_NULL:
			{
				uint8_t temp = 0;
				separated >> temp;
				*(char*)(m_data + m_writeOffset) = (char)temp;
			}
			break;
		case STREAM_DATA_INT16:
		case STREAM_DATA_INT16_NULL:
			{
				int16_t temp = 0;
				separated >> temp;
				*(char*)(m_data + m_writeOffset) = (char)temp;
			}
			break;
		case STREAM_DATA_UINT16:
		case STREAM_DATA_UINT16_NULL:
			{
				uint16_t temp = 0;
				separated >> temp;
				*(char*)(m_data + m_writeOffset) = (char)temp;
			}
			break;
		case STREAM_DATA_INT32:
		case STREAM_DATA_INT32_NULL:
			{
				int32_t temp = 0;
				separated >> temp;
				*(char*)(m_data + m_writeOffset) = (char)temp;
			}
			break;
		case STREAM_DATA_UINT32:
		case STREAM_DATA_UINT32_NULL:
			{
				uint32_t temp = 0;
				separated >> temp;
				*(char*)(m_data + m_writeOffset) = (char)temp;
			}
			break;
		case STREAM_DATA_INT64:
		case STREAM_DATA_INT64_NULL:
			{
				int64_t temp = 0;
				separated >> temp;
				*(char*)(m_data + m_writeOffset) = (char)temp;
			}
			break;
		case STREAM_DATA_UINT64:
		case STREAM_DATA_UINT64_NULL:
			{
				uint64_t temp = 0;
				separated >> temp;
				*(char*)(m_data + m_writeOffset) = (char)temp;
			}
			break;
		case STREAM_DATA_FLOAT:
		case STREAM_DATA_FLOAT_NULL:
			{
				float temp = 0.0f;
				separated >> temp;
				*(char*)(m_data + m_writeOffset) = (char)temp;
			}
			break;
		case STREAM_DATA_DOUBLE:
		case STREAM_DATA_DOUBLE_NULL:
			{
				double temp = 0.0;
				separated >> temp;
				*(char*)(m_data + m_writeOffset) = (char)temp;
			}
			break;
		default:
			assert(false);
			break;
		};
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteBoolFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsSingleType(nType));

	int nSize = sizeof(bool);

	if(NULL != szInput) {

#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif

		AddBytesAndReallocate(nSize);

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		switch(nType) {
		case STREAM_DATA_CHAR:
		case STREAM_DATA_CHAR_NULL:
			{
				char temp = 0;
				separated >> temp;
				*(bool*)(m_data + m_writeOffset) = (0 != temp);
			}
			break;
		case STREAM_DATA_BOOL:
		case STREAM_DATA_BOOL_NULL:
			{
				separated >> *(bool*)(m_data + m_writeOffset);
			}
			break;
		case STREAM_DATA_INT8:
		case STREAM_DATA_INT8_NULL:
			{
				int8_t temp = 0;
				separated >> temp;
				*(bool*)(m_data + m_writeOffset) = (0 != temp);
			}
			break;
		case STREAM_DATA_UINT8:
		case STREAM_DATA_UINT8_NULL:
			{
				uint8_t temp = 0;
				separated >> temp;
				*(bool*)(m_data + m_writeOffset) = (0 != temp);
			}
			break;
		case STREAM_DATA_INT16:
		case STREAM_DATA_INT16_NULL:
			{
				int16_t temp = 0;
				separated >> temp;
				*(bool*)(m_data + m_writeOffset) = (0 != temp);
			}
			break;
		case STREAM_DATA_UINT16:
		case STREAM_DATA_UINT16_NULL:
			{
				uint16_t temp = 0;
				separated >> temp;
				*(bool*)(m_data + m_writeOffset) = (0 != temp);
			}
			break;
		case STREAM_DATA_INT32:
		case STREAM_DATA_INT32_NULL:
			{
				int32_t temp = 0;
				separated >> temp;
				*(bool*)(m_data + m_writeOffset) = (0 != temp);
			}
			break;
		case STREAM_DATA_UINT32:
		case STREAM_DATA_UINT32_NULL:
			{
				uint32_t temp = 0;
				separated >> temp;
				*(bool*)(m_data + m_writeOffset) = (0 != temp);
			}
			break;
		case STREAM_DATA_INT64:
		case STREAM_DATA_INT64_NULL:
			{
				int64_t temp = 0;
				separated >> temp;
				*(bool*)(m_data + m_writeOffset) = (0 != temp);
			}
			break;
		case STREAM_DATA_UINT64:
		case STREAM_DATA_UINT64_NULL:
			{
				uint64_t temp = 0;
				separated >> temp;
				*(bool*)(m_data + m_writeOffset) = (0 != temp);
			}
			break;
		case STREAM_DATA_FLOAT:
		case STREAM_DATA_FLOAT_NULL:
			{
				float temp = 0.0f;
				separated >> temp;
				*(bool*)(m_data + m_writeOffset) = (0.0f != temp);
			}
			break;
		case STREAM_DATA_DOUBLE:
		case STREAM_DATA_DOUBLE_NULL:
			{
				double temp = 0.0;
				separated >> temp;
				*(bool*)(m_data + m_writeOffset) = (0.0 != temp);
			}
			break;
		default:
			assert(false);
			break;
		};
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteUInt8FromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsSingleType(nType));

	int nSize = sizeof(uint8_t);

	if(NULL != szInput) {

#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif

		AddBytesAndReallocate(nSize);

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		switch(nType) {
		case STREAM_DATA_CHAR:
		case STREAM_DATA_CHAR_NULL:
			{
				char temp = 0;
				separated >> temp;
				*(uint8_t*)(m_data + m_writeOffset) = (uint8_t)temp;
			}
			break;
		case STREAM_DATA_BOOL:
		case STREAM_DATA_BOOL_NULL:
			{
				bool temp = false;
				separated >> temp;
				*(uint8_t*)(m_data + m_writeOffset) = (uint8_t)temp;
			}
			break;
		case STREAM_DATA_INT8:
		case STREAM_DATA_INT8_NULL:
			{
				int8_t temp = 0;
				separated >> temp;
				*(uint8_t*)(m_data + m_writeOffset) = (uint8_t)temp;
			}
			break;
		case STREAM_DATA_UINT8:
		case STREAM_DATA_UINT8_NULL:
			{
				separated >> *(uint8_t*)(m_data + m_writeOffset);
			}
			break;
		case STREAM_DATA_INT16:
		case STREAM_DATA_INT16_NULL:
			{
				int16_t temp = 0;
				separated >> temp;
				*(uint8_t*)(m_data + m_writeOffset) = (uint8_t)temp;
			}
			break;
		case STREAM_DATA_UINT16:
		case STREAM_DATA_UINT16_NULL:
			{
				uint16_t temp = 0;
				separated >> temp;
				*(uint8_t*)(m_data + m_writeOffset) = (uint8_t)temp;
			}
			break;
		case STREAM_DATA_INT32:
		case STREAM_DATA_INT32_NULL:
			{
				int32_t temp = 0;
				separated >> temp;
				*(uint8_t*)(m_data + m_writeOffset) = (uint8_t)temp;
			}
			break;
		case STREAM_DATA_UINT32:
		case STREAM_DATA_UINT32_NULL:
			{
				uint32_t temp = 0;
				separated >> temp;
				*(uint8_t*)(m_data + m_writeOffset) = (uint8_t)temp;
			}
			break;
		case STREAM_DATA_INT64:
		case STREAM_DATA_INT64_NULL:
			{
				int64_t temp = 0;
				separated >> temp;
				*(uint8_t*)(m_data + m_writeOffset) = (uint8_t)temp;
			}
			break;
		case STREAM_DATA_UINT64:
		case STREAM_DATA_UINT64_NULL:
			{
				uint64_t temp = 0;
				separated >> temp;
				*(uint8_t*)(m_data + m_writeOffset) = (uint8_t)temp;
			}
			break;
		case STREAM_DATA_FLOAT:
		case STREAM_DATA_FLOAT_NULL:
			{
				float temp = 0.0f;
				separated >> temp;
				*(uint8_t*)(m_data + m_writeOffset) = (uint8_t)temp;
			}
			break;
		case STREAM_DATA_DOUBLE:
		case STREAM_DATA_DOUBLE_NULL:
			{
				double temp = 0.0;
				separated >> temp;
				*(uint8_t*)(m_data + m_writeOffset) = (uint8_t)temp;
			}
			break;
		default:
			assert(false);
			break;
		};
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteUInt16FromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsSingleType(nType));

	int nSize = sizeof(uint16_t);

	if(NULL != szInput) {

#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif

		AddBytesAndReallocate(nSize);

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		switch(nType) {
		case STREAM_DATA_CHAR:
		case STREAM_DATA_CHAR_NULL:
			{
				char temp = 0;
				separated >> temp;
				*(uint16_t*)(m_data + m_writeOffset) = (uint16_t)temp;
			}
			break;
		case STREAM_DATA_BOOL:
		case STREAM_DATA_BOOL_NULL:
			{
				bool temp = false;
				separated >> temp;
				*(uint16_t*)(m_data + m_writeOffset) = (uint16_t)temp;
			}
			break;
		case STREAM_DATA_INT8:
		case STREAM_DATA_INT8_NULL:
			{
				int8_t temp = 0;
				separated >> temp;
				*(uint16_t*)(m_data + m_writeOffset) = (uint16_t)temp;
			}
			break;
		case STREAM_DATA_UINT8:
		case STREAM_DATA_UINT8_NULL:
			{
				uint8_t temp = 0;
				separated >> temp;
				*(uint16_t*)(m_data + m_writeOffset) = (uint16_t)temp;
			}
			break;
		case STREAM_DATA_INT16:
		case STREAM_DATA_INT16_NULL:
			{
				int16_t temp = 0;
				separated >> temp;
				*(uint16_t*)(m_data + m_writeOffset) = (uint16_t)temp;
			}
			break;
		case STREAM_DATA_UINT16:
		case STREAM_DATA_UINT16_NULL:
			{
				separated >> *(uint16_t*)(m_data + m_writeOffset);
			}
			break;
		case STREAM_DATA_INT32:
		case STREAM_DATA_INT32_NULL:
			{
				int32_t temp = 0;
				separated >> temp;
				*(uint16_t*)(m_data + m_writeOffset) = (uint16_t)temp;
			}
			break;
		case STREAM_DATA_UINT32:
		case STREAM_DATA_UINT32_NULL:
			{
				uint32_t temp = 0;
				separated >> temp;
				*(uint16_t*)(m_data + m_writeOffset) = (uint16_t)temp;
			}
			break;
		case STREAM_DATA_INT64:
		case STREAM_DATA_INT64_NULL:
			{
				int64_t temp = 0;
				separated >> temp;
				*(uint16_t*)(m_data + m_writeOffset) = (uint16_t)temp;
			}
			break;
		case STREAM_DATA_UINT64:
		case STREAM_DATA_UINT64_NULL:
			{
				uint64_t temp = 0;
				separated >> temp;
				*(uint16_t*)(m_data + m_writeOffset) = (uint16_t)temp;
			}
			break;
		case STREAM_DATA_FLOAT:
		case STREAM_DATA_FLOAT_NULL:
			{
				float temp = 0.0f;
				separated >> temp;
				*(uint16_t*)(m_data + m_writeOffset) = (uint16_t)temp;
			}
			break;
		case STREAM_DATA_DOUBLE:
		case STREAM_DATA_DOUBLE_NULL:
			{
				double temp = 0.0;
				separated >> temp;
				*(uint16_t*)(m_data + m_writeOffset) = (uint16_t)temp;
			}
			break;
		default:
			assert(false);
			break;
		};
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteUInt32FromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsSingleType(nType));

	int nSize = sizeof(uint32_t);

	if(NULL != szInput) {

#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif

		AddBytesAndReallocate(nSize);

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		switch(nType) {
		case STREAM_DATA_CHAR:
		case STREAM_DATA_CHAR_NULL:
			{
				char temp = 0;
				separated >> temp;
				*(uint32_t*)(m_data + m_writeOffset) = (uint32_t)temp;
			}
			break;
		case STREAM_DATA_BOOL:
		case STREAM_DATA_BOOL_NULL:
			{
				bool temp = false;
				separated >> temp;
				*(uint32_t*)(m_data + m_writeOffset) = (uint32_t)temp;
			}
			break;
		case STREAM_DATA_INT8:
		case STREAM_DATA_INT8_NULL:
			{
				int8_t temp = 0;
				separated >> temp;
				*(uint32_t*)(m_data + m_writeOffset) = (uint32_t)temp;
			}
			break;
		case STREAM_DATA_UINT8:
		case STREAM_DATA_UINT8_NULL:
			{
				uint8_t temp = 0;
				separated >> temp;
				*(uint32_t*)(m_data + m_writeOffset) = (uint32_t)temp;
			}
			break;
		case STREAM_DATA_INT16:
		case STREAM_DATA_INT16_NULL:
			{
				int16_t temp = 0;
				separated >> temp;
				*(uint32_t*)(m_data + m_writeOffset) = (uint32_t)temp;
			}
			break;
		case STREAM_DATA_UINT16:
		case STREAM_DATA_UINT16_NULL:
			{
				uint16_t temp = 0;
				separated >> temp;
				*(uint32_t*)(m_data + m_writeOffset) = (uint32_t)temp;
			}
			break;
		case STREAM_DATA_INT32:
		case STREAM_DATA_INT32_NULL:
			{
				int32_t temp = 0;
				separated >> temp;
				*(uint32_t*)(m_data + m_writeOffset) = (uint32_t)temp;
			}
			break;
		case STREAM_DATA_UINT32:
		case STREAM_DATA_UINT32_NULL:
			{
				separated >> *(uint32_t*)(m_data + m_writeOffset);
			}
			break;
		case STREAM_DATA_INT64:
		case STREAM_DATA_INT64_NULL:
			{
				int64_t temp = 0;
				separated >> temp;
				*(uint32_t*)(m_data + m_writeOffset) = (uint32_t)temp;
			}
			break;
		case STREAM_DATA_UINT64:
		case STREAM_DATA_UINT64_NULL:
			{
				uint64_t temp = 0;
				separated >> temp;
				*(uint32_t*)(m_data + m_writeOffset) = (uint32_t)temp;
			}
			break;
		case STREAM_DATA_FLOAT:
		case STREAM_DATA_FLOAT_NULL:
			{
				float temp = 0.0f;
				separated >> temp;
				*(uint32_t*)(m_data + m_writeOffset) = (uint32_t)temp;
			}
			break;
		case STREAM_DATA_DOUBLE:
		case STREAM_DATA_DOUBLE_NULL:
			{
				double temp = 0.0;
				separated >> temp;
				*(uint32_t*)(m_data + m_writeOffset) = (uint32_t)temp;
			}
			break;
		default:
			assert(false);
			break;
		};
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteUInt64FromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsSingleType(nType));

	int nSize = sizeof(uint64_t);

	if(NULL != szInput) {

#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif

		AddBytesAndReallocate(nSize);

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		switch(nType) {
		case STREAM_DATA_CHAR:
		case STREAM_DATA_CHAR_NULL:
			{
				char temp = 0;
				separated >> temp;
				*(uint64_t*)(m_data + m_writeOffset) = (uint64_t)temp;
			}
			break;
		case STREAM_DATA_BOOL:
		case STREAM_DATA_BOOL_NULL:
			{
				bool temp = false;
				separated >> temp;
				*(uint64_t*)(m_data + m_writeOffset) = (uint64_t)temp;
			}
			break;
		case STREAM_DATA_INT8:
		case STREAM_DATA_INT8_NULL:
			{
				int8_t temp = 0;
				separated >> temp;
				*(uint64_t*)(m_data + m_writeOffset) = (uint64_t)temp;
			}
			break;
		case STREAM_DATA_UINT8:
		case STREAM_DATA_UINT8_NULL:
			{
				uint8_t temp = 0;
				separated >> temp;
				*(uint64_t*)(m_data + m_writeOffset) = (uint64_t)temp;
			}
			break;
		case STREAM_DATA_INT16:
		case STREAM_DATA_INT16_NULL:
			{
				int16_t temp = 0;
				separated >> temp;
				*(uint64_t*)(m_data + m_writeOffset) = (uint64_t)temp;
			}
			break;
		case STREAM_DATA_UINT16:
		case STREAM_DATA_UINT16_NULL:
			{
				uint16_t temp = 0;
				separated >> temp;
				*(uint64_t*)(m_data + m_writeOffset) = (uint64_t)temp;
			}
			break;
		case STREAM_DATA_INT32:
		case STREAM_DATA_INT32_NULL:
			{
				int32_t temp = 0;
				separated >> temp;
				*(uint64_t*)(m_data + m_writeOffset) = (uint64_t)temp;
			}
			break;
		case STREAM_DATA_UINT32:
		case STREAM_DATA_UINT32_NULL:
			{
				uint32_t temp = 0;
				separated >> temp;
				*(uint64_t*)(m_data + m_writeOffset) = (uint64_t)temp;
			}
			break;
		case STREAM_DATA_INT64:
		case STREAM_DATA_INT64_NULL:
			{
				int64_t temp = 0;
				separated >> temp;
				*(uint64_t*)(m_data + m_writeOffset) = (uint64_t)temp;
			}
			break;
		case STREAM_DATA_UINT64:
		case STREAM_DATA_UINT64_NULL:
			{
				separated >> *(uint64_t*)(m_data + m_writeOffset);
			}
			break;
		case STREAM_DATA_FLOAT:
		case STREAM_DATA_FLOAT_NULL:
			{
				float temp = 0.0f;
				separated >> temp;
				*(uint64_t*)(m_data + m_writeOffset) = (uint64_t)temp;
			}
			break;
		case STREAM_DATA_DOUBLE:
		case STREAM_DATA_DOUBLE_NULL:
			{
				double temp = 0.0;
				separated >> temp;
				*(uint64_t*)(m_data + m_writeOffset) = (uint64_t)temp;
			}
			break;
		default:
			assert(false);
			break;
		};
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteInt8FromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsSingleType(nType));

	int nSize = sizeof(int8_t);

	if(NULL != szInput) {

#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif

		AddBytesAndReallocate(nSize);

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		switch(nType) {
		case STREAM_DATA_CHAR:
		case STREAM_DATA_CHAR_NULL:
			{
				char temp = 0;
				separated >> temp;
				*(int8_t*)(m_data + m_writeOffset) = (int8_t)temp;
			}
			break;
		case STREAM_DATA_BOOL:
		case STREAM_DATA_BOOL_NULL:
			{
				bool temp = false;
				separated >> temp;
				*(int8_t*)(m_data + m_writeOffset) = (int8_t)temp;
			}
			break;
		case STREAM_DATA_INT8:
		case STREAM_DATA_INT8_NULL:
			{
				separated >> *(int8_t*)(m_data + m_writeOffset);
			}
			break;
		case STREAM_DATA_UINT8:
		case STREAM_DATA_UINT8_NULL:
			{
				uint8_t temp = 0;
				separated >> temp;
				*(int8_t*)(m_data + m_writeOffset) = (int8_t)temp;
			}
			break;
		case STREAM_DATA_INT16:
		case STREAM_DATA_INT16_NULL:
			{
				int16_t temp = 0;
				separated >> temp;
				*(int8_t*)(m_data + m_writeOffset) = (int8_t)temp;
			}
			break;
		case STREAM_DATA_UINT16:
		case STREAM_DATA_UINT16_NULL:
			{
				uint16_t temp = 0;
				separated >> temp;
				*(int8_t*)(m_data + m_writeOffset) = (int8_t)temp;
			}
			break;
		case STREAM_DATA_INT32:
		case STREAM_DATA_INT32_NULL:
			{
				int32_t temp = 0;
				separated >> temp;
				*(int8_t*)(m_data + m_writeOffset) = (int8_t)temp;
			}
			break;
		case STREAM_DATA_UINT32:
		case STREAM_DATA_UINT32_NULL:
			{
				uint32_t temp = 0;
				separated >> temp;
				*(int8_t*)(m_data + m_writeOffset) = (int8_t)temp;
			}
			break;
		case STREAM_DATA_INT64:
		case STREAM_DATA_INT64_NULL:
			{
				int64_t temp = 0;
				separated >> temp;
				*(int8_t*)(m_data + m_writeOffset) = (int8_t)temp;
			}
			break;
		case STREAM_DATA_UINT64:
		case STREAM_DATA_UINT64_NULL:
			{
				uint64_t temp = 0;
				separated >> temp;
				*(int8_t*)(m_data + m_writeOffset) = (int8_t)temp;
			}
			break;
		case STREAM_DATA_FLOAT:
		case STREAM_DATA_FLOAT_NULL:
			{
				float temp = 0.0f;
				separated >> temp;
				*(int8_t*)(m_data + m_writeOffset) = (int8_t)temp;
			}
			break;
		case STREAM_DATA_DOUBLE:
		case STREAM_DATA_DOUBLE_NULL:
			{
				double temp = 0.0;
				separated >> temp;
				*(int8_t*)(m_data + m_writeOffset) = (int8_t)temp;
			}
			break;
		default:
			assert(false);
			break;
		};
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteInt16FromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsSingleType(nType));

	int nSize = sizeof(int16_t);

	if(NULL != szInput) {

#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif

		AddBytesAndReallocate(nSize);

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		switch(nType) {
		case STREAM_DATA_CHAR:
		case STREAM_DATA_CHAR_NULL:
			{
				char temp = 0;
				separated >> temp;
				*(int16_t*)(m_data + m_writeOffset) = (int16_t)temp;
			}
			break;
		case STREAM_DATA_BOOL:
		case STREAM_DATA_BOOL_NULL:
			{
				bool temp = false;
				separated >> temp;
				*(int16_t*)(m_data + m_writeOffset) = (int16_t)temp;
			}
			break;
		case STREAM_DATA_INT8:
		case STREAM_DATA_INT8_NULL:
			{
				int8_t temp = 0;
				separated >> temp;
				*(int16_t*)(m_data + m_writeOffset) = (int16_t)temp;
			}
			break;
		case STREAM_DATA_UINT8:
		case STREAM_DATA_UINT8_NULL:
			{
				uint8_t temp = 0;
				separated >> temp;
				*(int16_t*)(m_data + m_writeOffset) = (int16_t)temp;
			}
			break;
		case STREAM_DATA_INT16:
		case STREAM_DATA_INT16_NULL:
			{
				separated >> *(int16_t*)(m_data + m_writeOffset);
			}
			break;
		case STREAM_DATA_UINT16:
		case STREAM_DATA_UINT16_NULL:
			{
				uint16_t temp = 0;
				separated >> temp;
				*(int16_t*)(m_data + m_writeOffset) = (int16_t)temp;
			}
			break;
		case STREAM_DATA_INT32:
		case STREAM_DATA_INT32_NULL:
			{
				int32_t temp = 0;
				separated >> temp;
				*(int16_t*)(m_data + m_writeOffset) = (int16_t)temp;
			}
			break;
		case STREAM_DATA_UINT32:
		case STREAM_DATA_UINT32_NULL:
			{
				uint32_t temp = 0;
				separated >> temp;
				*(int16_t*)(m_data + m_writeOffset) = (int16_t)temp;
			}
			break;
		case STREAM_DATA_INT64:
		case STREAM_DATA_INT64_NULL:
			{
				int64_t temp = 0;
				separated >> temp;
				*(int16_t*)(m_data + m_writeOffset) = (int16_t)temp;
			}
			break;
		case STREAM_DATA_UINT64:
		case STREAM_DATA_UINT64_NULL:
			{
				uint64_t temp = 0;
				separated >> temp;
				*(int16_t*)(m_data + m_writeOffset) = (int16_t)temp;
			}
			break;
		case STREAM_DATA_FLOAT:
		case STREAM_DATA_FLOAT_NULL:
			{
				float temp = 0.0f;
				separated >> temp;
				*(int16_t*)(m_data + m_writeOffset) = (int16_t)temp;
			}
			break;
		case STREAM_DATA_DOUBLE:
		case STREAM_DATA_DOUBLE_NULL:
			{
				double temp = 0.0;
				separated >> temp;
				*(int16_t*)(m_data + m_writeOffset) = (int16_t)temp;
			}
			break;
		default:
			assert(false);
			break;
		};
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteInt32FromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsSingleType(nType));

	int nSize = sizeof(int32_t);

	if(NULL != szInput) {

#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif

		AddBytesAndReallocate(nSize);

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		switch(nType) {
		case STREAM_DATA_CHAR:
		case STREAM_DATA_CHAR_NULL:
			{
				char temp = 0;
				separated >> temp;
				*(int32_t*)(m_data + m_writeOffset) = (int32_t)temp;
			}
			break;
		case STREAM_DATA_BOOL:
		case STREAM_DATA_BOOL_NULL:
			{
				bool temp = false;
				separated >> temp;
				*(int32_t*)(m_data + m_writeOffset) = (int32_t)temp;
			}
			break;
		case STREAM_DATA_INT8:
		case STREAM_DATA_INT8_NULL:
			{
				int8_t temp = 0;
				separated >> temp;
				*(int32_t*)(m_data + m_writeOffset) = (int32_t)temp;
			}
			break;
		case STREAM_DATA_UINT8:
		case STREAM_DATA_UINT8_NULL:
			{
				uint8_t temp = 0;
				separated >> temp;
				*(int32_t*)(m_data + m_writeOffset) = (int32_t)temp;
			}
			break;
		case STREAM_DATA_INT16:
		case STREAM_DATA_INT16_NULL:
			{
				int16_t temp = 0;
				separated >> temp;
				*(int32_t*)(m_data + m_writeOffset) = (int32_t)temp;
			}
			break;
		case STREAM_DATA_UINT16:
		case STREAM_DATA_UINT16_NULL:
			{
				uint16_t temp = 0;
				separated >> temp;
				*(int32_t*)(m_data + m_writeOffset) = (int32_t)temp;
			}
			break;
		case STREAM_DATA_INT32:
		case STREAM_DATA_INT32_NULL:
			{
				separated >> *(int32_t*)(m_data + m_writeOffset);
			}
			break;
		case STREAM_DATA_UINT32:
		case STREAM_DATA_UINT32_NULL:
			{
				uint32_t temp = 0;
				separated >> temp;
				*(int32_t*)(m_data + m_writeOffset) = (int32_t)temp;
			}
			break;
		case STREAM_DATA_INT64:
		case STREAM_DATA_INT64_NULL:
			{
				int64_t temp = 0;
				separated >> temp;
				*(int32_t*)(m_data + m_writeOffset) = (int32_t)temp;
			}
			break;
		case STREAM_DATA_UINT64:
		case STREAM_DATA_UINT64_NULL:
			{
				uint64_t temp = 0;
				separated >> temp;
				*(int32_t*)(m_data + m_writeOffset) = (int32_t)temp;
			}
			break;
		case STREAM_DATA_FLOAT:
		case STREAM_DATA_FLOAT_NULL:
			{
				float temp = 0.0f;
				separated >> temp;
				*(int32_t*)(m_data + m_writeOffset) = (int32_t)temp;
			}
			break;
		case STREAM_DATA_DOUBLE:
		case STREAM_DATA_DOUBLE_NULL:
			{
				double temp = 0.0;
				separated >> temp;
				*(int32_t*)(m_data + m_writeOffset) = (int32_t)temp;
			}
			break;
		default:
			assert(false);
			break;
		};
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteInt64FromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsSingleType(nType));

	int nSize = sizeof(int64_t);

	if(NULL != szInput) {

#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif

		AddBytesAndReallocate(nSize);

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		switch(nType) {
		case STREAM_DATA_CHAR:
		case STREAM_DATA_CHAR_NULL:
			{
				char temp = 0;
				separated >> temp;
				*(int64_t*)(m_data + m_writeOffset) = (int64_t)temp;
			}
			break;
		case STREAM_DATA_BOOL:
		case STREAM_DATA_BOOL_NULL:
			{
				bool temp = false;
				separated >> temp;
				*(int64_t*)(m_data + m_writeOffset) = (int64_t)temp;
			}
			break;
		case STREAM_DATA_INT8:
		case STREAM_DATA_INT8_NULL:
			{
				int8_t temp = 0;
				separated >> temp;
				*(int64_t*)(m_data + m_writeOffset) = (int64_t)temp;
			}
			break;
		case STREAM_DATA_UINT8:
		case STREAM_DATA_UINT8_NULL:
			{
				uint8_t temp = 0;
				separated >> temp;
				*(int64_t*)(m_data + m_writeOffset) = (int64_t)temp;
			}
			break;
		case STREAM_DATA_INT16:
		case STREAM_DATA_INT16_NULL:
			{
				int16_t temp = 0;
				separated >> temp;
				*(int64_t*)(m_data + m_writeOffset) = (int64_t)temp;
			}
			break;
		case STREAM_DATA_UINT16:
		case STREAM_DATA_UINT16_NULL:
			{
				uint16_t temp = 0;
				separated >> temp;
				*(int64_t*)(m_data + m_writeOffset) = (int64_t)temp;
			}
			break;
		case STREAM_DATA_INT32:
		case STREAM_DATA_INT32_NULL:
			{
				int32_t temp = 0;
				separated >> temp;
				*(int64_t*)(m_data + m_writeOffset) = (int64_t)temp;
			}
			break;
		case STREAM_DATA_UINT32:
		case STREAM_DATA_UINT32_NULL:
			{
				uint32_t temp = 0;
				separated >> temp;
				*(int64_t*)(m_data + m_writeOffset) = (int64_t)temp;
			}
			break;
		case STREAM_DATA_INT64:
		case STREAM_DATA_INT64_NULL:
			{
				separated >> *(int64_t*)(m_data + m_writeOffset);
			}
			break;
		case STREAM_DATA_UINT64:
		case STREAM_DATA_UINT64_NULL:
			{
				uint64_t temp = 0;
				separated >> temp;
				*(int64_t*)(m_data + m_writeOffset) = (int64_t)temp;
			}
			break;
		case STREAM_DATA_FLOAT:
		case STREAM_DATA_FLOAT_NULL:
			{
				float temp = 0.0f;
				separated >> temp;
				*(int64_t*)(m_data + m_writeOffset) = (int64_t)temp;
			}
			break;
		case STREAM_DATA_DOUBLE:
		case STREAM_DATA_DOUBLE_NULL:
			{
				double temp = 0.0;
				separated >> temp;
				*(int64_t*)(m_data + m_writeOffset) = (int64_t)temp;
			}
			break;
		default:
			assert(false);
			break;
		};
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteFloatFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsSingleType(nType));

	int nSize = sizeof(float);

	if(NULL != szInput) {

#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif

		AddBytesAndReallocate(nSize);

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		switch(nType) {
		case STREAM_DATA_CHAR:
		case STREAM_DATA_CHAR_NULL:
			{
				char temp = 0;
				separated >> temp;
				*(float*)(m_data + m_writeOffset) = (float)temp;
			}
			break;
		case STREAM_DATA_BOOL:
		case STREAM_DATA_BOOL_NULL:
			{
				bool temp = false;
				separated >> temp;
				*(float*)(m_data + m_writeOffset) = (float)temp;
			}
			break;
		case STREAM_DATA_INT8:
		case STREAM_DATA_INT8_NULL:
			{
				int8_t temp = 0;
				separated >> temp;
				*(float*)(m_data + m_writeOffset) = (float)temp;
			}
			break;
		case STREAM_DATA_UINT8:
		case STREAM_DATA_UINT8_NULL:
			{
				uint8_t temp = 0;
				separated >> temp;
				*(float*)(m_data + m_writeOffset) = (float)temp;
			}
			break;
		case STREAM_DATA_INT16:
		case STREAM_DATA_INT16_NULL:
			{
				int16_t temp = 0;
				separated >> temp;
				*(float*)(m_data + m_writeOffset) = (float)temp;
			}
			break;
		case STREAM_DATA_UINT16:
		case STREAM_DATA_UINT16_NULL:
			{
				uint16_t temp = 0;
				separated >> temp;
				*(float*)(m_data + m_writeOffset) = (float)temp;
			}
			break;
		case STREAM_DATA_INT32:
		case STREAM_DATA_INT32_NULL:
			{
				int32_t temp = 0;
				separated >> temp;
				*(float*)(m_data + m_writeOffset) = (float)temp;
			}
			break;
		case STREAM_DATA_UINT32:
		case STREAM_DATA_UINT32_NULL:
			{
				uint32_t temp = 0;
				separated >> temp;
				*(float*)(m_data + m_writeOffset) = (float)temp;
			}
			break;
		case STREAM_DATA_INT64:
		case STREAM_DATA_INT64_NULL:
			{
				int64_t temp = 0;
				separated >> temp;
				*(float*)(m_data + m_writeOffset) = (float)temp;
			}
			break;
		case STREAM_DATA_UINT64:
		case STREAM_DATA_UINT64_NULL:
			{
				uint64_t temp = 0;
				separated >> temp;
				*(float*)(m_data + m_writeOffset) = (float)temp;
			}
			break;
		case STREAM_DATA_FLOAT:
		case STREAM_DATA_FLOAT_NULL:
			{
				separated >> *(float*)(m_data + m_writeOffset);
			}
			break;
		case STREAM_DATA_DOUBLE:
		case STREAM_DATA_DOUBLE_NULL:
			{
				double temp = 0.0;
				separated >> temp;
				*(float*)(m_data + m_writeOffset) = (float)temp;
			}
			break;
		default:
			assert(false);
			break;
		};
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteDoubleFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsSingleType(nType));

	int nSize = sizeof(double);

	if(NULL != szInput) {

#ifdef SS_DANGER_AREA_CHECKING
		CheckDangerArea(m_writeOffset);
#endif

		AddBytesAndReallocate(nSize);

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		switch(nType) {
		case STREAM_DATA_CHAR:
		case STREAM_DATA_CHAR_NULL:
			{
				char temp = 0;
				separated >> temp;
				*(double*)(m_data + m_writeOffset) = (double)temp;
			}
			break;
		case STREAM_DATA_BOOL:
		case STREAM_DATA_BOOL_NULL:
			{
				bool temp = false;
				separated >> temp;
				*(double*)(m_data + m_writeOffset) = (double)temp;
			}
			break;
		case STREAM_DATA_INT8:
		case STREAM_DATA_INT8_NULL:
			{
				int8_t temp = 0;
				separated >> temp;
				*(double*)(m_data + m_writeOffset) = (double)temp;
			}
			break;
		case STREAM_DATA_UINT8:
		case STREAM_DATA_UINT8_NULL:
			{
				uint8_t temp = 0;
				separated >> temp;
				*(double*)(m_data + m_writeOffset) = (double)temp;
			}
			break;
		case STREAM_DATA_INT16:
		case STREAM_DATA_INT16_NULL:
			{
				int16_t temp = 0;
				separated >> temp;
				*(double*)(m_data + m_writeOffset) = (double)temp;
			}
			break;
		case STREAM_DATA_UINT16:
		case STREAM_DATA_UINT16_NULL:
			{
				uint16_t temp = 0;
				separated >> temp;
				*(double*)(m_data + m_writeOffset) = (double)temp;
			}
			break;
		case STREAM_DATA_INT32:
		case STREAM_DATA_INT32_NULL:
			{
				int32_t temp = 0;
				separated >> temp;
				*(double*)(m_data + m_writeOffset) = (double)temp;
			}
			break;
		case STREAM_DATA_UINT32:
		case STREAM_DATA_UINT32_NULL:
			{
				uint32_t temp = 0;
				separated >> temp;
				*(double*)(m_data + m_writeOffset) = (double)temp;
			}
			break;
		case STREAM_DATA_INT64:
		case STREAM_DATA_INT64_NULL:
			{
				int64_t temp = 0;
				separated >> temp;
				*(double*)(m_data + m_writeOffset) = (double)temp;
			}
			break;
		case STREAM_DATA_UINT64:
		case STREAM_DATA_UINT64_NULL:
			{
				uint64_t temp = 0;
				separated >> temp;
				*(double*)(m_data + m_writeOffset) = (double)temp;
			}
			break;
		case STREAM_DATA_FLOAT:
		case STREAM_DATA_FLOAT_NULL:
			{
				float temp = 0.0f;
				separated >> temp;
				*(double*)(m_data + m_writeOffset) = (double)temp;
			}
			break;
		case STREAM_DATA_DOUBLE:
		case STREAM_DATA_DOUBLE_NULL:
			{
				separated >> *(double*)(m_data + m_writeOffset);
			}
			break;
		default:
			assert(false);
			break;
		};
	}
	m_writeOffset += nSize;
}

void util::CStoreStream::WriteStringFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsStringType(nType));

	if(NULL != szInput) {

		uint16_t len = (uint16_t)length;

		CStoreStream object(len + sizeof(uint16_t));
		object << len;

		if(len > 0) {
			memcpy((char*)(object.GetData() + object.GetWriteOffset()), szInput, len);
			object.m_writeOffset += len;
		}

		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteBoolSetFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsContainerType(nType));

	if(NULL != szInput) {

		if(length < 0) {
			length = 0;
		}

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		uint8_t nInnerType = GetInnerType(nType);

		int32_t nCount = 0;

		CStoreStream object(length + sizeof(int32_t));
		object << nCount;

		const char* pData = NULL;
		int nBytesLength = 0;
		do {
			separated.GetLine(pData, nBytesLength);
			object.WriteBoolFromString(pData, nBytesLength, nInnerType);
			++nCount;
		}while(separated.MoreData());

		int offset = object.GetWriteOffset();
		object.SetWriteOffset(0);
		object << nCount;
		object.SetWriteOffset(offset);

		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteUInt8SetFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsContainerType(nType));

	if(NULL != szInput) {

		if(length < 0) {
			length = 0;
		}

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		uint8_t nInnerType = GetInnerType(nType);

		int32_t nCount = 0;

		CStoreStream object(length + sizeof(int32_t));
		object << nCount;

		const char* pData = NULL;
		int nBytesLength = 0;
		do {
			separated.GetLine(pData, nBytesLength);
			object.WriteUInt8FromString(pData, nBytesLength, nInnerType);
			++nCount;
		}while(separated.MoreData());

		int offset = object.GetWriteOffset();
		object.SetWriteOffset(0);
		object << nCount;
		object.SetWriteOffset(offset);

		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteInt8SetFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsContainerType(nType));

	if(NULL != szInput) {

		if(length < 0) {
			length = 0;
		}

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		uint8_t nInnerType = GetInnerType(nType);

		int32_t nCount = 0;

		CStoreStream object(length + sizeof(int32_t));
		object << nCount;

		const char* pData = NULL;
		int nBytesLength = 0;
		do {
			separated.GetLine(pData, nBytesLength);
			object.WriteInt8FromString(pData, nBytesLength, nInnerType);
			++nCount;
		}while(separated.MoreData());

		int offset = object.GetWriteOffset();
		object.SetWriteOffset(0);
		object << nCount;
		object.SetWriteOffset(offset);

		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteUInt16SetFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsContainerType(nType));

	if(NULL != szInput) {

		if(length < 0) {
			length = 0;
		}

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		uint8_t nInnerType = GetInnerType(nType);

		int32_t nCount = 0;

		CStoreStream object(length + sizeof(int32_t));
		object << nCount;

		const char* pData = NULL;
		int nBytesLength = 0;
		do {
			separated.GetLine(pData, nBytesLength);
			object.WriteUInt16FromString(pData, nBytesLength, nInnerType);
			++nCount;
		}while(separated.MoreData());

		int offset = object.GetWriteOffset();
		object.SetWriteOffset(0);
		object << nCount;
		object.SetWriteOffset(offset);

		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteInt16SetFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsContainerType(nType));

	if(NULL != szInput) {

		if(length < 0) {
			length = 0;
		}

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		uint8_t nInnerType = GetInnerType(nType);

		int32_t nCount = 0;

		CStoreStream object(length + sizeof(int32_t));
		object << nCount;

		const char* pData = NULL;
		int nBytesLength = 0;
		do {
			separated.GetLine(pData, nBytesLength);
			object.WriteInt16FromString(pData, nBytesLength, nInnerType);
			++nCount;
		}while(separated.MoreData());

		int offset = object.GetWriteOffset();
		object.SetWriteOffset(0);
		object << nCount;
		object.SetWriteOffset(offset);

		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteUInt32SetFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsContainerType(nType));

	if(NULL != szInput) {

		if(length < 0) {
			length = 0;
		}

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		uint8_t nInnerType = GetInnerType(nType);

		int32_t nCount = 0;

		CStoreStream object(length + sizeof(int32_t));
		object << nCount;

		const char* pData = NULL;
		int nBytesLength = 0;
		do {
			separated.GetLine(pData, nBytesLength);
			object.WriteUInt32FromString(pData, nBytesLength, nInnerType);
			++nCount;
		}while(separated.MoreData());

		int offset = object.GetWriteOffset();
		object.SetWriteOffset(0);
		object << nCount;
		object.SetWriteOffset(offset);

		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteInt32SetFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsContainerType(nType));

	if(NULL != szInput) {

		if(length < 0) {
			length = 0;
		}

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		uint8_t nInnerType = GetInnerType(nType);

		int32_t nCount = 0;

		CStoreStream object(length + sizeof(int32_t));
		object << nCount;

		const char* pData = NULL;
		int nBytesLength = 0;
		do {
			separated.GetLine(pData, nBytesLength);
			object.WriteInt32FromString(pData, nBytesLength, nInnerType);
			++nCount;
		}while(separated.MoreData());

		int offset = object.GetWriteOffset();
		object.SetWriteOffset(0);
		object << nCount;
		object.SetWriteOffset(offset);

		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteUInt64SetFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsContainerType(nType));

	if(NULL != szInput) {

		if(length < 0) {
			length = 0;
		}

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		uint8_t nInnerType = GetInnerType(nType);

		int32_t nCount = 0;

		CStoreStream object(length + sizeof(int32_t));
		object << nCount;

		const char* pData = NULL;
		int nBytesLength = 0;
		do {
			separated.GetLine(pData, nBytesLength);
			object.WriteUInt64FromString(pData, nBytesLength, nInnerType);
			++nCount;
		}while(separated.MoreData());

		int offset = object.GetWriteOffset();
		object.SetWriteOffset(0);
		object << nCount;
		object.SetWriteOffset(offset);

		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteInt64SetFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsContainerType(nType));

	if(NULL != szInput) {

		if(length < 0) {
			length = 0;
		}

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		uint8_t nInnerType = GetInnerType(nType);

		int32_t nCount = 0;

		CStoreStream object(length + sizeof(int32_t));
		object << nCount;

		const char* pData = NULL;
		int nBytesLength = 0;
		do {
			separated.GetLine(pData, nBytesLength);
			object.WriteInt64FromString(pData, nBytesLength, nInnerType);
			++nCount;
		}while(separated.MoreData());

		int offset = object.GetWriteOffset();
		object.SetWriteOffset(0);
		object << nCount;
		object.SetWriteOffset(offset);

		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteFloatSetFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsContainerType(nType));

	if(NULL != szInput) {

		if(length < 0) {
			length = 0;
		}

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		uint8_t nInnerType = GetInnerType(nType);

		int32_t nCount = 0;

		CStoreStream object(length + sizeof(int32_t));
		object << nCount;

		const char* pData = NULL;
		int nBytesLength = 0;
		do {
			separated.GetLine(pData, nBytesLength);
			object.WriteFloatFromString(pData, nBytesLength, nInnerType);
			++nCount;
		}while(separated.MoreData());

		int offset = object.GetWriteOffset();
		object.SetWriteOffset(0);
		object << nCount;
		object.SetWriteOffset(offset);

		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteDoubleSetFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsContainerType(nType));

	if(NULL != szInput) {

		if(length < 0) {
			length = 0;
		}

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		uint8_t nInnerType = GetInnerType(nType);

		int32_t nCount = 0;

		CStoreStream object(length + sizeof(int32_t));
		object << nCount;

		const char* pData = NULL;
		int nBytesLength = 0;
		do {
			separated.GetLine(pData, nBytesLength);
			object.WriteDoubleFromString(pData, nBytesLength, nInnerType);
			++nCount;
		}while(separated.MoreData());

		int offset = object.GetWriteOffset();
		object.SetWriteOffset(0);
		object << nCount;
		object.SetWriteOffset(offset);

		WriteObject(object);
	} else {
		WriteObject();
	}
}

void util::CStoreStream::WriteStringSetFromString(const char* szInput, int length, uint8_t nType)
{
	assert(IsContainerType(nType));
	uint8_t nInnerType = GetInnerType(nType);
	assert(IsStringType(nInnerType));

	if(NULL != szInput) {

		if(length < 0) {
			length = 0;
		}

		CSeparatedStream separated(szInput, length, false,
			STORESTREAM_STRING_DELIM, STORESTREAM_STRING_DELIM);

		int32_t nCount = 0;

		CStoreStream object(length + length + sizeof(int32_t));
		object << nCount;

		const char* pData = NULL;
		int nBytesLength = 0;
		do {
			separated.GetLine(pData, nBytesLength);
			object << (uint16_t)nBytesLength;
			object.AddBytesAndReallocate(nBytesLength);
			memcpy((char*)(object.m_data + object.m_writeOffset), pData, nBytesLength);
			object.m_writeOffset += nBytesLength;
			++nCount;
		} while(separated.MoreData());

		int offset = object.GetWriteOffset();
		object.SetWriteOffset(0);
		object << nCount;
		object.SetWriteOffset(offset);

		WriteObject(object);
	} else {
		WriteObject();
	}
}

//////////////////////////////////////////////////////////////////////////
void util::CStoreStream::ReadCharToString(int& readOffset, std::string& output) const
{
	int nSize = sizeof(char);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}

	output = *(char*)(m_data + readOffset);

	readOffset += nSize;
}

void util::CStoreStream::ReadBoolToString(int& readOffset, std::string& output) const
{
	int nSize = sizeof(bool);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << *(bool*)(m_data + readOffset);
	separated.EndLine();
	output = separated.Str();

	readOffset += nSize;
}

void util::CStoreStream::ReadUInt8ToString(int& readOffset, std::string& output) const
{
	int nSize = sizeof(uint8_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << *(uint8_t*)(m_data + readOffset);
	separated.EndLine();
	output = separated.Str();

	readOffset += nSize;
}

void util::CStoreStream::ReadUInt16ToString(int& readOffset, std::string& output) const
{
	int nSize = sizeof(uint16_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << *(uint16_t*)(m_data + readOffset);
	separated.EndLine();
	output = separated.Str();

	readOffset += nSize;
}

void util::CStoreStream::ReadUInt32ToString(int& readOffset, std::string& output) const
{
	int nSize = sizeof(uint32_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << *(uint32_t*)(m_data + readOffset);
	separated.EndLine();
	output = separated.Str();

	readOffset += nSize;
}

void util::CStoreStream::ReadUInt64ToString(int& readOffset, std::string& output) const
{
	int nSize = sizeof(uint64_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << *(uint64_t*)(m_data + readOffset);
	separated.EndLine();
	output = separated.Str();

	readOffset += nSize;
}

void util::CStoreStream::ReadInt8ToString(int& readOffset, std::string& output) const
{
	int nSize = sizeof(int8_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << *(int8_t*)(m_data + readOffset);
	separated.EndLine();
	output = separated.Str();

	readOffset += nSize;
}

void util::CStoreStream::ReadInt16ToString(int& readOffset, std::string& output) const
{
	int nSize = sizeof(int16_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << *(int16_t*)(m_data + readOffset);
	separated.EndLine();
	output = separated.Str();

	readOffset += nSize;
}

void util::CStoreStream::ReadInt32ToString(int& readOffset, std::string& output) const
{
	int nSize = sizeof(int32_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << *(int32_t*)(m_data + readOffset);
	separated.EndLine();
	output = separated.Str();

	readOffset += nSize;
}

void util::CStoreStream::ReadInt64ToString(int& readOffset, std::string& output) const
{
	int nSize = sizeof(int64_t);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << *(int64_t*)(m_data + readOffset);
	separated.EndLine();
	output = separated.Str();

	readOffset += nSize;
}

void util::CStoreStream::ReadFloatToString(int& readOffset, std::string& output) const
{
	int nSize = sizeof(float);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << *(float*)(m_data + readOffset);
	separated.EndLine();
	output = separated.Str();

	readOffset += nSize;
}

void util::CStoreStream::ReadDoubleToString(int& readOffset, std::string& output) const
{
	int nSize = sizeof(double);
	if(nSize + readOffset > m_writeOffset) {
		assert(false);
		return;
	}

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << *(double*)(m_data + readOffset);
	separated.EndLine();
	output = separated.Str();

	readOffset += nSize;
}

void util::CStoreStream::ReadStringToString(int& readOffset, std::string& output) const
{
	ReadObject(readOffset, output);
}

void util::CStoreStream::ReadBoolSetToString(int& readOffset, std::string& output) const
{
	std::vector<bool> object;
	ReadObject(readOffset, object);

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CStoreStream::ReadUInt8SetToString(int& readOffset, std::string& output) const
{
	std::vector<uint8_t> object;
	ReadObject(readOffset, object);

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CStoreStream::ReadInt8SetToString(int& readOffset, std::string& output) const
{
	std::vector<int8_t> object;
	ReadObject(readOffset, object);

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CStoreStream::ReadUInt16SetToString(int& readOffset, std::string& output) const
{
	std::vector<uint16_t> object;
	ReadObject(readOffset, object);

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CStoreStream::ReadInt16SetToString(int& readOffset, std::string& output) const
{
	std::vector<int16_t> object;
	ReadObject(readOffset, object);

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CStoreStream::ReadUInt32SetToString(int& readOffset, std::string& output) const
{
	std::vector<uint32_t> object;
	ReadObject(readOffset, object);

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CStoreStream::ReadInt32SetToString(int& readOffset, std::string& output) const
{
	std::vector<int32_t> object;
	ReadObject(readOffset, object);

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CStoreStream::ReadUInt64SetToString(int& readOffset, std::string& output) const
{
	std::vector<uint64_t> object;
	ReadObject(readOffset, object);

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CStoreStream::ReadInt64SetToString(int& readOffset, std::string& output) const
{
	std::vector<int64_t> object;
	ReadObject(readOffset, object);

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CStoreStream::ReadFloatSetToString(int& readOffset, std::string& output) const
{
	std::vector<float> object;
	ReadObject(readOffset, object);

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CStoreStream::ReadDoubleSetToString(int& readOffset, std::string& output) const
{
	std::vector<double> object;
	ReadObject(readOffset, object);

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CStoreStream::ReadStringSetToString(int& readOffset, std::string& output) const
{
	std::vector<std::string> object;
	ReadObject(readOffset, object);

	CSeparatedStream separated(
		STORESTREAM_STRING_DELIM,
		STORESTREAM_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

// 		struct TypeOperatorSet {
//			void (util::CStoreStream::*m_pWriteByType)(CTransferStream& stream, uint8_t nType);
//			void (util::CStoreStream::*m_pReadByType)(int& readOffset, CTransferStream& stream, bool bChange) const;
//			void (util::CStoreStream::*m_pWriteFromString)(const char* szInput, int length, uint8_t nType);
//			void (util::CStoreStream::*m_pReadToString)(int& readOffset, std::string& output) const;
//	};
struct util::CStoreStream::TypeOperatorSet util::CStoreStream::s_typeOperators[STREAM_DATA_SIZE] = {
	{	// s_typeOperators[STREAM_DATA_NIL]
		NULL, NULL, NULL, NULL 
	},
	{	// s_typeOperators[STREAM_DATA_CHAR]
		&util::CStoreStream::WriteCharByType, 
		&util::CStoreStream::ReadCharByType,
		&util::CStoreStream::WriteCharFromString,
		&util::CStoreStream::ReadCharToString,
	},
	{	// s_typeOperators[STREAM_DATA_CHAR_NULL]
		&util::CStoreStream::WriteCharByType,
		&util::CStoreStream::ReadCharByType,
		&util::CStoreStream::WriteCharFromString,
		&util::CStoreStream::ReadCharToString,
	},
	{	// s_typeOperators[STREAM_DATA_BOOL]
		&util::CStoreStream::WriteBoolByType,
		&util::CStoreStream::ReadBoolByType,
		&util::CStoreStream::WriteBoolFromString,
		&util::CStoreStream::ReadBoolToString,
	},
	{	// s_typeOperators[STREAM_DATA_BOOL_NULL]
		&util::CStoreStream::WriteBoolByType,
		&util::CStoreStream::ReadBoolByType,
		&util::CStoreStream::WriteBoolFromString,
		&util::CStoreStream::ReadBoolToString,
	},
	{	// s_typeOperators[STREAM_DATA_INT8]
		&util::CStoreStream::WriteInt8ByType,
		&util::CStoreStream::ReadInt8ByType,
		&util::CStoreStream::WriteInt8FromString,
		&util::CStoreStream::ReadInt8ToString,
	},
	{	// s_typeOperators[STREAM_DATA_INT8_NULL]
		&util::CStoreStream::WriteInt8ByType,
		&util::CStoreStream::ReadInt8ByType,
		&util::CStoreStream::WriteInt8FromString,
		&util::CStoreStream::ReadInt8ToString,
	},
	{	// s_typeOperators[STREAM_DATA_UINT8]
		&util::CStoreStream::WriteUInt8ByType,
		&util::CStoreStream::ReadUInt8ByType,
		&util::CStoreStream::WriteUInt8FromString,
		&util::CStoreStream::ReadUInt8ToString,
	},
	{	// s_typeOperators[STREAM_DATA_UINT8_NULL]
		&util::CStoreStream::WriteUInt8ByType,
		&util::CStoreStream::ReadUInt8ByType,
		&util::CStoreStream::WriteUInt8FromString,
		&util::CStoreStream::ReadUInt8ToString,
	},
	{	// s_typeOperators[STREAM_DATA_INT16]
		&util::CStoreStream::WriteInt16ByType,
		&util::CStoreStream::ReadInt16ByType,
		&util::CStoreStream::WriteInt16FromString,
		&util::CStoreStream::ReadInt16ToString,
	},
	{	// s_typeOperators[STREAM_DATA_INT16_NULL]
		&util::CStoreStream::WriteInt16ByType,
		&util::CStoreStream::ReadInt16ByType,
		&util::CStoreStream::WriteInt16FromString,
		&util::CStoreStream::ReadInt16ToString,
	},
	{	// s_typeOperators[STREAM_DATA_UINT16]
		&util::CStoreStream::WriteUInt16ByType,
		&util::CStoreStream::ReadUInt16ByType,
		&util::CStoreStream::WriteUInt16FromString,
		&util::CStoreStream::ReadUInt16ToString,
	},
	{	// s_typeOperators[STREAM_DATA_UINT16_NULL]
		&util::CStoreStream::WriteUInt16ByType,
		&util::CStoreStream::ReadUInt16ByType,
		&util::CStoreStream::WriteUInt16FromString,
		&util::CStoreStream::ReadUInt16ToString,
	},
	{	// s_typeOperators[STREAM_DATA_INT32]
		&util::CStoreStream::WriteInt32ByType,
		&util::CStoreStream::ReadInt32ByType,
		&util::CStoreStream::WriteInt32FromString,
		&util::CStoreStream::ReadInt32ToString,
	},
	{	// s_typeOperators[STREAM_DATA_INT32_NULL]
		&util::CStoreStream::WriteInt32ByType,
		&util::CStoreStream::ReadInt32ByType,
		&util::CStoreStream::WriteInt32FromString,
		&util::CStoreStream::ReadInt32ToString,
	},
	{	// s_typeOperators[STREAM_DATA_UINT32]
		&util::CStoreStream::WriteUInt32ByType,
		&util::CStoreStream::ReadUInt32ByType,
		&util::CStoreStream::WriteUInt32FromString,
		&util::CStoreStream::ReadUInt32ToString,
	},
	{	// s_typeOperators[STREAM_DATA_UINT32_NULL]
		&util::CStoreStream::WriteUInt32ByType,
		&util::CStoreStream::ReadUInt32ByType,
		&util::CStoreStream::WriteUInt32FromString,
		&util::CStoreStream::ReadUInt32ToString,
	},
	{	// s_typeOperators[STREAM_DATA_FLOAT]
		&util::CStoreStream::WriteFloatByType,
		&util::CStoreStream::ReadFloatByType,
		&util::CStoreStream::WriteFloatFromString,
		&util::CStoreStream::ReadFloatToString,
	},
	{	// s_typeOperators[STREAM_DATA_FLOAT_NULL]
		&util::CStoreStream::WriteFloatByType,
		&util::CStoreStream::ReadFloatByType,
		&util::CStoreStream::WriteFloatFromString,
		&util::CStoreStream::ReadFloatToString,
	},
	{	// s_typeOperators[STREAM_DATA_INT64]
		&util::CStoreStream::WriteInt64ByType,
		&util::CStoreStream::ReadInt64ByType,
		&util::CStoreStream::WriteInt64FromString,
		&util::CStoreStream::ReadInt64ToString,
	},
	{	// s_typeOperators[STREAM_DATA_INT64_NULL]
		&util::CStoreStream::WriteInt64ByType,
		&util::CStoreStream::ReadInt64ByType,
		&util::CStoreStream::WriteInt64FromString,
		&util::CStoreStream::ReadInt64ToString,
	},
	{	// s_typeOperators[STREAM_DATA_UINT64]
		&util::CStoreStream::WriteUInt64ByType,
		&util::CStoreStream::ReadUInt64ByType,
		&util::CStoreStream::WriteUInt64FromString,
		&util::CStoreStream::ReadUInt64ToString,
	},
	{	// s_typeOperators[STREAM_DATA_UINT64_NULL]
		&util::CStoreStream::WriteUInt64ByType,
		&util::CStoreStream::ReadUInt64ByType,
		&util::CStoreStream::WriteUInt64FromString,
		&util::CStoreStream::ReadUInt64ToString,
	},
	{	// s_typeOperators[STREAM_DATA_DOUBLE]
		&util::CStoreStream::WriteDoubleByType,
		&util::CStoreStream::ReadDoubleByType,
		&util::CStoreStream::WriteDoubleFromString,
		&util::CStoreStream::ReadDoubleToString,
	},
	{	// s_typeOperators[STREAM_DATA_DOUBLE_NULL]
		&util::CStoreStream::WriteDoubleByType,
		&util::CStoreStream::ReadDoubleByType,
		&util::CStoreStream::WriteDoubleFromString,
		&util::CStoreStream::ReadDoubleToString,
	},
	{	// s_typeOperators[STREAM_DATA_STD_STRING]
		&util::CStoreStream::WriteStringByType,
		&util::CStoreStream::ReadStringByType,
		&util::CStoreStream::WriteStringFromString,
		&util::CStoreStream::ReadStringToString,
	},
	{	// s_typeOperators[STREAM_DATA_STD_STRING_NULL]
		&util::CStoreStream::WriteStringByType,
		&util::CStoreStream::ReadStringByType,
		&util::CStoreStream::WriteStringFromString,
		&util::CStoreStream::ReadStringToString,
	},
	{	// s_typeOperators[STREAM_DATA_C_STRING]
		&util::CStoreStream::WriteStringByType,
		&util::CStoreStream::ReadStringByType,
		&util::CStoreStream::WriteStringFromString,
		&util::CStoreStream::ReadStringToString,
	},
	{	// s_typeOperators[STREAM_DATA_C_STRING_NULL]
		&util::CStoreStream::WriteStringByType,
		&util::CStoreStream::ReadStringByType,
		&util::CStoreStream::WriteStringFromString,
		&util::CStoreStream::ReadStringToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_BOOL]
		&util::CStoreStream::WriteBoolSetByType,
		&util::CStoreStream::ReadBoolSetByType,
		&util::CStoreStream::WriteBoolSetFromString,
		&util::CStoreStream::ReadBoolSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_BOOL_NULL]
		&util::CStoreStream::WriteBoolSetByType,
		&util::CStoreStream::ReadBoolSetByType,
		&util::CStoreStream::WriteBoolSetFromString,
		&util::CStoreStream::ReadBoolSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_INT8]
		&util::CStoreStream::WriteInt8SetByType,
		&util::CStoreStream::ReadInt8SetByType,
		&util::CStoreStream::WriteInt8SetFromString,
		&util::CStoreStream::ReadInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_INT8_NULL]
		&util::CStoreStream::WriteInt8SetByType,
		&util::CStoreStream::ReadInt8SetByType,
		&util::CStoreStream::WriteInt8SetFromString,
		&util::CStoreStream::ReadInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_UINT8]
		&util::CStoreStream::WriteUInt8SetByType,
		&util::CStoreStream::ReadUInt8SetByType,
		&util::CStoreStream::WriteUInt8SetFromString,
		&util::CStoreStream::ReadUInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_UINT8_NULL]
		&util::CStoreStream::WriteUInt8SetByType,
		&util::CStoreStream::ReadUInt8SetByType,
		&util::CStoreStream::WriteUInt8SetFromString,
		&util::CStoreStream::ReadUInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_INT16]
		&util::CStoreStream::WriteInt16SetByType,
		&util::CStoreStream::ReadInt16SetByType,
		&util::CStoreStream::WriteInt16SetFromString,
		&util::CStoreStream::ReadInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_INT16_NULL]
		&util::CStoreStream::WriteInt16SetByType,
		&util::CStoreStream::ReadInt16SetByType,
		&util::CStoreStream::WriteInt16SetFromString,
		&util::CStoreStream::ReadInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_UINT16]
		&util::CStoreStream::WriteUInt16SetByType,
		&util::CStoreStream::ReadUInt16SetByType,
		&util::CStoreStream::WriteUInt16SetFromString,
		&util::CStoreStream::ReadUInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_UINT16_NULL]
		&util::CStoreStream::WriteUInt16SetByType,
		&util::CStoreStream::ReadUInt16SetByType,
		&util::CStoreStream::WriteUInt16SetFromString,
		&util::CStoreStream::ReadUInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_INT32]
		&util::CStoreStream::WriteInt32SetByType,
		&util::CStoreStream::ReadInt32SetByType,
		&util::CStoreStream::WriteInt32SetFromString,
		&util::CStoreStream::ReadInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_INT32_NULL]
		&util::CStoreStream::WriteInt32SetByType,
		&util::CStoreStream::ReadInt32SetByType,
		&util::CStoreStream::WriteInt32SetFromString,
		&util::CStoreStream::ReadInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_UINT32]
		&util::CStoreStream::WriteUInt32SetByType,
		&util::CStoreStream::ReadUInt32SetByType,
		&util::CStoreStream::WriteUInt32SetFromString,
		&util::CStoreStream::ReadUInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_UINT32_NULL]
		&util::CStoreStream::WriteUInt32SetByType,
		&util::CStoreStream::ReadUInt32SetByType,
		&util::CStoreStream::WriteUInt32SetFromString,
		&util::CStoreStream::ReadUInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_FLOAT]
		&util::CStoreStream::WriteFloatSetByType,
		&util::CStoreStream::ReadFloatSetByType,
		&util::CStoreStream::WriteFloatSetFromString,
		&util::CStoreStream::ReadFloatSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_FLOAT_NULL]
		&util::CStoreStream::WriteFloatSetByType,
		&util::CStoreStream::ReadFloatSetByType,
		&util::CStoreStream::WriteFloatSetFromString,
		&util::CStoreStream::ReadFloatSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_INT64]
		&util::CStoreStream::WriteInt64SetByType,
		&util::CStoreStream::ReadInt64SetByType,
		&util::CStoreStream::WriteInt64SetFromString,
		&util::CStoreStream::ReadInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_INT64_NULL]
		&util::CStoreStream::WriteInt64SetByType,
		&util::CStoreStream::ReadInt64SetByType,
		&util::CStoreStream::WriteInt64SetFromString,
		&util::CStoreStream::ReadInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_UINT64]
		&util::CStoreStream::WriteUInt64SetByType,
		&util::CStoreStream::ReadUInt64SetByType,
		&util::CStoreStream::WriteUInt64SetFromString,
		&util::CStoreStream::ReadUInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_UINT64_NULL]
		&util::CStoreStream::WriteUInt64SetByType,
		&util::CStoreStream::ReadUInt64SetByType,
		&util::CStoreStream::WriteUInt64SetFromString,
		&util::CStoreStream::ReadUInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_DOUBLE]
		&util::CStoreStream::WriteDoubleSetByType,
		&util::CStoreStream::ReadDoubleSetByType,
		&util::CStoreStream::WriteDoubleSetFromString,
		&util::CStoreStream::ReadDoubleSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_DOUBLE_NULL]
		&util::CStoreStream::WriteDoubleSetByType,
		&util::CStoreStream::ReadDoubleSetByType,
		&util::CStoreStream::WriteDoubleSetFromString,
		&util::CStoreStream::ReadDoubleSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_STRING]
		&util::CStoreStream::WriteStringSetByType,
		&util::CStoreStream::ReadStringSetByType,
		&util::CStoreStream::WriteStringSetFromString,
		&util::CStoreStream::ReadStringSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_VECTOR_STRING_NULL]
		&util::CStoreStream::WriteStringSetByType,
		&util::CStoreStream::ReadStringSetByType,
		&util::CStoreStream::WriteStringSetFromString,
		&util::CStoreStream::ReadStringSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_BOOL]
		&util::CStoreStream::WriteBoolSetByType,
		&util::CStoreStream::ReadBoolSetByType,
		&util::CStoreStream::WriteBoolSetFromString,
		&util::CStoreStream::ReadBoolSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_BOOL_NULL]
		&util::CStoreStream::WriteBoolSetByType,
		&util::CStoreStream::ReadBoolSetByType,
		&util::CStoreStream::WriteBoolSetFromString,
		&util::CStoreStream::ReadBoolSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_INT8]    
		&util::CStoreStream::WriteInt8SetByType,
		&util::CStoreStream::ReadInt8SetByType,
		&util::CStoreStream::WriteInt8SetFromString,
		&util::CStoreStream::ReadInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_INT8_NULL]
		&util::CStoreStream::WriteInt8SetByType,
		&util::CStoreStream::ReadInt8SetByType,
		&util::CStoreStream::WriteInt8SetFromString,
		&util::CStoreStream::ReadInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_UINT8]
		&util::CStoreStream::WriteUInt8SetByType,
		&util::CStoreStream::ReadUInt8SetByType,
		&util::CStoreStream::WriteUInt8SetFromString,
		&util::CStoreStream::ReadUInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_UINT8_NULL]
		&util::CStoreStream::WriteUInt8SetByType,
		&util::CStoreStream::ReadUInt8SetByType,
		&util::CStoreStream::WriteUInt8SetFromString,
		&util::CStoreStream::ReadUInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_INT16]
		&util::CStoreStream::WriteInt16SetByType,
		&util::CStoreStream::ReadInt16SetByType,
		&util::CStoreStream::WriteInt16SetFromString,
		&util::CStoreStream::ReadInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_INT16_NULL]
		&util::CStoreStream::WriteInt16SetByType,
		&util::CStoreStream::ReadInt16SetByType,
		&util::CStoreStream::WriteInt16SetFromString,
		&util::CStoreStream::ReadInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_UINT16]
		&util::CStoreStream::WriteUInt16SetByType,
		&util::CStoreStream::ReadUInt16SetByType,
		&util::CStoreStream::WriteUInt16SetFromString,
		&util::CStoreStream::ReadUInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_UINT16_NULL]
		&util::CStoreStream::WriteUInt16SetByType,
		&util::CStoreStream::ReadUInt16SetByType,
		&util::CStoreStream::WriteUInt16SetFromString,
		&util::CStoreStream::ReadUInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_INT32]
		&util::CStoreStream::WriteInt32SetByType,
		&util::CStoreStream::ReadInt32SetByType,
		&util::CStoreStream::WriteInt32SetFromString,
		&util::CStoreStream::ReadInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_INT32_NULL]
		&util::CStoreStream::WriteInt32SetByType,
		&util::CStoreStream::ReadInt32SetByType,
		&util::CStoreStream::WriteInt32SetFromString,
		&util::CStoreStream::ReadInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_UINT32]
		&util::CStoreStream::WriteUInt32SetByType,
		&util::CStoreStream::ReadUInt32SetByType,
		&util::CStoreStream::WriteUInt32SetFromString,
		&util::CStoreStream::ReadUInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_UINT32_NULL]
		&util::CStoreStream::WriteUInt32SetByType,
		&util::CStoreStream::ReadUInt32SetByType,
		&util::CStoreStream::WriteUInt32SetFromString,
		&util::CStoreStream::ReadUInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_FLOAT]
		&util::CStoreStream::WriteFloatSetByType,
		&util::CStoreStream::ReadFloatSetByType,
		&util::CStoreStream::WriteFloatSetFromString,
		&util::CStoreStream::ReadFloatSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_FLOAT_NULL]
		&util::CStoreStream::WriteFloatSetByType,
		&util::CStoreStream::ReadFloatSetByType,
		&util::CStoreStream::WriteFloatSetFromString,
		&util::CStoreStream::ReadFloatSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_INT64]
		&util::CStoreStream::WriteInt64SetByType,
		&util::CStoreStream::ReadInt64SetByType,
		&util::CStoreStream::WriteInt64SetFromString,
		&util::CStoreStream::ReadInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_INT64_NULL]
		&util::CStoreStream::WriteInt64SetByType,
		&util::CStoreStream::ReadInt64SetByType,
		&util::CStoreStream::WriteInt64SetFromString,
		&util::CStoreStream::ReadInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_UINT64]
		&util::CStoreStream::WriteUInt64SetByType,
		&util::CStoreStream::ReadUInt64SetByType,
		&util::CStoreStream::WriteUInt64SetFromString,
		&util::CStoreStream::ReadUInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_UINT64_NULL]
		&util::CStoreStream::WriteUInt64SetByType,
		&util::CStoreStream::ReadUInt64SetByType,
		&util::CStoreStream::WriteUInt64SetFromString,
		&util::CStoreStream::ReadUInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_DOUBLE]
		&util::CStoreStream::WriteDoubleSetByType,
		&util::CStoreStream::ReadDoubleSetByType,
		&util::CStoreStream::WriteDoubleSetFromString,
		&util::CStoreStream::ReadDoubleSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_DOUBLE_NULL]
		&util::CStoreStream::WriteDoubleSetByType,
		&util::CStoreStream::ReadDoubleSetByType,
		&util::CStoreStream::WriteDoubleSetFromString,
		&util::CStoreStream::ReadDoubleSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_STRING]
		&util::CStoreStream::WriteStringSetByType,
		&util::CStoreStream::ReadStringSetByType,
		&util::CStoreStream::WriteStringSetFromString,
		&util::CStoreStream::ReadStringSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_SET_STRING_NULL]
		&util::CStoreStream::WriteStringSetByType,
		&util::CStoreStream::ReadStringSetByType,
		&util::CStoreStream::WriteStringSetFromString,
		&util::CStoreStream::ReadStringSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_BOOL]
		&util::CStoreStream::WriteBoolSetByType,
		&util::CStoreStream::ReadBoolSetByType,
		&util::CStoreStream::WriteBoolSetFromString,
		&util::CStoreStream::ReadBoolSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_BOOL_NULL]
		&util::CStoreStream::WriteBoolSetByType,
		&util::CStoreStream::ReadBoolSetByType,
		&util::CStoreStream::WriteBoolSetFromString,
		&util::CStoreStream::ReadBoolSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_INT8]
		&util::CStoreStream::WriteInt8SetByType,
		&util::CStoreStream::ReadInt8SetByType,
		&util::CStoreStream::WriteInt8SetFromString,
		&util::CStoreStream::ReadInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_INT8_NULL]
		&util::CStoreStream::WriteInt8SetByType,
		&util::CStoreStream::ReadInt8SetByType,
		&util::CStoreStream::WriteInt8SetFromString,
		&util::CStoreStream::ReadInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_UINT8]
		&util::CStoreStream::WriteUInt8SetByType,
		&util::CStoreStream::ReadUInt8SetByType,
		&util::CStoreStream::WriteUInt8SetFromString,
		&util::CStoreStream::ReadUInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_UINT8_NULL]
		&util::CStoreStream::WriteUInt8SetByType,
		&util::CStoreStream::ReadUInt8SetByType,
		&util::CStoreStream::WriteUInt8SetFromString,
		&util::CStoreStream::ReadUInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_INT16]
		&util::CStoreStream::WriteInt16SetByType,
		&util::CStoreStream::ReadInt16SetByType,
		&util::CStoreStream::WriteInt16SetFromString,
		&util::CStoreStream::ReadInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_INT16_NULL]
		&util::CStoreStream::WriteInt16SetByType,
		&util::CStoreStream::ReadInt16SetByType,
		&util::CStoreStream::WriteInt16SetFromString,
		&util::CStoreStream::ReadInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_UINT16]
		&util::CStoreStream::WriteUInt16SetByType,
		&util::CStoreStream::ReadUInt16SetByType,
		&util::CStoreStream::WriteUInt16SetFromString,
		&util::CStoreStream::ReadUInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_UINT16_NULL]
		&util::CStoreStream::WriteUInt16SetByType,
		&util::CStoreStream::ReadUInt16SetByType,
		&util::CStoreStream::WriteUInt16SetFromString,
		&util::CStoreStream::ReadUInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_INT32]
		&util::CStoreStream::WriteInt32SetByType,
		&util::CStoreStream::ReadInt32SetByType,
		&util::CStoreStream::WriteInt32SetFromString,
		&util::CStoreStream::ReadInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_INT32_NULL]
		&util::CStoreStream::WriteInt32SetByType,
		&util::CStoreStream::ReadInt32SetByType,
		&util::CStoreStream::WriteInt32SetFromString,
		&util::CStoreStream::ReadInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_UINT32]
		&util::CStoreStream::WriteUInt32SetByType,
		&util::CStoreStream::ReadUInt32SetByType,
		&util::CStoreStream::WriteUInt32SetFromString,
		&util::CStoreStream::ReadUInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_UINT32_NULL]
		&util::CStoreStream::WriteUInt32SetByType,
		&util::CStoreStream::ReadUInt32SetByType,
		&util::CStoreStream::WriteUInt32SetFromString,
		&util::CStoreStream::ReadUInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_FLOAT]
		&util::CStoreStream::WriteFloatSetByType,
		&util::CStoreStream::ReadFloatSetByType,
		&util::CStoreStream::WriteFloatSetFromString,
		&util::CStoreStream::ReadFloatSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_FLOAT_NULL]
		&util::CStoreStream::WriteFloatSetByType,
		&util::CStoreStream::ReadFloatSetByType,
		&util::CStoreStream::WriteFloatSetFromString,
		&util::CStoreStream::ReadFloatSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_INT64]
		&util::CStoreStream::WriteInt64SetByType,
		&util::CStoreStream::ReadInt64SetByType,
		&util::CStoreStream::WriteInt64SetFromString,
		&util::CStoreStream::ReadInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_INT64_NULL]
		&util::CStoreStream::WriteInt64SetByType,
		&util::CStoreStream::ReadInt64SetByType,
		&util::CStoreStream::WriteInt64SetFromString,
		&util::CStoreStream::ReadInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_UINT64]
		&util::CStoreStream::WriteUInt64SetByType,
		&util::CStoreStream::ReadUInt64SetByType,
		&util::CStoreStream::WriteUInt64SetFromString,
		&util::CStoreStream::ReadUInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_UINT64_NULL]
		&util::CStoreStream::WriteUInt64SetByType,
		&util::CStoreStream::ReadUInt64SetByType,
		&util::CStoreStream::WriteUInt64SetFromString,
		&util::CStoreStream::ReadUInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_DOUBLE]
		&util::CStoreStream::WriteDoubleSetByType,
		&util::CStoreStream::ReadDoubleSetByType,
		&util::CStoreStream::WriteDoubleSetFromString,
		&util::CStoreStream::ReadDoubleSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_DOUBLE_NULL]
		&util::CStoreStream::WriteDoubleSetByType,
		&util::CStoreStream::ReadDoubleSetByType,
		&util::CStoreStream::WriteDoubleSetFromString,
		&util::CStoreStream::ReadDoubleSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_STRING]
		&util::CStoreStream::WriteStringSetByType,
		&util::CStoreStream::ReadStringSetByType,
		&util::CStoreStream::WriteStringSetFromString,
		&util::CStoreStream::ReadStringSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_LIST_STRING_NULL]
		&util::CStoreStream::WriteStringSetByType,
		&util::CStoreStream::ReadStringSetByType,
		&util::CStoreStream::WriteStringSetFromString,
		&util::CStoreStream::ReadStringSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_BOOL]
		&util::CStoreStream::WriteBoolSetByType,
		&util::CStoreStream::ReadBoolSetByType,
		&util::CStoreStream::WriteBoolSetFromString,
		&util::CStoreStream::ReadBoolSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_BOOL_NULL]
		&util::CStoreStream::WriteBoolSetByType,
		&util::CStoreStream::ReadBoolSetByType,
		&util::CStoreStream::WriteBoolSetFromString,
		&util::CStoreStream::ReadBoolSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_INT8]
		&util::CStoreStream::WriteInt8SetByType,
		&util::CStoreStream::ReadInt8SetByType,
		&util::CStoreStream::WriteInt8SetFromString,
		&util::CStoreStream::ReadInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_INT8_NULL]
		&util::CStoreStream::WriteInt8SetByType,
		&util::CStoreStream::ReadInt8SetByType,
		&util::CStoreStream::WriteInt8SetFromString,
		&util::CStoreStream::ReadInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_UINT8]
		&util::CStoreStream::WriteUInt8SetByType,
		&util::CStoreStream::ReadUInt8SetByType,
		&util::CStoreStream::WriteUInt8SetFromString,
		&util::CStoreStream::ReadUInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_UINT8_NULL]
		&util::CStoreStream::WriteUInt8SetByType,
		&util::CStoreStream::ReadUInt8SetByType,
		&util::CStoreStream::WriteUInt8SetFromString,
		&util::CStoreStream::ReadUInt8SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_INT16]
		&util::CStoreStream::WriteInt16SetByType,
		&util::CStoreStream::ReadInt16SetByType,
		&util::CStoreStream::WriteInt16SetFromString,
		&util::CStoreStream::ReadInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_INT16_NULL]
		&util::CStoreStream::WriteInt16SetByType,
		&util::CStoreStream::ReadInt16SetByType,
		&util::CStoreStream::WriteInt16SetFromString,
		&util::CStoreStream::ReadInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_UINT16]
		&util::CStoreStream::WriteUInt16SetByType,
		&util::CStoreStream::ReadUInt16SetByType,
		&util::CStoreStream::WriteUInt16SetFromString,
		&util::CStoreStream::ReadUInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_UINT16_NULL]
		&util::CStoreStream::WriteUInt16SetByType,
		&util::CStoreStream::ReadUInt16SetByType,
		&util::CStoreStream::WriteUInt16SetFromString,
		&util::CStoreStream::ReadUInt16SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_INT32]
		&util::CStoreStream::WriteInt32SetByType,
		&util::CStoreStream::ReadInt32SetByType,
		&util::CStoreStream::WriteInt32SetFromString,
		&util::CStoreStream::ReadInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_INT32_NULL]
		&util::CStoreStream::WriteInt32SetByType,
		&util::CStoreStream::ReadInt32SetByType,
		&util::CStoreStream::WriteInt32SetFromString,
		&util::CStoreStream::ReadInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_UINT32]
		&util::CStoreStream::WriteUInt32SetByType,
		&util::CStoreStream::ReadUInt32SetByType,
		&util::CStoreStream::WriteUInt32SetFromString,
		&util::CStoreStream::ReadUInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_UINT32_NULL]
		&util::CStoreStream::WriteUInt32SetByType,
		&util::CStoreStream::ReadUInt32SetByType,
		&util::CStoreStream::WriteUInt32SetFromString,
		&util::CStoreStream::ReadUInt32SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_FLOAT]
		&util::CStoreStream::WriteFloatSetByType,
		&util::CStoreStream::ReadFloatSetByType,
		&util::CStoreStream::WriteFloatSetFromString,
		&util::CStoreStream::ReadFloatSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_FLOAT_NULL]
		&util::CStoreStream::WriteFloatSetByType,
		&util::CStoreStream::ReadFloatSetByType,
		&util::CStoreStream::WriteFloatSetFromString,
		&util::CStoreStream::ReadFloatSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_INT64]
		&util::CStoreStream::WriteInt64SetByType,
		&util::CStoreStream::ReadInt64SetByType,
		&util::CStoreStream::WriteInt64SetFromString,
		&util::CStoreStream::ReadInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_INT64_NULL]
		&util::CStoreStream::WriteInt64SetByType,
		&util::CStoreStream::ReadInt64SetByType,
		&util::CStoreStream::WriteInt64SetFromString,
		&util::CStoreStream::ReadInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_UINT64]
		&util::CStoreStream::WriteUInt64SetByType,
		&util::CStoreStream::ReadUInt64SetByType,
		&util::CStoreStream::WriteUInt64SetFromString,
		&util::CStoreStream::ReadUInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_UINT64_NULL]
		&util::CStoreStream::WriteUInt64SetByType,
		&util::CStoreStream::ReadUInt64SetByType,
		&util::CStoreStream::WriteUInt64SetFromString,
		&util::CStoreStream::ReadUInt64SetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_DOUBLE]
		&util::CStoreStream::WriteDoubleSetByType,
		&util::CStoreStream::ReadDoubleSetByType,
		&util::CStoreStream::WriteDoubleSetFromString,
		&util::CStoreStream::ReadDoubleSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_DOUBLE_NULL]
		&util::CStoreStream::WriteDoubleSetByType,
		&util::CStoreStream::ReadDoubleSetByType,
		&util::CStoreStream::WriteDoubleSetFromString,
		&util::CStoreStream::ReadDoubleSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_STRING]
		&util::CStoreStream::WriteStringSetByType,
		&util::CStoreStream::ReadStringSetByType,
		&util::CStoreStream::WriteStringSetFromString,
		&util::CStoreStream::ReadStringSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_ARRAY_STRING_NULL]
		&util::CStoreStream::WriteStringSetByType,
		&util::CStoreStream::ReadStringSetByType,
		&util::CStoreStream::WriteStringSetFromString,
		&util::CStoreStream::ReadStringSetToString,
	},
	{	// s_typeOperators[STREAM_DATA_TINY_JSON]
		&util::CStoreStream::WriteStringByType,
		&util::CStoreStream::ReadStringByType,
		&util::CStoreStream::WriteStringFromString,
		&util::CStoreStream::ReadStringToString,
	},
	{	// s_typeOperators[STREAM_DATA_TINY_JSON_NULL]
		&util::CStoreStream::WriteStringByType,
		&util::CStoreStream::ReadStringByType,
		&util::CStoreStream::WriteStringFromString,
		&util::CStoreStream::ReadStringToString,
	}
};
















