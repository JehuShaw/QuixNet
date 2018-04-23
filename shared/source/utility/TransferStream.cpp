#include "TransferStream.h"
#include <assert.h>
#include <string.h>

#ifdef _WIN32
	#include <stdlib.h>
	#include <memory.h>
	#include <stdio.h>
	#include <float.h>
#endif


#if defined ( __APPLE__ ) || defined ( __APPLE_CC__ )
	#include <malloc/malloc.h>
#else
	#include <malloc.h>
	#include <string>
#endif


static bool IsLocalLittleEndian()
{
	union w {
		int a;
		char b;
	} c;
	c.a = 1;
	return(c.b ==1);
}

inline static void ReverseBytes(char *input, int length){
	int count = length/2;
	for(int i = 0; i < count; ++i){
		input[i] ^= input[length-i-1] ^= input[i] ^= input[length-i-1];
	}
}


#define MOD8(value) ((value) & 0x7)
#define DIV8(value) ((value) >> 3)
#define MUL8(value) ((value) << 3)

using namespace util;

bool util::CTransferStream::s_bInitTypeOperator = false;
struct util::CTransferStream::TypeOperatorSet util::CTransferStream::s_typeOperators[STREAM_DATA_SIZE];

CTransferStream::CTransferStream()
{
	InitTypeOperator();
	m_isLittleEndian = IsLocalLittleEndian();
	m_writeOffset = 0;
	m_allocBitSize = TRANSFERSTREAM_STACK_ALLOCA_SIZE * 8;
	m_readOffset = 0;
	m_data = (unsigned char*) m_stackData;
	m_copyData = true;
}

CTransferStream::CTransferStream(uint32_t initByteSize)
{
	InitTypeOperator();
	m_isLittleEndian = IsLocalLittleEndian();
	m_writeOffset = 0;
	m_readOffset = 0;
	if(initByteSize <= TRANSFERSTREAM_STACK_ALLOCA_SIZE) {
		m_data = (unsigned char*) m_stackData;
		m_allocBitSize = TRANSFERSTREAM_STACK_ALLOCA_SIZE * 8;
	} else {
		m_data = (unsigned char*) malloc(initByteSize);
		m_allocBitSize = initByteSize << 3;
	}
#ifdef _DEBUG
	assert(m_data);
#endif
	m_copyData = true;
}

CTransferStream::CTransferStream(const char* data, uint32_t lengthInBytes, bool copyData)
{
	InitTypeOperator();
	m_isLittleEndian = IsLocalLittleEndian();
	m_writeOffset = MUL8(lengthInBytes);
	m_readOffset = 0;
	m_copyData = copyData;
	m_allocBitSize = MUL8(lengthInBytes);

	if(m_copyData) {
		if(lengthInBytes > 0) {
			if(lengthInBytes < TRANSFERSTREAM_STACK_ALLOCA_SIZE) {
				m_data = ( unsigned char* ) m_stackData;
				m_allocBitSize = MUL8(TRANSFERSTREAM_STACK_ALLOCA_SIZE);
			} else {
				m_data = (unsigned char*) malloc(lengthInBytes);
			}
#ifdef _DEBUG
			assert(m_data);
#endif
			memcpy(m_data, data, lengthInBytes);
		} else {
			m_data = NULL;
		}
	} else {
		m_data = (unsigned char*)data;
	}
}

CTransferStream::CTransferStream(const std::string& strData, bool copyData)
{
	InitTypeOperator();
	const char* data = strData.c_str();
	uint32_t lengthInBytes = strData.length();
	m_isLittleEndian = IsLocalLittleEndian();
	m_writeOffset = MUL8(lengthInBytes);
	m_readOffset = 0;
	m_copyData = copyData;
	m_allocBitSize = MUL8(lengthInBytes);

	if(m_copyData) {
		if(lengthInBytes > 0) {
			if(lengthInBytes < TRANSFERSTREAM_STACK_ALLOCA_SIZE) {
				m_data = ( unsigned char* ) m_stackData;
				m_allocBitSize = MUL8(TRANSFERSTREAM_STACK_ALLOCA_SIZE);
			} else {
				m_data = (unsigned char*) malloc(lengthInBytes);
			}
#ifdef _DEBUG
			assert(m_data);
#endif
			memcpy(m_data, data, lengthInBytes);
		} else {
			m_data = NULL;
		}
	} else {
		m_data = (unsigned char*)data;
	}
}

CTransferStream::CTransferStream(const CTransferStream& orig)
{
	InitTypeOperator();

	m_isLittleEndian = IsLocalLittleEndian();
	m_writeOffset = orig.m_writeOffset;
	m_readOffset = 0;
	m_copyData = orig.m_copyData;
	m_allocBitSize = orig.m_writeOffset;

	if(m_copyData) {
		uint32_t lengthInBytes = TS_BITS_TO_BYTES(orig.m_writeOffset);
		if(lengthInBytes > 0) {
			if(lengthInBytes < TRANSFERSTREAM_STACK_ALLOCA_SIZE) {
				m_data = (unsigned char*) m_stackData;
				m_allocBitSize = MUL8(TRANSFERSTREAM_STACK_ALLOCA_SIZE);
			} else {
				m_data = (unsigned char*) malloc(lengthInBytes);
			}
#ifdef _DEBUG
			assert(m_data);
#endif
			memcpy(m_data, orig.m_data, lengthInBytes);
		} else {
			m_data = NULL;
		}
	} else {
		m_data = orig.m_data;
	}
}


// Use this if you pass a pointer copy to the constructor (_copyData==false) and want to overallocate to prevent reallocation
void CTransferStream::SetNumberOfBitsAllocated(const uint32_t lengthInBits)
{
#ifdef _DEBUG
	assert(lengthInBits >= m_allocBitSize);
#endif
	m_allocBitSize = lengthInBits;
}

CTransferStream::~CTransferStream()
{
	if(m_copyData && m_stackData != m_data) {
		// Use realloc and free so we are more efficient than delete and new for resizing
		free(m_data);
		m_data = NULL;
	}
}

void CTransferStream::Reset(void)
{
	m_writeOffset = 0;
	m_readOffset = 0;
}

// Write the native types to the end of the buffer
CTransferStream& CTransferStream::operator<<(const char input)
{
	WriteBits((unsigned char*) &input, sizeof(input) * 8, true);
	return *this;
}

CTransferStream& CTransferStream::operator<<(const bool input)
{
	if(input) {
		Write1();
	} else {
		Write0();
	}
	return *this;
}

CTransferStream& CTransferStream::operator<<(const uint8_t input)
{
	WriteBits((unsigned char*) &input, sizeof(input) * 8, true);
	return *this;
}

CTransferStream& CTransferStream::operator<<(const int8_t input)
{
	WriteBits((unsigned char*) &input, sizeof(input) * 8, true);
	return *this;
}

CTransferStream& CTransferStream::operator<<(const uint16_t input)
{
	if(m_isLittleEndian) {
		ReverseBytes((char*)&input, sizeof(uint16_t));
	}
	WriteBits((unsigned char*) &input, sizeof(input) * 8, true);
	return *this;
}

CTransferStream& CTransferStream::operator<<(const int16_t input)
{
	if(m_isLittleEndian) {
		ReverseBytes((char*)&input, sizeof(int16_t));
	}
	WriteBits((unsigned char*) &input, sizeof(input) * 8, true);
	return *this;
}

CTransferStream& CTransferStream::operator<<(const uint32_t input)
{
	if(m_isLittleEndian) {
		ReverseBytes((char*)&input, sizeof(uint32_t));
	}
	WriteBits((unsigned char*) &input, sizeof(input) * 8, true);
	return *this;
}

CTransferStream& CTransferStream::operator<<(const int32_t input)
{
	if(m_isLittleEndian) {
		ReverseBytes((char*)&input, sizeof(int32_t));
	}
	WriteBits((unsigned char*) &input, sizeof(input) * 8, true);
	return *this;
}

#ifndef NO_INT64
CTransferStream& CTransferStream::operator<<(const uint64_t input)
{
	if(m_isLittleEndian) {
		ReverseBytes((char*)&input, sizeof(uint64_t));
	}
	WriteBits((unsigned char*) &input, sizeof(input) * 8, true);
	return *this;
}

CTransferStream& CTransferStream::operator<<(const int64_t input)
{
	if(m_isLittleEndian) {
		ReverseBytes((char*)&input, sizeof(int64_t));
	}
	WriteBits((unsigned char*) &input, sizeof(input) * 8, true);
	return *this;
}

#endif

CTransferStream& CTransferStream::operator<<(const float input)
{
	if(m_isLittleEndian) {
		ReverseBytes((char*)&input, sizeof(float));
	}
	WriteBits((unsigned char*) &input, sizeof(input) * 8, true);
	return *this;
}

CTransferStream& CTransferStream::operator<<(const double input)
{
	if(m_isLittleEndian) {
		ReverseBytes((char*)&input, sizeof(double));
	}
	WriteBits((unsigned char*) &input, sizeof(input) * 8, true);
	return *this;
}
// Write an array or casted stream
void CTransferStream::WriteBytes(const char* input, const int numberOfBytes)
{
	WriteBits((unsigned char*) input, numberOfBytes * 8, true);
}

CTransferStream& CTransferStream::operator<<(const char input[])
{
    uint16_t len = (uint16_t)strlen(input);
	*this << len;

    if(len > 0){
        WriteBytes(input, len);
    }
	return *this;
}

#ifndef NO_TEMPLATE

CTransferStream& CTransferStream::operator<<(const std::string& input)
{
	uint16_t len = (uint16_t)input.size();
	*this << len;

	if(len > 0){
		WriteBytes(input.data(), len);
	}
	return *this;
}

#endif

//////////////////////////////////////////////////////////////////////////
void CTransferStream::ReadByType(char& output, uint8_t nType)
{
	assert(IsSingleType(nType));

	switch(nType) {
	case STREAM_DATA_CHAR:
	case STREAM_DATA_CHAR_NULL:
		{
			*this >> output;
		}
		break;
	case STREAM_DATA_BOOL:
	case STREAM_DATA_BOOL_NULL:
		{
			bool temp = false;
			*this >> temp;
			output = (char)temp;
		}
		break;
	case STREAM_DATA_INT8:
	case STREAM_DATA_INT8_NULL:
		{
			int8_t temp = 0;
			*this >> temp;
			output = (char)temp;
		}
		break;
	case STREAM_DATA_UINT8:
	case STREAM_DATA_UINT8_NULL:
		{
			uint8_t temp = 0;
			*this >> temp;
			output = (char)temp;
		}
		break;
	case STREAM_DATA_INT16:
	case STREAM_DATA_INT16_NULL:
		{
			int16_t temp = 0;
			*this >> temp;
			output = (char)temp;
		}
		break;
	case STREAM_DATA_UINT16:
	case STREAM_DATA_UINT16_NULL:
		{
			uint16_t temp = 0;
			*this >> temp;
			output = (char)temp;
		}
		break;
	case STREAM_DATA_INT32:
	case STREAM_DATA_INT32_NULL:
		{
			int32_t temp = 0;
			*this >> temp;
			output = (char)temp;
		}
		break;
	case STREAM_DATA_UINT32:
	case STREAM_DATA_UINT32_NULL:
		{
			uint32_t temp = 0;
			*this >> temp;
			output = (char)temp;
		}
		break;
	case STREAM_DATA_INT64:
	case STREAM_DATA_INT64_NULL:
		{
			int64_t temp = 0;
			*this >> temp;
			output = (char)temp;
		}
		break;
	case STREAM_DATA_UINT64:
	case STREAM_DATA_UINT64_NULL:
		{
			uint64_t temp = 0;
			*this >> temp;
			output = (char)temp;
		}
		break;
	case STREAM_DATA_FLOAT:
	case STREAM_DATA_FLOAT_NULL:
		{
			float temp = 0.0f;
			*this >> temp;
			output = (char)temp;
		}
		break;
	case STREAM_DATA_DOUBLE:
	case STREAM_DATA_DOUBLE_NULL:
		{
			double temp = 0.0;
			*this >> temp;
			output = (char)temp;
		}
		break;
	case STREAM_DATA_C_STRING:
	case STREAM_DATA_C_STRING_NULL:
	case STREAM_DATA_STD_STRING:
	case STREAM_DATA_STD_STRING_NULL:
		{
			uint16_t len = 0;
			*this >> len;
			if(len > 0) {
				char szBuf[TS_STRING_MAX_SIZE];
				ReadBytes(szBuf, len);
				szBuf[len] = '\0';
				CSeparatedStream separated(szBuf, len,
					false, TS_STRING_DELIM, TS_STRING_DELIM);
				separated >> output;
			}
		}
		break;
	default:
		assert(false);
		break;
	};
}

void CTransferStream::ReadByType(bool& output, uint8_t nType)
{
	assert(IsSingleType(nType));

	switch(nType) {
	case STREAM_DATA_CHAR:
	case STREAM_DATA_CHAR_NULL:
		{
			char temp = 0;
			*this >> temp;
			output = (0 != temp);
		}
		break;
	case STREAM_DATA_BOOL:
	case STREAM_DATA_BOOL_NULL:
		{
			*this >> output;
		}
		break;
	case STREAM_DATA_INT8:
	case STREAM_DATA_INT8_NULL:
		{
			int8_t temp = 0;
			*this >> temp;
			output = (0 != temp);
		}
		break;
	case STREAM_DATA_UINT8:
	case STREAM_DATA_UINT8_NULL:
		{
			uint8_t temp = 0;
			*this >> temp;
			output = (0 != temp);
		}
		break;
	case STREAM_DATA_INT16:
	case STREAM_DATA_INT16_NULL:
		{
			int16_t temp = 0;
			*this >> temp;
			output = (0 != temp);
		}
		break;
	case STREAM_DATA_UINT16:
	case STREAM_DATA_UINT16_NULL:
		{
			uint16_t temp = 0;
			*this >> temp;
			output = (0 != temp);
		}
		break;
	case STREAM_DATA_INT32:
	case STREAM_DATA_INT32_NULL:
		{
			int32_t temp = 0;
			*this >> temp;
			output = (0 != temp);
		}
		break;
	case STREAM_DATA_UINT32:
	case STREAM_DATA_UINT32_NULL:
		{
			uint32_t temp = 0;
			*this >> temp;
			output = (0 != temp);
		}
		break;
	case STREAM_DATA_INT64:
	case STREAM_DATA_INT64_NULL:
		{
			int64_t temp = 0;
			*this >> temp;
			output = (0 != temp);
		}
		break;
	case STREAM_DATA_UINT64:
	case STREAM_DATA_UINT64_NULL:
		{
			uint64_t temp = 0;
			*this >> temp;
			output = (0 != temp);
		}
		break;
	case STREAM_DATA_FLOAT:
	case STREAM_DATA_FLOAT_NULL:
		{
			float temp = 0.0f;
			*this >> temp;
			output = (0.0f != temp);
		}
		break;
	case STREAM_DATA_DOUBLE:
	case STREAM_DATA_DOUBLE_NULL:
		{
			double temp = 0.0;
			*this >> temp;
			output = (0.0 != temp);
		}
		break;
	case STREAM_DATA_C_STRING:
	case STREAM_DATA_C_STRING_NULL:
	case STREAM_DATA_STD_STRING:
	case STREAM_DATA_STD_STRING_NULL:
		{
			uint16_t len = 0;
			*this >> len;
			if(len > 0) {
				char szBuf[TS_STRING_MAX_SIZE];
				ReadBytes(szBuf, len);
				szBuf[len] = '\0';
				CSeparatedStream separated(szBuf, len,
					false, TS_STRING_DELIM, TS_STRING_DELIM);
				separated >> output;
			}
		}
		break;
	default:
		assert(false);
		break;
	};
}

void CTransferStream::ReadByType(uint8_t& output, uint8_t nType)
{
	assert(IsSingleType(nType));

	switch(nType) {
	case STREAM_DATA_CHAR:
	case STREAM_DATA_CHAR_NULL:
		{
			char temp = 0;
			*this >> temp;
			output = (uint8_t)temp;
		}
		break;
	case STREAM_DATA_BOOL:
	case STREAM_DATA_BOOL_NULL:
		{
			bool temp = false;
			*this >> temp;
			output = (uint8_t)temp;
		}
		break;
	case STREAM_DATA_INT8:
	case STREAM_DATA_INT8_NULL:
		{
			int8_t temp = 0;
			*this >> temp;
			output = (uint8_t)temp;
		}
		break;
	case STREAM_DATA_UINT8:
	case STREAM_DATA_UINT8_NULL:
		{
			*this >> output;
		}
		break;
	case STREAM_DATA_INT16:
	case STREAM_DATA_INT16_NULL:
		{
			int16_t temp = 0;
			*this >> temp;
			output = (uint8_t)temp;
		}
		break;
	case STREAM_DATA_UINT16:
	case STREAM_DATA_UINT16_NULL:
		{
			uint16_t temp = 0;
			*this >> temp;
			output = (uint8_t)temp;
		}
		break;
	case STREAM_DATA_INT32:
	case STREAM_DATA_INT32_NULL:
		{
			int32_t temp = 0;
			*this >> temp;
			output = (uint8_t)temp;
		}
		break;
	case STREAM_DATA_UINT32:
	case STREAM_DATA_UINT32_NULL:
		{
			uint32_t temp = 0;
			*this >> temp;
			output = (uint8_t)temp;
		}
		break;
	case STREAM_DATA_INT64:
	case STREAM_DATA_INT64_NULL:
		{
			int64_t temp = 0;
			*this >> temp;
			output = (uint8_t)temp;
		}
		break;
	case STREAM_DATA_UINT64:
	case STREAM_DATA_UINT64_NULL:
		{
			uint64_t temp = 0;
			*this >> temp;
			output = (uint8_t)temp;
		}
		break;
	case STREAM_DATA_FLOAT:
	case STREAM_DATA_FLOAT_NULL:
		{
			float temp = 0.0f;
			*this >> temp;
			output = (uint8_t)temp;
		}
		break;
	case STREAM_DATA_DOUBLE:
	case STREAM_DATA_DOUBLE_NULL:
		{
			double temp = 0.0;
			*this >> temp;
			output = (uint8_t)temp;
		}
		break;
	case STREAM_DATA_C_STRING:
	case STREAM_DATA_C_STRING_NULL:
	case STREAM_DATA_STD_STRING:
	case STREAM_DATA_STD_STRING_NULL:
		{
			uint16_t len = 0;
			*this >> len;
			if(len > 0) {
				char szBuf[TS_STRING_MAX_SIZE];
				ReadBytes(szBuf, len);
				szBuf[len] = '\0';
				CSeparatedStream separated(szBuf, len,
					false, TS_STRING_DELIM, TS_STRING_DELIM);
				separated >> output;
			}
		}
		break;
	default:
		assert(false);
		break;
	};
}

void CTransferStream::ReadByType(int8_t& output, uint8_t nType)
{
	assert(IsSingleType(nType));

	switch(nType) {
	case STREAM_DATA_CHAR:
	case STREAM_DATA_CHAR_NULL:
		{
			char temp = 0;
			*this >> temp;
			output = (int8_t)temp;
		}
		break;
	case STREAM_DATA_BOOL:
	case STREAM_DATA_BOOL_NULL:
		{
			bool temp = false;
			*this >> temp;
			output = (int8_t)temp;
		}
		break;
	case STREAM_DATA_INT8:
	case STREAM_DATA_INT8_NULL:
		{
			*this >> output;
		}
		break;
	case STREAM_DATA_UINT8:
	case STREAM_DATA_UINT8_NULL:
		{
			uint8_t temp = 0;
			*this >> temp;
			output = (int8_t)temp;
		}
		break;
	case STREAM_DATA_INT16:
	case STREAM_DATA_INT16_NULL:
		{
			int16_t temp = 0;
			*this >> temp;
			output = (int8_t)temp;
		}
		break;
	case STREAM_DATA_UINT16:
	case STREAM_DATA_UINT16_NULL:
		{
			uint16_t temp = 0;
			*this >> temp;
			output = (int8_t)temp;
		}
		break;
	case STREAM_DATA_INT32:
	case STREAM_DATA_INT32_NULL:
		{
			int32_t temp = 0;
			*this >> temp;
			output = (int8_t)temp;
		}
		break;
	case STREAM_DATA_UINT32:
	case STREAM_DATA_UINT32_NULL:
		{
			uint32_t temp = 0;
			*this >> temp;
			output = (int8_t)temp;
		}
		break;
	case STREAM_DATA_INT64:
	case STREAM_DATA_INT64_NULL:
		{
			int64_t temp = 0;
			*this >> temp;
			output = (int8_t)temp;
		}
		break;
	case STREAM_DATA_UINT64:
	case STREAM_DATA_UINT64_NULL:
		{
			uint64_t temp = 0;
			*this >> temp;
			output = (int8_t)temp;
		}
		break;
	case STREAM_DATA_FLOAT:
	case STREAM_DATA_FLOAT_NULL:
		{
			float temp = 0.0f;
			*this >> temp;
			output = (int8_t)temp;
		}
		break;
	case STREAM_DATA_DOUBLE:
	case STREAM_DATA_DOUBLE_NULL:
		{
			double temp = 0.0;
			*this >> temp;
			output = (int8_t)temp;
		}
		break;
	case STREAM_DATA_C_STRING:
	case STREAM_DATA_C_STRING_NULL:
	case STREAM_DATA_STD_STRING:
	case STREAM_DATA_STD_STRING_NULL:
		{
			uint16_t len = 0;
			*this >> len;
			if(len > 0) {
				char szBuf[TS_STRING_MAX_SIZE];
				ReadBytes(szBuf, len);
				szBuf[len] = '\0';
				CSeparatedStream separated(szBuf, len,
					false, TS_STRING_DELIM, TS_STRING_DELIM);
				separated >> output;
			}
		}
		break;
	default:
		assert(false);
		break;
	};
}

void CTransferStream::ReadByType(uint16_t& output, uint8_t nType)
{
	assert(IsSingleType(nType));

	switch(nType) {
	case STREAM_DATA_CHAR:
	case STREAM_DATA_CHAR_NULL:
		{
			char temp = 0;
			*this >> temp;
			output = (uint16_t)temp;
		}
		break;
	case STREAM_DATA_BOOL:
	case STREAM_DATA_BOOL_NULL:
		{
			bool temp = false;
			*this >> temp;
			output = (uint16_t)temp;
		}
		break;
	case STREAM_DATA_INT8:
	case STREAM_DATA_INT8_NULL:
		{
			int8_t temp = 0;
			*this >> temp;
			output = (uint16_t)temp;
		}
		break;
	case STREAM_DATA_UINT8:
	case STREAM_DATA_UINT8_NULL:
		{
			uint8_t temp = 0;
			*this >> temp;
			output = (uint16_t)temp;
		}
		break;
	case STREAM_DATA_INT16:
	case STREAM_DATA_INT16_NULL:
		{
			int16_t temp = 0;
			*this >> temp;
			output = (uint16_t)temp;
		}
		break;
	case STREAM_DATA_UINT16:
	case STREAM_DATA_UINT16_NULL:
		{
			*this >> output;
		}
		break;
	case STREAM_DATA_INT32:
	case STREAM_DATA_INT32_NULL:
		{
			int32_t temp = 0;
			*this >> temp;
			output = (uint16_t)temp;
		}
		break;
	case STREAM_DATA_UINT32:
	case STREAM_DATA_UINT32_NULL:
		{
			uint32_t temp = 0;
			*this >> temp;
			output = (uint16_t)temp;
		}
		break;
	case STREAM_DATA_INT64:
	case STREAM_DATA_INT64_NULL:
		{
			int64_t temp = 0;
			*this >> temp;
			output = (uint16_t)temp;
		}
		break;
	case STREAM_DATA_UINT64:
	case STREAM_DATA_UINT64_NULL:
		{
			uint64_t temp = 0;
			*this >> temp;
			output = (uint16_t)temp;
		}
		break;
	case STREAM_DATA_FLOAT:
	case STREAM_DATA_FLOAT_NULL:
		{
			float temp = 0.0f;
			*this >> temp;
			output = (uint16_t)temp;
		}
		break;
	case STREAM_DATA_DOUBLE:
	case STREAM_DATA_DOUBLE_NULL:
		{
			double temp = 0.0;
			*this >> temp;
			output = (uint16_t)temp;
		}
		break;
	case STREAM_DATA_C_STRING:
	case STREAM_DATA_C_STRING_NULL:
	case STREAM_DATA_STD_STRING:
	case STREAM_DATA_STD_STRING_NULL:
		{
			uint16_t len = 0;
			*this >> len;
			if(len > 0) {
				char szBuf[TS_STRING_MAX_SIZE];
				ReadBytes(szBuf, len);
				szBuf[len] = '\0';
				CSeparatedStream separated(szBuf, len,
					false, TS_STRING_DELIM, TS_STRING_DELIM);
				separated >> output;
			}
		}
		break;
	default:
		assert(false);
		break;
	};
}

void CTransferStream::ReadByType(int16_t& output, uint8_t nType)
{
	assert(IsSingleType(nType));

	switch(nType) {
	case STREAM_DATA_CHAR:
	case STREAM_DATA_CHAR_NULL:
		{
			char temp = 0;
			*this >> temp;
			output = (int16_t)temp;
		}
		break;
	case STREAM_DATA_BOOL:
	case STREAM_DATA_BOOL_NULL:
		{
			bool temp = false;
			*this >> temp;
			output = (int16_t)temp;
		}
		break;
	case STREAM_DATA_INT8:
	case STREAM_DATA_INT8_NULL:
		{
			int8_t temp = 0;
			*this >> temp;
			output = (int16_t)temp;
		}
		break;
	case STREAM_DATA_UINT8:
	case STREAM_DATA_UINT8_NULL:
		{
			uint8_t temp = 0;
			*this >> temp;
			output = (int16_t)temp;
		}
		break;
	case STREAM_DATA_INT16:
	case STREAM_DATA_INT16_NULL:
		{
			*this >> output;
		}
		break;
	case STREAM_DATA_UINT16:
	case STREAM_DATA_UINT16_NULL:
		{
			uint16_t temp = 0;
			*this >> temp;
			output = (int16_t)temp;
		}
		break;
	case STREAM_DATA_INT32:
	case STREAM_DATA_INT32_NULL:
		{
			int32_t temp = 0;
			*this >> temp;
			output = (int16_t)temp;
		}
		break;
	case STREAM_DATA_UINT32:
	case STREAM_DATA_UINT32_NULL:
		{
			uint32_t temp = 0;
			*this >> temp;
			output = (int16_t)temp;
		}
		break;
	case STREAM_DATA_INT64:
	case STREAM_DATA_INT64_NULL:
		{
			int64_t temp = 0;
			*this >> temp;
			output = (int16_t)temp;
		}
		break;
	case STREAM_DATA_UINT64:
	case STREAM_DATA_UINT64_NULL:
		{
			uint64_t temp = 0;
			*this >> temp;
			output = (int16_t)temp;
		}
		break;
	case STREAM_DATA_FLOAT:
	case STREAM_DATA_FLOAT_NULL:
		{
			float temp = 0.0f;
			*this >> temp;
			output = (int16_t)temp;
		}
		break;
	case STREAM_DATA_DOUBLE:
	case STREAM_DATA_DOUBLE_NULL:
		{
			double temp = 0.0;
			*this >> temp;
			output = (int16_t)temp;
		}
		break;
	case STREAM_DATA_C_STRING:
	case STREAM_DATA_C_STRING_NULL:
	case STREAM_DATA_STD_STRING:
	case STREAM_DATA_STD_STRING_NULL:
		{
			uint16_t len = 0;
			*this >> len;
			if(len > 0) {
				char szBuf[TS_STRING_MAX_SIZE];
				ReadBytes(szBuf, len);
				szBuf[len] = '\0';
				CSeparatedStream separated(szBuf, len,
					false, TS_STRING_DELIM, TS_STRING_DELIM);
				separated >> output;
			}
		}
		break;
	default:
		assert(false);
		break;
	};
}

void CTransferStream::ReadByType(uint32_t& output, uint8_t nType)
{
	assert(IsSingleType(nType));

	switch(nType) {
	case STREAM_DATA_CHAR:
	case STREAM_DATA_CHAR_NULL:
		{
			char temp = 0;
			*this >> temp;
			output = (uint32_t)temp;
		}
		break;
	case STREAM_DATA_BOOL:
	case STREAM_DATA_BOOL_NULL:
		{
			bool temp = false;
			*this >> temp;
			output = (uint32_t)temp;
		}
		break;
	case STREAM_DATA_INT8:
	case STREAM_DATA_INT8_NULL:
		{
			int8_t temp = 0;
			*this >> temp;
			output = (uint32_t)temp;
		}
		break;
	case STREAM_DATA_UINT8:
	case STREAM_DATA_UINT8_NULL:
		{
			uint8_t temp = 0;
			*this >> temp;
			output = (uint32_t)temp;
		}
		break;
	case STREAM_DATA_INT16:
	case STREAM_DATA_INT16_NULL:
		{
			int16_t temp = 0;
			*this >> temp;
			output = (uint32_t)temp;
		}
		break;
	case STREAM_DATA_UINT16:
	case STREAM_DATA_UINT16_NULL:
		{
			uint16_t temp = 0;
			*this >> temp;
			output = (uint32_t)temp;
		}
		break;
	case STREAM_DATA_INT32:
	case STREAM_DATA_INT32_NULL:
		{
			int32_t temp = 0;
			*this >> temp;
			output = (uint32_t)temp;
		}
		break;
	case STREAM_DATA_UINT32:
	case STREAM_DATA_UINT32_NULL:
		{
			*this >> output;
		}
		break;
	case STREAM_DATA_INT64:
	case STREAM_DATA_INT64_NULL:
		{
			int64_t temp = 0;
			*this >> temp;
			output = (uint32_t)temp;
		}
		break;
	case STREAM_DATA_UINT64:
	case STREAM_DATA_UINT64_NULL:
		{
			uint64_t temp = 0;
			*this >> temp;
			output = (uint32_t)temp;
		}
		break;
	case STREAM_DATA_FLOAT:
	case STREAM_DATA_FLOAT_NULL:
		{
			float temp = 0.0f;
			*this >> temp;
			output = (uint32_t)temp;
		}
		break;
	case STREAM_DATA_DOUBLE:
	case STREAM_DATA_DOUBLE_NULL:
		{
			double temp = 0.0;
			*this >> temp;
			output = (uint32_t)temp;
		}
		break;
	case STREAM_DATA_C_STRING:
	case STREAM_DATA_C_STRING_NULL:
	case STREAM_DATA_STD_STRING:
	case STREAM_DATA_STD_STRING_NULL:
		{
			uint16_t len = 0;
			*this >> len;
			if(len > 0) {
				char szBuf[TS_STRING_MAX_SIZE];
				ReadBytes(szBuf, len);
				szBuf[len] = '\0';
				CSeparatedStream separated(szBuf, len,
					false, TS_STRING_DELIM, TS_STRING_DELIM);
				separated >> output;
			}
		}
		break;
	default:
		assert(false);
		break;
	};
}

void CTransferStream::ReadByType(int32_t& output, uint8_t nType)
{
	assert(IsSingleType(nType));

	switch(nType) {
	case STREAM_DATA_CHAR:
	case STREAM_DATA_CHAR_NULL:
		{
			char temp = 0;
			*this >> temp;
			output = (int32_t)temp;
		}
		break;
	case STREAM_DATA_BOOL:
	case STREAM_DATA_BOOL_NULL:
		{
			bool temp = false;
			*this >> temp;
			output = (int32_t)temp;
		}
		break;
	case STREAM_DATA_INT8:
	case STREAM_DATA_INT8_NULL:
		{
			int8_t temp = 0;
			*this >> temp;
			output = (int32_t)temp;
		}
		break;
	case STREAM_DATA_UINT8:
	case STREAM_DATA_UINT8_NULL:
		{
			uint8_t temp = 0;
			*this >> temp;
			output = (int32_t)temp;
		}
		break;
	case STREAM_DATA_INT16:
	case STREAM_DATA_INT16_NULL:
		{
			int16_t temp = 0;
			*this >> temp;
			output = (int32_t)temp;
		}
		break;
	case STREAM_DATA_UINT16:
	case STREAM_DATA_UINT16_NULL:
		{
			uint16_t temp = 0;
			*this >> temp;
			output = (int32_t)temp;
		}
		break;
	case STREAM_DATA_INT32:
	case STREAM_DATA_INT32_NULL:
		{
			*this >> output;
		}
		break;
	case STREAM_DATA_UINT32:
	case STREAM_DATA_UINT32_NULL:
		{
			uint32_t temp = 0;
			*this >> temp;
			output = (int32_t)temp;
		}
		break;
	case STREAM_DATA_INT64:
	case STREAM_DATA_INT64_NULL:
		{
			int64_t temp = 0;
			*this >> temp;
			output = (int32_t)temp;
		}
		break;
	case STREAM_DATA_UINT64:
	case STREAM_DATA_UINT64_NULL:
		{
			uint64_t temp = 0;
			*this >> temp;
			output = (int32_t)temp;
		}
		break;
	case STREAM_DATA_FLOAT:
	case STREAM_DATA_FLOAT_NULL:
		{
			float temp = 0.0f;
			*this >> temp;
			output = (int32_t)temp;
		}
		break;
	case STREAM_DATA_DOUBLE:
	case STREAM_DATA_DOUBLE_NULL:
		{
			double temp = 0.0;
			*this >> temp;
			output = (int32_t)temp;
		}
		break;
	case STREAM_DATA_C_STRING:
	case STREAM_DATA_C_STRING_NULL:
	case STREAM_DATA_STD_STRING:
	case STREAM_DATA_STD_STRING_NULL:
		{
			uint16_t len = 0;
			*this >> len;
			if(len > 0) {
				char szBuf[TS_STRING_MAX_SIZE];
				ReadBytes(szBuf, len);
				szBuf[len] = '\0';
				CSeparatedStream separated(szBuf, len,
					false, TS_STRING_DELIM, TS_STRING_DELIM);
				separated >> output;
			}
		}
		break;
	default:
		assert(false);
		break;
	};
}

#ifndef NO_INT64

void CTransferStream::ReadByType(uint64_t& output, uint8_t nType)
{
	assert(IsSingleType(nType));

	switch(nType) {
	case STREAM_DATA_CHAR:
	case STREAM_DATA_CHAR_NULL:
		{
			char temp = 0;
			*this >> temp;
			output = (uint64_t)temp;
		}
		break;
	case STREAM_DATA_BOOL:
	case STREAM_DATA_BOOL_NULL:
		{
			bool temp = false;
			*this >> temp;
			output = (uint64_t)temp;
		}
		break;
	case STREAM_DATA_INT8:
	case STREAM_DATA_INT8_NULL:
		{
			int8_t temp = 0;
			*this >> temp;
			output = (uint64_t)temp;
		}
		break;
	case STREAM_DATA_UINT8:
	case STREAM_DATA_UINT8_NULL:
		{
			uint8_t temp = 0;
			*this >> temp;
			output = (uint64_t)temp;
		}
		break;
	case STREAM_DATA_INT16:
	case STREAM_DATA_INT16_NULL:
		{
			int16_t temp = 0;
			*this >> temp;
			output = (uint64_t)temp;
		}
		break;
	case STREAM_DATA_UINT16:
	case STREAM_DATA_UINT16_NULL:
		{
			uint16_t temp = 0;
			*this >> temp;
			output = (uint64_t)temp;
		}
		break;
	case STREAM_DATA_INT32:
	case STREAM_DATA_INT32_NULL:
		{
			int32_t temp = 0;
			*this >> temp;
			output = (uint64_t)temp;
		}
		break;
	case STREAM_DATA_UINT32:
	case STREAM_DATA_UINT32_NULL:
		{
			uint32_t temp = 0;
			*this >> temp;
			output = (uint64_t)temp;
		}
		break;
	case STREAM_DATA_INT64:
	case STREAM_DATA_INT64_NULL:
		{
			int64_t temp = 0;
			*this >> temp;
			output = (uint64_t)temp;
		}
		break;
	case STREAM_DATA_UINT64:
	case STREAM_DATA_UINT64_NULL:
		{
			*this >> output;
		}
		break;
	case STREAM_DATA_FLOAT:
	case STREAM_DATA_FLOAT_NULL:
		{
			float temp = 0.0f;
			*this >> temp;
			output = (uint64_t)temp;
		}
		break;
	case STREAM_DATA_DOUBLE:
	case STREAM_DATA_DOUBLE_NULL:
		{
			double temp = 0.0;
			*this >> temp;
			output = (uint64_t)temp;
		}
		break;
	case STREAM_DATA_C_STRING:
	case STREAM_DATA_C_STRING_NULL:
	case STREAM_DATA_STD_STRING:
	case STREAM_DATA_STD_STRING_NULL:
		{
			uint16_t len = 0;
			*this >> len;
			if(len > 0) {
				char szBuf[TS_STRING_MAX_SIZE];
				ReadBytes(szBuf, len);
				szBuf[len] = '\0';
				CSeparatedStream separated(szBuf, len,
					false, TS_STRING_DELIM, TS_STRING_DELIM);
				separated >> output;
			}
		}
		break;
	default:
		assert(false);
		break;
	};
}

void CTransferStream::ReadByType(int64_t& output, uint8_t nType)
{
	assert(IsSingleType(nType));

	switch(nType) {
	case STREAM_DATA_CHAR:
	case STREAM_DATA_CHAR_NULL:
		{
			char temp = 0;
			*this >> temp;
			output = (int64_t)temp;
		}
		break;
	case STREAM_DATA_BOOL:
	case STREAM_DATA_BOOL_NULL:
		{
			bool temp = false;
			*this >> temp;
			output = (int64_t)temp;
		}
		break;
	case STREAM_DATA_INT8:
	case STREAM_DATA_INT8_NULL:
		{
			int8_t temp = 0;
			*this >> temp;
			output = (int64_t)temp;
		}
		break;
	case STREAM_DATA_UINT8:
	case STREAM_DATA_UINT8_NULL:
		{
			uint8_t temp = 0;
			*this >> temp;
			output = (int64_t)temp;
		}
		break;
	case STREAM_DATA_INT16:
	case STREAM_DATA_INT16_NULL:
		{
			int16_t temp = 0;
			*this >> temp;
			output = (int64_t)temp;
		}
		break;
	case STREAM_DATA_UINT16:
	case STREAM_DATA_UINT16_NULL:
		{
			uint16_t temp = 0;
			*this >> temp;
			output = (int64_t)temp;
		}
		break;
	case STREAM_DATA_INT32:
	case STREAM_DATA_INT32_NULL:
		{
			int32_t temp = 0;
			*this >> temp;
			output = (int64_t)temp;
		}
		break;
	case STREAM_DATA_UINT32:
	case STREAM_DATA_UINT32_NULL:
		{
			uint32_t temp = 0;
			*this >> temp;
			output = (int64_t)temp;
		}
		break;
	case STREAM_DATA_INT64:
	case STREAM_DATA_INT64_NULL:
		{
			*this >> output;
		}
		break;
	case STREAM_DATA_UINT64:
	case STREAM_DATA_UINT64_NULL:
		{
			uint64_t temp = 0;
			*this >> temp;
			output = (int64_t)temp;
		}
		break;
	case STREAM_DATA_FLOAT:
	case STREAM_DATA_FLOAT_NULL:
		{
			float temp = 0.0f;
			*this >> temp;
			output = (int64_t)temp;
		}
		break;
	case STREAM_DATA_DOUBLE:
	case STREAM_DATA_DOUBLE_NULL:
		{
			double temp = 0.0;
			*this >> temp;
			output = (int64_t)temp;
		}
		break;
	case STREAM_DATA_C_STRING:
	case STREAM_DATA_C_STRING_NULL:
	case STREAM_DATA_STD_STRING:
	case STREAM_DATA_STD_STRING_NULL:
		{
			uint16_t len = 0;
			*this >> len;
			if(len > 0) {
				char szBuf[TS_STRING_MAX_SIZE];
				ReadBytes(szBuf, len);
				szBuf[len] = '\0';
				CSeparatedStream separated(szBuf, len,
					false, TS_STRING_DELIM, TS_STRING_DELIM);
				separated >> output;
			}
		}
		break;
	default:
		assert(false);
		break;
	};
}
#endif

void CTransferStream::ReadByType(float& output, uint8_t nType)
{
	assert(IsSingleType(nType));

	switch(nType) {
	case STREAM_DATA_CHAR:
	case STREAM_DATA_CHAR_NULL:
		{
			char temp = 0;
			*this >> temp;
			output = (float)temp;
		}
		break;
	case STREAM_DATA_BOOL:
	case STREAM_DATA_BOOL_NULL:
		{
			bool temp = false;
			*this >> temp;
			output = (float)temp;
		}
		break;
	case STREAM_DATA_INT8:
	case STREAM_DATA_INT8_NULL:
		{
			int8_t temp = 0;
			*this >> temp;
			output = (float)temp;
		}
		break;
	case STREAM_DATA_UINT8:
	case STREAM_DATA_UINT8_NULL:
		{
			uint8_t temp = 0;
			*this >> temp;
			output = (float)temp;
		}
		break;
	case STREAM_DATA_INT16:
	case STREAM_DATA_INT16_NULL:
		{
			int16_t temp = 0;
			*this >> temp;
			output = (float)temp;
		}
		break;
	case STREAM_DATA_UINT16:
	case STREAM_DATA_UINT16_NULL:
		{
			uint16_t temp = 0;
			*this >> temp;
			output = (float)temp;
		}
		break;
	case STREAM_DATA_INT32:
	case STREAM_DATA_INT32_NULL:
		{
			int32_t temp = 0;
			*this >> temp;
			output = (float)temp;
		}
		break;
	case STREAM_DATA_UINT32:
	case STREAM_DATA_UINT32_NULL:
		{
			uint32_t temp = 0;
			*this >> temp;
			output = (float)temp;
		}
		break;
	case STREAM_DATA_INT64:
	case STREAM_DATA_INT64_NULL:
		{
			int64_t temp = 0;
			*this >> temp;
			output = (float)temp;
		}
		break;
	case STREAM_DATA_UINT64:
	case STREAM_DATA_UINT64_NULL:
		{
			uint64_t temp = 0;
			*this >> temp;
			output = (float)temp;
		}
		break;
	case STREAM_DATA_FLOAT:
	case STREAM_DATA_FLOAT_NULL:
		{
			*this >> output;
		}
		break;
	case STREAM_DATA_DOUBLE:
	case STREAM_DATA_DOUBLE_NULL:
		{
			double temp = 0.0;
			*this >> temp;
			output = (float)temp;
		}
		break;
	case STREAM_DATA_C_STRING:
	case STREAM_DATA_C_STRING_NULL:
	case STREAM_DATA_STD_STRING:
	case STREAM_DATA_STD_STRING_NULL:
		{
			uint16_t len = 0;
			*this >> len;
			if(len > 0) {
				char szBuf[TS_STRING_MAX_SIZE];
				ReadBytes(szBuf, len);
				szBuf[len] = '\0';
				CSeparatedStream separated(szBuf, len,
					false, TS_STRING_DELIM, TS_STRING_DELIM);
				separated >> output;
			}
		}
		break;
	default:
		assert(false);
		break;
	};
}

void CTransferStream::ReadByType(double& output, uint8_t nType)
{
	assert(IsSingleType(nType));

	switch(nType) {
	case STREAM_DATA_CHAR:
	case STREAM_DATA_CHAR_NULL:
		{
			char temp = 0;
			*this >> temp;
			output = (double)temp;
		}
		break;
	case STREAM_DATA_BOOL:
	case STREAM_DATA_BOOL_NULL:
		{
			bool temp = false;
			*this >> temp;
			output = (double)temp;
		}
		break;
	case STREAM_DATA_INT8:
	case STREAM_DATA_INT8_NULL:
		{
			int8_t temp = 0;
			*this >> temp;
			output = (double)temp;
		}
		break;
	case STREAM_DATA_UINT8:
	case STREAM_DATA_UINT8_NULL:
		{
			uint8_t temp = 0;
			*this >> temp;
			output = (double)temp;
		}
		break;
	case STREAM_DATA_INT16:
	case STREAM_DATA_INT16_NULL:
		{
			int16_t temp = 0;
			*this >> temp;
			output = (double)temp;
		}
		break;
	case STREAM_DATA_UINT16:
	case STREAM_DATA_UINT16_NULL:
		{
			uint16_t temp = 0;
			*this >> temp;
			output = (double)temp;
		}
		break;
	case STREAM_DATA_INT32:
	case STREAM_DATA_INT32_NULL:
		{
			int32_t temp = 0;
			*this >> temp;
			output = (double)temp;
		}
		break;
	case STREAM_DATA_UINT32:
	case STREAM_DATA_UINT32_NULL:
		{
			uint32_t temp = 0;
			*this >> temp;
			output = (double)temp;
		}
		break;
	case STREAM_DATA_INT64:
	case STREAM_DATA_INT64_NULL:
		{
			int64_t temp = 0;
			*this >> temp;
			output = (double)temp;
		}
		break;
	case STREAM_DATA_UINT64:
	case STREAM_DATA_UINT64_NULL:
		{
			uint64_t temp = 0;
			*this >> temp;
			output = (double)temp;
		}
		break;
	case STREAM_DATA_FLOAT:
	case STREAM_DATA_FLOAT_NULL:
		{
			float temp = 0.0f;
			*this >> temp;
			output = (double)temp;
		}
		break;
	case STREAM_DATA_DOUBLE:
	case STREAM_DATA_DOUBLE_NULL:
		{
			*this >> output;
		}
		break;
	case STREAM_DATA_C_STRING:
	case STREAM_DATA_C_STRING_NULL:
	case STREAM_DATA_STD_STRING:
	case STREAM_DATA_STD_STRING_NULL:
		{
			uint16_t len = 0;
			*this >> len;
			if(len > 0) {
				char szBuf[TS_STRING_MAX_SIZE];
				ReadBytes(szBuf, len);
				szBuf[len] = '\0';
				CSeparatedStream separated(szBuf, len,
					false, TS_STRING_DELIM, TS_STRING_DELIM);
				separated >> output;
			}
		}
		break;
	default:
		assert(false);
		break;
	};
}

//////////////////////////////////////////////////////////////////////////
void util::CTransferStream::ReadCharToString(std::string& output)
{
	char temp = 0;
	*this >> temp;

	output = temp;
}

void util::CTransferStream::ReadBoolToString(std::string& output)
{
	bool temp = false;
	*this >> temp;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated << temp;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadUInt8ToString(std::string& output)
{
	uint8_t temp = 0;
	*this >> temp;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated << temp;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadUInt16ToString(std::string& output)
{
	uint16_t temp = 0;
	*this >> temp;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated << temp;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadUInt32ToString(std::string& output)
{
	uint32_t temp = 0;
	*this >> temp;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated << temp;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadUInt64ToString(std::string& output)
{
	uint64_t temp = 0;
	*this >> temp;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated << temp;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadInt8ToString(std::string& output)
{
	int8_t temp = 0;
	*this >> temp;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated << temp;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadInt16ToString(std::string& output)
{
	int16_t temp = 0;
	*this >> temp;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated << temp;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadInt32ToString(std::string& output)
{
	int32_t temp = 0;
	*this >> temp;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated << temp;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadInt64ToString(std::string& output)
{
	int64_t temp = 0;
	*this >> temp;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated << temp;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadFloatToString(std::string& output)
{
	float temp = 0.0f;
	*this >> temp;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated << temp;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadDoubleToString(std::string& output)
{
	double temp = 0.0;
	*this >> temp;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated << temp;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadStringToString(std::string& output)
{
	uint16_t len = 0;
	*this >> len;
	if(len < 1) {
		return;
	}

	char szBuf[TS_STRING_MAX_SIZE];
	ReadBytes(szBuf, len);
	szBuf[len] = '\0';
	output = szBuf;
}

void util::CTransferStream::ReadBoolSetToString(std::string& output)
{
	std::vector<bool> object;
	*this >> object;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadUInt8SetToString(std::string& output)
{
	std::vector<uint8_t> object;
	*this >> object;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadInt8SetToString(std::string& output)
{
	std::vector<int8_t> object;
	*this >> object;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadUInt16SetToString(std::string& output)
{
	std::vector<uint16_t> object;
	*this >> object;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadInt16SetToString(std::string& output)
{
	std::vector<int16_t> object;
	*this >> object;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadUInt32SetToString(std::string& output)
{
	std::vector<uint32_t> object;
	*this >> object;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadInt32SetToString(std::string& output)
{
	std::vector<int32_t> object;
	*this >> object;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadUInt64SetToString(std::string& output)
{
	std::vector<uint64_t> object;
	*this >> object;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadInt64SetToString(std::string& output)
{
	std::vector<int64_t> object;
	*this >> object;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadFloatSetToString(std::string& output)
{
	std::vector<float> object;
	*this >> object;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadDoubleSetToString(std::string& output)
{
	std::vector<double> object;
	*this >> object;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

void util::CTransferStream::ReadStringSetToString(std::string& output)
{
	std::vector<std::string> object;
	*this >> object;

	CSeparatedStream separated(
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated << object;
	separated.EndLine();

	output = separated.Str();
}

//////////////////////////////////////////////////////////////////////////
void util::CTransferStream::WriteCharFromString(const char* input, int length)
{
	char temp = 0;

	if(NULL != input && length > 0) {
		temp = input[0];
	}

	*this << temp;
}

void util::CTransferStream::WriteBoolFromString(const char* input, int length)
{
	bool temp = false;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated >> temp;

	*this << temp;
}

void util::CTransferStream::WriteUInt8FromString(const char* input, int length)
{
	uint8_t temp = 0;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated >> temp;

	*this << temp;
}

void util::CTransferStream::WriteUInt16FromString(const char* input, int length)
{
	uint16_t temp = 0;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated >> temp;

	*this << temp;
}

void util::CTransferStream::WriteUInt32FromString(const char* input, int length)
{
	uint32_t temp = 0;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated >> temp;

	*this << temp;
}

void util::CTransferStream::WriteUInt64FromString(const char* input, int length)
{
	uint64_t temp = 0;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated >> temp;

	*this << temp;
}

void util::CTransferStream::WriteInt8FromString(const char* input, int length)
{
	int8_t temp = 0;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated >> temp;

	*this << temp;
}

void util::CTransferStream::WriteInt16FromString(const char* input, int length)
{
	int16_t temp = 0;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated >> temp;

	*this << temp;
}

void util::CTransferStream::WriteInt32FromString(const char* input, int length)
{
	int32_t temp = 0;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated >> temp;

	*this << temp;
}

void util::CTransferStream::WriteInt64FromString(const char* input, int length)
{
	int64_t temp = 0;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated >> temp;

	*this << temp;
}

void util::CTransferStream::WriteFloatFromString(const char* input, int length)
{
	float temp = 0.0f;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated >> temp;

	*this << temp;
}

void util::CTransferStream::WriteDoubleFromString(const char* input, int length)
{
	double temp = 0.0;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);
	separated >> temp;

	*this << temp;
}

void util::CTransferStream::WriteStringFromString(const char* input, int length)
{
	uint16_t len = (uint16_t)length;
	*this << len;

	if(len > 0){
		WriteBytes(input, len);
	}
}

void util::CTransferStream::WriteBoolSetFromString(const char* input, int length)
{
	std::vector<bool> object;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated >> object;

	*this << object;
}

void util::CTransferStream::WriteUInt8SetFromString(const char* input, int length)
{
	std::vector<uint8_t> object;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated >> object;

	*this << object;
}

void util::CTransferStream::WriteInt8SetFromString(const char* input, int length)
{
	std::vector<int8_t> object;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated >> object;

	*this << object;
}

void util::CTransferStream::WriteUInt16SetFromString(const char* input, int length)
{
	std::vector<uint16_t> object;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated >> object;

	*this << object;
}

void util::CTransferStream::WriteInt16SetFromString(const char* input, int length)
{
	std::vector<int16_t> object;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated >> object;

	*this << object;
}

void util::CTransferStream::WriteUInt32SetFromString(const char* input, int length)
{
	std::vector<uint32_t> object;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated >> object;

	*this << object;
}

void util::CTransferStream::WriteInt32SetFromString(const char* input, int length)
{
	std::vector<int32_t> object;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated >> object;

	*this << object;
}

void util::CTransferStream::WriteUInt64SetFromString(const char* input, int length)
{
	std::vector<uint64_t> object;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated >> object;

	*this << object;
}

void util::CTransferStream::WriteInt64SetFromString(const char* input, int length)
{
	std::vector<int64_t> object;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated >> object;

	*this << object;
}

void util::CTransferStream::WriteFloatSetFromString(const char* input, int length)
{
	std::vector<float> object;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated >> object;

	*this << object;
}

void util::CTransferStream::WriteDoubleSetFromString(const char* input, int length)
{
	std::vector<double> object;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated >> object;

	*this << object;
}

void util::CTransferStream::WriteStringSetFromString(const char* input, int length)
{
	std::vector<std::string> object;

	CSeparatedStream separated(
		input, length, false,
		TS_STRING_DELIM,
		TS_STRING_DELIM);

	separated >> object;

	*this << object;
}
//////////////////////////////////////////////////////////////////////////

void util::CTransferStream::InitTypeOperator()
{
	if(s_bInitTypeOperator) {
		return;
	}
	s_bInitTypeOperator = true;

	s_typeOperators[STREAM_DATA_NIL].m_pReadToString = NULL;
	s_typeOperators[STREAM_DATA_NIL].m_pWriteFromString = NULL;

	s_typeOperators[STREAM_DATA_CHAR].m_pReadToString = &util::CTransferStream::ReadCharToString;
	s_typeOperators[STREAM_DATA_CHAR].m_pWriteFromString = &util::CTransferStream::WriteCharFromString;

	s_typeOperators[STREAM_DATA_CHAR_NULL].m_pReadToString = &util::CTransferStream::ReadCharToString;
	s_typeOperators[STREAM_DATA_CHAR_NULL].m_pWriteFromString = &util::CTransferStream::WriteCharFromString;

	s_typeOperators[STREAM_DATA_BOOL].m_pReadToString = &util::CTransferStream::ReadBoolToString;
	s_typeOperators[STREAM_DATA_BOOL].m_pWriteFromString = &util::CTransferStream::WriteBoolFromString;

	s_typeOperators[STREAM_DATA_BOOL_NULL].m_pReadToString = &util::CTransferStream::ReadBoolToString;
	s_typeOperators[STREAM_DATA_BOOL_NULL].m_pWriteFromString = &util::CTransferStream::WriteBoolFromString;

	s_typeOperators[STREAM_DATA_UINT8].m_pReadToString = &util::CTransferStream::ReadUInt8ToString;
	s_typeOperators[STREAM_DATA_UINT8].m_pWriteFromString = &util::CTransferStream::WriteUInt8FromString;

	s_typeOperators[STREAM_DATA_UINT8_NULL].m_pReadToString = &util::CTransferStream::ReadUInt8ToString;
	s_typeOperators[STREAM_DATA_UINT8_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt8FromString;

	s_typeOperators[STREAM_DATA_UINT16].m_pReadToString = &util::CTransferStream::ReadUInt16ToString;
	s_typeOperators[STREAM_DATA_UINT16].m_pWriteFromString = &util::CTransferStream::WriteUInt16FromString;

	s_typeOperators[STREAM_DATA_UINT16_NULL].m_pReadToString = &util::CTransferStream::ReadUInt16ToString;
	s_typeOperators[STREAM_DATA_UINT16_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt16FromString;

	s_typeOperators[STREAM_DATA_UINT32].m_pReadToString = &util::CTransferStream::ReadUInt32ToString;
	s_typeOperators[STREAM_DATA_UINT32].m_pWriteFromString = &util::CTransferStream::WriteUInt32FromString;

	s_typeOperators[STREAM_DATA_UINT32_NULL].m_pReadToString = &util::CTransferStream::ReadUInt32ToString;
	s_typeOperators[STREAM_DATA_UINT32_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt32FromString;

	s_typeOperators[STREAM_DATA_UINT64].m_pReadToString = &util::CTransferStream::ReadUInt64ToString;
	s_typeOperators[STREAM_DATA_UINT64].m_pWriteFromString = &util::CTransferStream::WriteUInt64FromString;

	s_typeOperators[STREAM_DATA_UINT64_NULL].m_pReadToString = &util::CTransferStream::ReadUInt64ToString;
	s_typeOperators[STREAM_DATA_UINT64_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt64FromString;

	s_typeOperators[STREAM_DATA_INT8].m_pReadToString = &util::CTransferStream::ReadInt8ToString;
	s_typeOperators[STREAM_DATA_INT8].m_pWriteFromString = &util::CTransferStream::WriteInt8FromString;

	s_typeOperators[STREAM_DATA_INT8_NULL].m_pReadToString = &util::CTransferStream::ReadInt8ToString;
	s_typeOperators[STREAM_DATA_INT8_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt8FromString;

	s_typeOperators[STREAM_DATA_INT16].m_pReadToString = &util::CTransferStream::ReadInt16ToString;
	s_typeOperators[STREAM_DATA_INT16].m_pWriteFromString = &util::CTransferStream::WriteInt16FromString;

	s_typeOperators[STREAM_DATA_INT16_NULL].m_pReadToString = &util::CTransferStream::ReadInt16ToString;
	s_typeOperators[STREAM_DATA_INT16_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt16FromString;

	s_typeOperators[STREAM_DATA_INT32].m_pReadToString = &util::CTransferStream::ReadInt32ToString;
	s_typeOperators[STREAM_DATA_INT32].m_pWriteFromString = &util::CTransferStream::WriteInt32FromString;

	s_typeOperators[STREAM_DATA_INT32_NULL].m_pReadToString = &util::CTransferStream::ReadInt32ToString;
	s_typeOperators[STREAM_DATA_INT32_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt32FromString;

	s_typeOperators[STREAM_DATA_INT64].m_pReadToString = &util::CTransferStream::ReadInt64ToString;
	s_typeOperators[STREAM_DATA_INT64].m_pWriteFromString = &util::CTransferStream::WriteInt64FromString;

	s_typeOperators[STREAM_DATA_INT64_NULL].m_pReadToString = &util::CTransferStream::ReadInt64ToString;
	s_typeOperators[STREAM_DATA_INT64_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt64FromString;

	s_typeOperators[STREAM_DATA_FLOAT].m_pReadToString = &util::CTransferStream::ReadFloatToString;
	s_typeOperators[STREAM_DATA_FLOAT].m_pWriteFromString = &util::CTransferStream::WriteFloatFromString;

	s_typeOperators[STREAM_DATA_FLOAT_NULL].m_pReadToString = &util::CTransferStream::ReadFloatToString;
	s_typeOperators[STREAM_DATA_FLOAT_NULL].m_pWriteFromString = &util::CTransferStream::WriteFloatFromString;

	s_typeOperators[STREAM_DATA_DOUBLE].m_pReadToString = &util::CTransferStream::ReadDoubleToString;
	s_typeOperators[STREAM_DATA_DOUBLE].m_pWriteFromString = &util::CTransferStream::WriteDoubleFromString;

	s_typeOperators[STREAM_DATA_DOUBLE_NULL].m_pReadToString = &util::CTransferStream::ReadDoubleToString;
	s_typeOperators[STREAM_DATA_DOUBLE_NULL].m_pWriteFromString = &util::CTransferStream::WriteDoubleFromString;

	s_typeOperators[STREAM_DATA_STD_STRING].m_pReadToString = &util::CTransferStream::ReadStringToString;
	s_typeOperators[STREAM_DATA_STD_STRING].m_pWriteFromString = &util::CTransferStream::WriteStringFromString;

	s_typeOperators[STREAM_DATA_STD_STRING_NULL].m_pReadToString = &util::CTransferStream::ReadStringToString;
	s_typeOperators[STREAM_DATA_STD_STRING_NULL].m_pWriteFromString = &util::CTransferStream::WriteStringFromString;

	s_typeOperators[STREAM_DATA_C_STRING].m_pReadToString = &util::CTransferStream::ReadStringToString;
	s_typeOperators[STREAM_DATA_C_STRING].m_pWriteFromString = &util::CTransferStream::WriteStringFromString;

	s_typeOperators[STREAM_DATA_C_STRING_NULL].m_pReadToString = &util::CTransferStream::ReadStringToString;
	s_typeOperators[STREAM_DATA_C_STRING_NULL].m_pWriteFromString = &util::CTransferStream::WriteStringFromString;
	////////////////////////////
	s_typeOperators[STREAM_DATA_VECTOR_BOOL].m_pReadToString = &util::CTransferStream::ReadBoolSetToString;
	s_typeOperators[STREAM_DATA_VECTOR_BOOL].m_pWriteFromString = &util::CTransferStream::WriteBoolSetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_BOOL_NULL].m_pReadToString = &util::CTransferStream::ReadBoolSetToString;
	s_typeOperators[STREAM_DATA_VECTOR_BOOL_NULL].m_pWriteFromString = &util::CTransferStream::WriteBoolSetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_UINT8].m_pReadToString = &util::CTransferStream::ReadUInt8SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_UINT8].m_pWriteFromString = &util::CTransferStream::WriteUInt8SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_UINT8_NULL].m_pReadToString = &util::CTransferStream::ReadUInt8SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_UINT8_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt8SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_UINT16].m_pReadToString = &util::CTransferStream::ReadUInt16SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_UINT16].m_pWriteFromString = &util::CTransferStream::WriteUInt16SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_UINT16_NULL].m_pReadToString = &util::CTransferStream::ReadUInt16SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_UINT16_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt16SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_UINT32].m_pReadToString = &util::CTransferStream::ReadUInt32SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_UINT32].m_pWriteFromString = &util::CTransferStream::WriteUInt32SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_UINT32_NULL].m_pReadToString = &util::CTransferStream::ReadUInt32SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_UINT32_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt32SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_UINT64].m_pReadToString = &util::CTransferStream::ReadUInt64SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_UINT64].m_pWriteFromString = &util::CTransferStream::WriteUInt64SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_UINT64_NULL].m_pReadToString = &util::CTransferStream::ReadUInt64SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_UINT64_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt64SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_INT8].m_pReadToString = &util::CTransferStream::ReadInt8SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_INT8].m_pWriteFromString = &util::CTransferStream::WriteInt8SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_INT8_NULL].m_pReadToString = &util::CTransferStream::ReadInt8SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_INT8_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt8SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_INT16].m_pReadToString = &util::CTransferStream::ReadInt16SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_INT16].m_pWriteFromString = &util::CTransferStream::WriteInt16SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_INT16_NULL].m_pReadToString = &util::CTransferStream::ReadInt16SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_INT16_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt16SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_INT32].m_pReadToString = &util::CTransferStream::ReadInt32SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_INT32].m_pWriteFromString = &util::CTransferStream::WriteInt32SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_INT32_NULL].m_pReadToString = &util::CTransferStream::ReadInt32SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_INT32_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt32SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_INT64].m_pReadToString = &util::CTransferStream::ReadInt64SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_INT64].m_pWriteFromString = &util::CTransferStream::WriteInt64SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_INT64_NULL].m_pReadToString = &util::CTransferStream::ReadInt64SetToString;
	s_typeOperators[STREAM_DATA_VECTOR_INT64_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt64SetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_FLOAT].m_pReadToString = &util::CTransferStream::ReadFloatSetToString;
	s_typeOperators[STREAM_DATA_VECTOR_FLOAT].m_pWriteFromString = &util::CTransferStream::WriteFloatSetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_FLOAT_NULL].m_pReadToString = &util::CTransferStream::ReadFloatSetToString;
	s_typeOperators[STREAM_DATA_VECTOR_FLOAT_NULL].m_pWriteFromString = &util::CTransferStream::WriteFloatSetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_DOUBLE].m_pReadToString = &util::CTransferStream::ReadDoubleSetToString;
	s_typeOperators[STREAM_DATA_VECTOR_DOUBLE].m_pWriteFromString = &util::CTransferStream::WriteDoubleSetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_DOUBLE_NULL].m_pReadToString = &util::CTransferStream::ReadDoubleSetToString;
	s_typeOperators[STREAM_DATA_VECTOR_DOUBLE_NULL].m_pWriteFromString = &util::CTransferStream::WriteDoubleSetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_STRING].m_pReadToString = &util::CTransferStream::ReadStringSetToString;
	s_typeOperators[STREAM_DATA_VECTOR_STRING].m_pWriteFromString = &util::CTransferStream::WriteStringSetFromString;

	s_typeOperators[STREAM_DATA_VECTOR_STRING_NULL].m_pReadToString = &util::CTransferStream::ReadStringSetToString;
	s_typeOperators[STREAM_DATA_VECTOR_STRING_NULL].m_pWriteFromString = &util::CTransferStream::WriteStringSetFromString;
	///////////////////////////////
	s_typeOperators[STREAM_DATA_SET_BOOL].m_pReadToString = &util::CTransferStream::ReadBoolSetToString;
	s_typeOperators[STREAM_DATA_SET_BOOL].m_pWriteFromString = &util::CTransferStream::WriteBoolSetFromString;

	s_typeOperators[STREAM_DATA_SET_BOOL_NULL].m_pReadToString = &util::CTransferStream::ReadBoolSetToString;
	s_typeOperators[STREAM_DATA_SET_BOOL_NULL].m_pWriteFromString = &util::CTransferStream::WriteBoolSetFromString;

	s_typeOperators[STREAM_DATA_SET_UINT8].m_pReadToString = &util::CTransferStream::ReadUInt8SetToString;
	s_typeOperators[STREAM_DATA_SET_UINT8].m_pWriteFromString = &util::CTransferStream::WriteUInt8SetFromString;

	s_typeOperators[STREAM_DATA_SET_UINT8_NULL].m_pReadToString = &util::CTransferStream::ReadUInt8SetToString;
	s_typeOperators[STREAM_DATA_SET_UINT8_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt8SetFromString;

	s_typeOperators[STREAM_DATA_SET_UINT16].m_pReadToString = &util::CTransferStream::ReadUInt16SetToString;
	s_typeOperators[STREAM_DATA_SET_UINT16].m_pWriteFromString = &util::CTransferStream::WriteUInt16SetFromString;

	s_typeOperators[STREAM_DATA_SET_UINT16_NULL].m_pReadToString = &util::CTransferStream::ReadUInt16SetToString;
	s_typeOperators[STREAM_DATA_SET_UINT16_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt16SetFromString;

	s_typeOperators[STREAM_DATA_SET_UINT32].m_pReadToString = &util::CTransferStream::ReadUInt32SetToString;
	s_typeOperators[STREAM_DATA_SET_UINT32].m_pWriteFromString = &util::CTransferStream::WriteUInt32SetFromString;

	s_typeOperators[STREAM_DATA_SET_UINT32_NULL].m_pReadToString = &util::CTransferStream::ReadUInt32SetToString;
	s_typeOperators[STREAM_DATA_SET_UINT32_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt32SetFromString;

	s_typeOperators[STREAM_DATA_SET_UINT64].m_pReadToString = &util::CTransferStream::ReadUInt64SetToString;
	s_typeOperators[STREAM_DATA_SET_UINT64].m_pWriteFromString = &util::CTransferStream::WriteUInt64SetFromString;

	s_typeOperators[STREAM_DATA_SET_UINT64_NULL].m_pReadToString = &util::CTransferStream::ReadUInt64SetToString;
	s_typeOperators[STREAM_DATA_SET_UINT64_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt64SetFromString;

	s_typeOperators[STREAM_DATA_SET_INT8].m_pReadToString = &util::CTransferStream::ReadInt8SetToString;
	s_typeOperators[STREAM_DATA_SET_INT8].m_pWriteFromString = &util::CTransferStream::WriteInt8SetFromString;

	s_typeOperators[STREAM_DATA_SET_INT8_NULL].m_pReadToString = &util::CTransferStream::ReadInt8SetToString;
	s_typeOperators[STREAM_DATA_SET_INT8_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt8SetFromString;

	s_typeOperators[STREAM_DATA_SET_INT16].m_pReadToString = &util::CTransferStream::ReadInt16SetToString;
	s_typeOperators[STREAM_DATA_SET_INT16].m_pWriteFromString = &util::CTransferStream::WriteInt16SetFromString;

	s_typeOperators[STREAM_DATA_SET_INT16_NULL].m_pReadToString = &util::CTransferStream::ReadInt16SetToString;
	s_typeOperators[STREAM_DATA_SET_INT16_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt16SetFromString;

	s_typeOperators[STREAM_DATA_SET_INT32].m_pReadToString = &util::CTransferStream::ReadInt32SetToString;
	s_typeOperators[STREAM_DATA_SET_INT32].m_pWriteFromString = &util::CTransferStream::WriteInt32SetFromString;

	s_typeOperators[STREAM_DATA_SET_INT32_NULL].m_pReadToString = &util::CTransferStream::ReadInt32SetToString;
	s_typeOperators[STREAM_DATA_SET_INT32_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt32SetFromString;

	s_typeOperators[STREAM_DATA_SET_INT64].m_pReadToString = &util::CTransferStream::ReadInt64SetToString;
	s_typeOperators[STREAM_DATA_SET_INT64].m_pWriteFromString = &util::CTransferStream::WriteInt64SetFromString;

	s_typeOperators[STREAM_DATA_SET_INT64_NULL].m_pReadToString = &util::CTransferStream::ReadInt64SetToString;
	s_typeOperators[STREAM_DATA_SET_INT64_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt64SetFromString;

	s_typeOperators[STREAM_DATA_SET_FLOAT].m_pReadToString = &util::CTransferStream::ReadFloatSetToString;
	s_typeOperators[STREAM_DATA_SET_FLOAT].m_pWriteFromString = &util::CTransferStream::WriteFloatSetFromString;

	s_typeOperators[STREAM_DATA_SET_FLOAT_NULL].m_pReadToString = &util::CTransferStream::ReadFloatSetToString;
	s_typeOperators[STREAM_DATA_SET_FLOAT_NULL].m_pWriteFromString = &util::CTransferStream::WriteFloatSetFromString;

	s_typeOperators[STREAM_DATA_SET_DOUBLE].m_pReadToString = &util::CTransferStream::ReadDoubleSetToString;
	s_typeOperators[STREAM_DATA_SET_DOUBLE].m_pWriteFromString = &util::CTransferStream::WriteDoubleSetFromString;

	s_typeOperators[STREAM_DATA_SET_DOUBLE_NULL].m_pReadToString = &util::CTransferStream::ReadDoubleSetToString;
	s_typeOperators[STREAM_DATA_SET_DOUBLE_NULL].m_pWriteFromString = &util::CTransferStream::WriteDoubleSetFromString;

	s_typeOperators[STREAM_DATA_SET_STRING].m_pReadToString = &util::CTransferStream::ReadStringSetToString;
	s_typeOperators[STREAM_DATA_SET_STRING].m_pWriteFromString = &util::CTransferStream::WriteStringSetFromString;

	s_typeOperators[STREAM_DATA_SET_STRING_NULL].m_pReadToString = &util::CTransferStream::ReadStringSetToString;
	s_typeOperators[STREAM_DATA_SET_STRING_NULL].m_pWriteFromString = &util::CTransferStream::WriteStringSetFromString;
	////////////////////////////
	s_typeOperators[STREAM_DATA_LIST_BOOL].m_pReadToString = &util::CTransferStream::ReadBoolSetToString;
	s_typeOperators[STREAM_DATA_LIST_BOOL].m_pWriteFromString = &util::CTransferStream::WriteBoolSetFromString;

	s_typeOperators[STREAM_DATA_LIST_BOOL_NULL].m_pReadToString = &util::CTransferStream::ReadBoolSetToString;
	s_typeOperators[STREAM_DATA_LIST_BOOL_NULL].m_pWriteFromString = &util::CTransferStream::WriteBoolSetFromString;

	s_typeOperators[STREAM_DATA_LIST_UINT8].m_pReadToString = &util::CTransferStream::ReadUInt8SetToString;
	s_typeOperators[STREAM_DATA_LIST_UINT8].m_pWriteFromString = &util::CTransferStream::WriteUInt8SetFromString;

	s_typeOperators[STREAM_DATA_LIST_UINT8_NULL].m_pReadToString = &util::CTransferStream::ReadUInt8SetToString;
	s_typeOperators[STREAM_DATA_LIST_UINT8_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt8SetFromString;

	s_typeOperators[STREAM_DATA_LIST_UINT16].m_pReadToString = &util::CTransferStream::ReadUInt16SetToString;
	s_typeOperators[STREAM_DATA_LIST_UINT16].m_pWriteFromString = &util::CTransferStream::WriteUInt16SetFromString;

	s_typeOperators[STREAM_DATA_LIST_UINT16_NULL].m_pReadToString = &util::CTransferStream::ReadUInt16SetToString;
	s_typeOperators[STREAM_DATA_LIST_UINT16_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt16SetFromString;

	s_typeOperators[STREAM_DATA_LIST_UINT32].m_pReadToString = &util::CTransferStream::ReadUInt32SetToString;
	s_typeOperators[STREAM_DATA_LIST_UINT32].m_pWriteFromString = &util::CTransferStream::WriteUInt32SetFromString;

	s_typeOperators[STREAM_DATA_LIST_UINT32_NULL].m_pReadToString = &util::CTransferStream::ReadUInt32SetToString;
	s_typeOperators[STREAM_DATA_LIST_UINT32_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt32SetFromString;

	s_typeOperators[STREAM_DATA_LIST_UINT64].m_pReadToString = &util::CTransferStream::ReadUInt64SetToString;
	s_typeOperators[STREAM_DATA_LIST_UINT64].m_pWriteFromString = &util::CTransferStream::WriteUInt64SetFromString;

	s_typeOperators[STREAM_DATA_LIST_UINT64_NULL].m_pReadToString = &util::CTransferStream::ReadUInt64SetToString;
	s_typeOperators[STREAM_DATA_LIST_UINT64_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt64SetFromString;

	s_typeOperators[STREAM_DATA_LIST_INT8].m_pReadToString = &util::CTransferStream::ReadInt8SetToString;
	s_typeOperators[STREAM_DATA_LIST_INT8].m_pWriteFromString = &util::CTransferStream::WriteInt8SetFromString;

	s_typeOperators[STREAM_DATA_LIST_INT8_NULL].m_pReadToString = &util::CTransferStream::ReadInt8SetToString;
	s_typeOperators[STREAM_DATA_LIST_INT8_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt8SetFromString;

	s_typeOperators[STREAM_DATA_LIST_INT16].m_pReadToString = &util::CTransferStream::ReadInt16SetToString;
	s_typeOperators[STREAM_DATA_LIST_INT16].m_pWriteFromString = &util::CTransferStream::WriteInt16SetFromString;

	s_typeOperators[STREAM_DATA_LIST_INT16_NULL].m_pReadToString = &util::CTransferStream::ReadInt16SetToString;
	s_typeOperators[STREAM_DATA_LIST_INT16_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt16SetFromString;

	s_typeOperators[STREAM_DATA_LIST_INT32].m_pReadToString = &util::CTransferStream::ReadInt32SetToString;
	s_typeOperators[STREAM_DATA_LIST_INT32].m_pWriteFromString = &util::CTransferStream::WriteInt32SetFromString;

	s_typeOperators[STREAM_DATA_LIST_INT32_NULL].m_pReadToString = &util::CTransferStream::ReadInt32SetToString;
	s_typeOperators[STREAM_DATA_LIST_INT32_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt32SetFromString;

	s_typeOperators[STREAM_DATA_LIST_INT64].m_pReadToString = &util::CTransferStream::ReadInt64SetToString;
	s_typeOperators[STREAM_DATA_LIST_INT64].m_pWriteFromString = &util::CTransferStream::WriteInt64SetFromString;

	s_typeOperators[STREAM_DATA_LIST_INT64_NULL].m_pReadToString = &util::CTransferStream::ReadInt64SetToString;
	s_typeOperators[STREAM_DATA_LIST_INT64_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt64SetFromString;

	s_typeOperators[STREAM_DATA_LIST_FLOAT].m_pReadToString = &util::CTransferStream::ReadFloatSetToString;
	s_typeOperators[STREAM_DATA_LIST_FLOAT].m_pWriteFromString = &util::CTransferStream::WriteFloatSetFromString;

	s_typeOperators[STREAM_DATA_LIST_FLOAT_NULL].m_pReadToString = &util::CTransferStream::ReadFloatSetToString;
	s_typeOperators[STREAM_DATA_LIST_FLOAT_NULL].m_pWriteFromString = &util::CTransferStream::WriteFloatSetFromString;

	s_typeOperators[STREAM_DATA_LIST_DOUBLE].m_pReadToString = &util::CTransferStream::ReadDoubleSetToString;
	s_typeOperators[STREAM_DATA_LIST_DOUBLE].m_pWriteFromString = &util::CTransferStream::WriteDoubleSetFromString;

	s_typeOperators[STREAM_DATA_LIST_DOUBLE_NULL].m_pReadToString = &util::CTransferStream::ReadDoubleSetToString;
	s_typeOperators[STREAM_DATA_LIST_DOUBLE_NULL].m_pWriteFromString = &util::CTransferStream::WriteDoubleSetFromString;

	s_typeOperators[STREAM_DATA_LIST_STRING].m_pReadToString = &util::CTransferStream::ReadStringSetToString;
	s_typeOperators[STREAM_DATA_LIST_STRING].m_pWriteFromString = &util::CTransferStream::WriteStringSetFromString;

	s_typeOperators[STREAM_DATA_LIST_STRING_NULL].m_pReadToString = &util::CTransferStream::ReadStringSetToString;
	s_typeOperators[STREAM_DATA_LIST_STRING_NULL].m_pWriteFromString = &util::CTransferStream::WriteStringSetFromString;
	/////////////////////////////
	s_typeOperators[STREAM_DATA_ARRAY_BOOL].m_pReadToString = &util::CTransferStream::ReadBoolSetToString;
	s_typeOperators[STREAM_DATA_ARRAY_BOOL].m_pWriteFromString = &util::CTransferStream::WriteBoolSetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_BOOL_NULL].m_pReadToString = &util::CTransferStream::ReadBoolSetToString;
	s_typeOperators[STREAM_DATA_ARRAY_BOOL_NULL].m_pWriteFromString = &util::CTransferStream::WriteBoolSetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_UINT8].m_pReadToString = &util::CTransferStream::ReadUInt8SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_UINT8].m_pWriteFromString = &util::CTransferStream::WriteUInt8SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_UINT8_NULL].m_pReadToString = &util::CTransferStream::ReadUInt8SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_UINT8_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt8SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_UINT16].m_pReadToString = &util::CTransferStream::ReadUInt16SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_UINT16].m_pWriteFromString = &util::CTransferStream::WriteUInt16SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_UINT16_NULL].m_pReadToString = &util::CTransferStream::ReadUInt16SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_UINT16_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt16SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_UINT32].m_pReadToString = &util::CTransferStream::ReadUInt32SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_UINT32].m_pWriteFromString = &util::CTransferStream::WriteUInt32SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_UINT32_NULL].m_pReadToString = &util::CTransferStream::ReadUInt32SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_UINT32_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt32SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_UINT64].m_pReadToString = &util::CTransferStream::ReadUInt64SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_UINT64].m_pWriteFromString = &util::CTransferStream::WriteUInt64SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_UINT64_NULL].m_pReadToString = &util::CTransferStream::ReadUInt64SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_UINT64_NULL].m_pWriteFromString = &util::CTransferStream::WriteUInt64SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_INT8].m_pReadToString = &util::CTransferStream::ReadInt8SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_INT8].m_pWriteFromString = &util::CTransferStream::WriteInt8SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_INT8_NULL].m_pReadToString = &util::CTransferStream::ReadInt8SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_INT8_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt8SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_INT16].m_pReadToString = &util::CTransferStream::ReadInt16SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_INT16].m_pWriteFromString = &util::CTransferStream::WriteInt16SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_INT16_NULL].m_pReadToString = &util::CTransferStream::ReadInt16SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_INT16_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt16SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_INT32].m_pReadToString = &util::CTransferStream::ReadInt32SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_INT32].m_pWriteFromString = &util::CTransferStream::WriteInt32SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_INT32_NULL].m_pReadToString = &util::CTransferStream::ReadInt32SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_INT32_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt32SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_INT64].m_pReadToString = &util::CTransferStream::ReadInt64SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_INT64].m_pWriteFromString = &util::CTransferStream::WriteInt64SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_INT64_NULL].m_pReadToString = &util::CTransferStream::ReadInt64SetToString;
	s_typeOperators[STREAM_DATA_ARRAY_INT64_NULL].m_pWriteFromString = &util::CTransferStream::WriteInt64SetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_FLOAT].m_pReadToString = &util::CTransferStream::ReadFloatSetToString;
	s_typeOperators[STREAM_DATA_ARRAY_FLOAT].m_pWriteFromString = &util::CTransferStream::WriteFloatSetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_FLOAT_NULL].m_pReadToString = &util::CTransferStream::ReadFloatSetToString;
	s_typeOperators[STREAM_DATA_ARRAY_FLOAT_NULL].m_pWriteFromString = &util::CTransferStream::WriteFloatSetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_DOUBLE].m_pReadToString = &util::CTransferStream::ReadDoubleSetToString;
	s_typeOperators[STREAM_DATA_ARRAY_DOUBLE].m_pWriteFromString = &util::CTransferStream::WriteDoubleSetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_DOUBLE_NULL].m_pReadToString = &util::CTransferStream::ReadDoubleSetToString;
	s_typeOperators[STREAM_DATA_ARRAY_DOUBLE_NULL].m_pWriteFromString = &util::CTransferStream::WriteDoubleSetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_STRING].m_pReadToString = &util::CTransferStream::ReadStringSetToString;
	s_typeOperators[STREAM_DATA_ARRAY_STRING].m_pWriteFromString = &util::CTransferStream::WriteStringSetFromString;

	s_typeOperators[STREAM_DATA_ARRAY_STRING_NULL].m_pReadToString = &util::CTransferStream::ReadStringSetToString;
	s_typeOperators[STREAM_DATA_ARRAY_STRING_NULL].m_pWriteFromString = &util::CTransferStream::WriteStringSetFromString;
}

//////////////////////////////////////////////////////////////////////////

// Read the native types from the front of the buffer
// Write the native types to the end of the buffer
CTransferStream& CTransferStream::operator>>(std::vector<bool>::reference output)
{
    //// If this assert is hit the stream wasn't long enough to read from
	//assert(readOffset+1 <=numberOfBitsUsed);
	if(m_readOffset + 1 > m_writeOffset) {
		output = false;
		return *this;
	}

	//// Check that bit
	//if (ReadBit())
	// Is it faster to just write it out here?
	if(m_data[DIV8(m_readOffset)] & (0x80 >> MOD8(m_readOffset++))) {
		output = true;
		return *this;
	}

	output = false;
	return *this;
}

CTransferStream& CTransferStream::operator>>(bool& output)
{
	//// If this assert is hit the stream wasn't long enough to read from
	//assert(readOffset+1 <=numberOfBitsUsed);
	if(m_readOffset + 1 > m_writeOffset) {
		output = false;
		return *this;
	}

	//// Check that bit
	//if (ReadBit())
	// Is it faster to just write it out here?
	if(m_data[DIV8(m_readOffset)] & (0x80 >> MOD8(m_readOffset++))) {
		output = true;
		return *this;
	}

	output = false;
	return *this;
}

CTransferStream& CTransferStream::operator>>(char& output)
{
	if(!ReadBits((unsigned char*) &output, sizeof(output) * 8)){
		output = 0;
	}
	return *this;
}

CTransferStream& CTransferStream::operator>>(uint8_t& output)
{
	if(!ReadBits((unsigned char*) &output, sizeof(output) * 8)){
		output = 0;
	}
	return *this;
}

CTransferStream& CTransferStream::operator>>(int8_t& output)
{
	if(!ReadBits((unsigned char*) &output, sizeof(output) * 8)) {
		output = 0;
	}
	return *this;
}

CTransferStream& CTransferStream::operator>>(uint16_t& output)
{
	if(!ReadBits((unsigned char*)&output, sizeof(uint16_t) * 8)){
		output = 0;
    } else {
		if(m_isLittleEndian) {
			ReverseBytes((char*)&output, sizeof(uint16_t));
		}
	}
	return *this;
}

CTransferStream& CTransferStream::operator>>(int16_t& output)
{
	if(!ReadBits((unsigned char*)&output, sizeof(int16_t) * 8)){
		output = 0;
	} else {
		if(m_isLittleEndian) {
			ReverseBytes((char*)&output, sizeof(int16_t));
		}
	}
	return *this;
}

CTransferStream& CTransferStream::operator>>(uint32_t& output)
{
	if(!ReadBits((unsigned char*)&output, sizeof(uint32_t) * 8)){
		output = 0;
    } else {
		if(m_isLittleEndian) {
			ReverseBytes((char*)&output, sizeof(uint32_t));
		}
	}
	return *this;
}

CTransferStream& CTransferStream::operator>>(int32_t& output)
{
	if(!ReadBits((unsigned char*) &output, sizeof(int32_t) * 8)){
		output = 0;
    } else {
		if(m_isLittleEndian) {
			ReverseBytes((char*)&output, sizeof(int32_t));
		}
	}
	return *this;
}

#ifndef NO_INT64
CTransferStream& CTransferStream::operator>>(uint64_t& output)
{
	if(!ReadBits((unsigned char*) &output, sizeof(uint64_t) * 8)) {
		output = 0;
	} else {
		if(m_isLittleEndian) {
			ReverseBytes((char*)&output, sizeof(uint64_t));
		}
	}
	return *this;
}

CTransferStream& CTransferStream::operator>>(int64_t& output)
{
	if(!ReadBits((unsigned char*) &output, sizeof(int64_t) * 8)){
		output = 0;
	} else {
		if(m_isLittleEndian) {
			ReverseBytes((char*)&output, sizeof(int64_t));
		}
	}
	return *this;
}
#endif

CTransferStream& CTransferStream::operator>>(float& output)
{
	if(!ReadBits((unsigned char*) &output, sizeof(float) * 8)){
		output = 0.;
	} else {
		if(m_isLittleEndian) {
			ReverseBytes((char*)&output, sizeof(float));
		}
	}
	return *this;
}

CTransferStream& CTransferStream::operator>>(double& output)
{
	if(!ReadBits((unsigned char*)&output, sizeof(double) * 8)){
		output = 0.;
	} else {
		if(m_isLittleEndian) {
			ReverseBytes((char*)&output, sizeof(double));
		}
	}
	return *this;
}
// Read an array or casted stream
bool CTransferStream::ReadBytes( char* output, const int numberOfBytes )
{
	return ReadBits((unsigned char*)output, numberOfBytes * 8);
}

#ifndef NO_TEMPLATE

CTransferStream& CTransferStream::operator>>(std::string& output)
{
	if(!output.empty()) {
		output.clear();
	}

    uint16_t len = 0;
	*this >> len;
	if(len < 1) {
		return *this;
	}

	char szBuf[TS_STRING_MAX_SIZE];
	ReadBytes(szBuf, len);
	szBuf[len] = '\0';
	output = szBuf;
	return *this;
}

#endif

// Sets the read pointer back to the beginning of your data.
void CTransferStream::ResetReadPointer(void)
{
	m_readOffset = 0;
}

// Sets the write pointer back to the beginning of your data.
void CTransferStream::ResetWritePointer(void)
{
	m_writeOffset = 0;
}

// Write a 0
void CTransferStream::Write0( void )
{
	AddBitsAndReallocate(1);

	// New bytes need to be zeroed
	if(MOD8(m_writeOffset) == 0) {
		m_data[DIV8(m_writeOffset)] = 0;
	}

	++m_writeOffset;
}

// Write a 1
void CTransferStream::Write1(void)
{
	AddBitsAndReallocate(1);

	int numberOfBitsMod8 = MOD8(m_writeOffset);

	if(0 == numberOfBitsMod8) {
		m_data[DIV8(m_writeOffset)] = 0x80;
	} else {
		// Set the bit to 1
		m_data[DIV8(m_writeOffset)] |= (0x80 >> numberOfBitsMod8);
	}

	++m_writeOffset;
}

// Returns true if the next data read is a 1, false if it is a 0
bool CTransferStream::ReadBit(void)
{
#pragma warning( disable : 4800 )
	return (bool) (m_data[DIV8(m_readOffset)] & (0x80 >> MOD8(m_readOffset++)));
#pragma warning( default : 4800 )
}

// Align the bit stream to the byte boundary and then write the specified number of bits.
// This is faster than WriteBits but wastes the bits to do the alignment and requires you to call
// SetReadToByteAlignment at the corresponding read position
void CTransferStream::WriteAlignedBytes(const unsigned char* input,
	const int numberOfBytesToWrite)
{
	if(numberOfBytesToWrite < 1) {
#ifdef _DEBUG
		assert(false);
#endif
		return;
	}

	AlignWriteToByteBoundary();
	// Allocate enough memory to hold everything
	AddBitsAndReallocate(MUL8(numberOfBytesToWrite));

	// Write the data
	memcpy(m_data + DIV8(m_writeOffset), input, numberOfBytesToWrite);

	m_writeOffset += MUL8(numberOfBytesToWrite);
}

// Read bits, starting at the next aligned bits. Note that the modulus 8 starting offset of the
// sequence must be the same as was used with WriteBits. This will be a problem with packet coalescence
// unless you byte align the coalesced packets.
bool CTransferStream::ReadAlignedBytes(unsigned char* output,
	const int numberOfBytesToRead)
{
	if(numberOfBytesToRead < 1) {
#ifdef _DEBUG
		assert(false);
#endif
		return false;
	}

	// Byte align
	AlignReadToByteBoundary();

	if(m_readOffset + MUL8(numberOfBytesToRead) > m_writeOffset) {
		return false;
	}

	// Write the data
	memcpy(output, m_data + DIV8(m_readOffset), numberOfBytesToRead);

	m_readOffset += MUL8(numberOfBytesToRead);

	return true;
}

// Align the next write and/or read to a byte boundary.  This can be used to 'waste' bits to byte align for efficiency reasons
void CTransferStream::AlignWriteToByteBoundary(void)
{
	if(m_writeOffset) {
		m_writeOffset += (8 - (MOD8( m_writeOffset - 1) + 1));
	}
}

// Align the next write and/or read to a byte boundary.  This can be used to 'waste' bits to byte align for efficiency reasons
void CTransferStream::AlignReadToByteBoundary(void)
{
	if(m_readOffset) {
		m_readOffset += (8 - (MOD8(m_readOffset - 1) + 1));
	}
}

// Write numberToWrite bits from the input source
void CTransferStream::WriteBits(const unsigned char *input,
	int numberOfBitsToWrite, const bool rightAlignedBits)
{
	if(numberOfBitsToWrite < 1) {
#ifdef _DEBUG
		assert(false);
#endif
		return;
	}

	AddBitsAndReallocate(numberOfBitsToWrite);
	uint32_t offset = 0;
	unsigned char dataByte;

	int numberOfBitsUsedMod8 = MOD8(m_writeOffset);

	// Faster to put the while at the top surprisingly enough
	while(numberOfBitsToWrite > 0) {

		unsigned char* dest = m_data + DIV8(m_writeOffset);
		dataByte = *(input + offset);
		// rightAlignedBits means in the case of a partial byte, the bits are aligned from the right (bit 0) rather than the left (as in the normal internal representation)
		if(numberOfBitsToWrite < 8 && rightAlignedBits) {
			// shift left to get the bits on the left, as in our internal representation
			dataByte <<= (8 - numberOfBitsToWrite);
		}
		// Writing to a new byte each time
		if(0 == numberOfBitsUsedMod8) {
			*dest = dataByte;
		} else {
			// Copy over the new data.
			// First half
			*dest |= (dataByte >> numberOfBitsUsedMod8);
			// If we didn't write it all out in the first half (8 - (numberOfBitsUsed%8) is the number we wrote in the first half)
			//if((8 - numberOfBitsUsedMod8) < 8 && (8 - numberOfBitsUsedMod8) < numberOfBitsToWrite) {
			if((8 - numberOfBitsUsedMod8) < numberOfBitsToWrite) {
				// Second half (overlaps byte boundary)
				*(dest + 1) = (unsigned char) (dataByte << (8 - numberOfBitsUsedMod8));
			}
		}

		if(numberOfBitsToWrite >= 8) {
			m_writeOffset += 8;
		} else {
			m_writeOffset += numberOfBitsToWrite;
		}

		numberOfBitsToWrite -= 8;

		++offset;
	}
}

// Set the stream to some initial data.  For internal use
void CTransferStream::SetData(const char* input, const int numberOfBits)
{
#ifdef _DEBUG
	// Make sure the stream is clear
	assert(0 == m_writeOffset);
#endif

	if(numberOfBits < 1) {
		return;
	}

	AddBitsAndReallocate(numberOfBits);

	memcpy(m_data, input, TS_BITS_TO_BYTES(numberOfBits));

	m_writeOffset = numberOfBits;
}

// Read numberOfBitsToRead bits to the output source
// alignBitsToRight should be set to true to convert internal bit stream data to user data
// It should be false if you used WriteBits with rightAlignedBits false
bool CTransferStream::ReadBits(unsigned char* output,
	int numberOfBitsToRead, const bool alignBitsToRight)
{
	if(numberOfBitsToRead < 1) {
#ifdef _DEBUG
		assert(false);
#endif
		return false;
	}

	if(m_readOffset + numberOfBitsToRead > m_writeOffset) {
		return false;
	}

	uint32_t offset = 0;

	memset(output, 0, TS_BITS_TO_BYTES(numberOfBitsToRead));

	int readOffsetMod8 = MOD8(m_readOffset);

	// Faster to put the while at the top surprisingly enough
	while(numberOfBitsToRead > 0) {
		unsigned char* dest = output + offset;
		unsigned char* src = m_data + DIV8(m_readOffset);
		// First half
		*dest |= *src << readOffsetMod8;
		// If we have a second half, we didn't read enough bytes in the first half
		if(readOffsetMod8 > 0 && numberOfBitsToRead > (8 - readOffsetMod8)) {
			 // Second half (overlaps byte boundary)
			*dest |= *(src + 1) >> (8 - readOffsetMod8);
		}

		numberOfBitsToRead -= 8;
		// Reading a partial byte for the last byte, shift right so the data is aligned on the right
		if(numberOfBitsToRead < 0) {

			if(alignBitsToRight) {
				*dest >>= -numberOfBitsToRead;
			}

			m_readOffset += (8 + numberOfBitsToRead);
		} else {
			m_readOffset += 8;
		}
		++offset;
	}
	return true;
}

// Reallocates (if necessary) in preparation of writing numberOfBitsToWrite
void CTransferStream::AddBitsAndReallocate(const uint32_t writeBitSize)
{
	if(writeBitSize < 1) {
		return;
	}

	uint32_t newAllocBitSize = writeBitSize + m_writeOffset;
	// If we need to allocate 1 or more new bytes
	if(newAllocBitSize > 0 && DIV8(m_allocBitSize - 1) < DIV8(newAllocBitSize - 1)) {
		if(m_copyData == false) {
			// If this assert hits then we need to specify true for the third parameter in the constructor
			// It needs to reallocate to hold all the data and can't do it unless we allocated to begin with
			assert(false);
			return;
		}

		// Less memory efficient but saves on news and deletes
		newAllocBitSize = newAllocBitSize * 2;
		// Use realloc and free so we are more efficient than delete and new for resizing
		uint32_t newAllocByteSize = TS_BITS_TO_BYTES(newAllocBitSize);
		if(m_data == (unsigned char*)m_stackData) {
			 if(newAllocByteSize > TRANSFERSTREAM_STACK_ALLOCA_SIZE) {
				 m_data = (unsigned char*) malloc(newAllocByteSize);

				 // need to copy the stack data over to our new memory area too
				 memcpy((void *)m_data, (void *)m_stackData, TS_BITS_TO_BYTES(m_allocBitSize));
			 }
		} else {
			m_data = (unsigned char*) realloc(m_data, newAllocBitSize);
		}

#ifdef _DEBUG
		// Make sure realloc succeeded
		assert( m_data );
#endif
	}

	if(newAllocBitSize > m_allocBitSize) {
		m_allocBitSize = newAllocBitSize;
	}
}

// Should hit if reads didn't match writes
void CTransferStream::AssertStreamEmpty(void)
{
	assert(m_readOffset == m_writeOffset);
}

void CTransferStream::PrintBits(void) const
{
	if(m_writeOffset < 1) {
		return;
	}
	uint32_t byteSize = TS_BITS_TO_BYTES(m_writeOffset);
	for(uint32_t counter = 0; counter < byteSize; ++counter) {
		int stop;

		if(counter == DIV8(m_writeOffset - 1)) {
			stop = 8 - (MOD8(m_writeOffset - 1) + 1);
		} else {
			stop = 0;
		}

		for(int counter2 = 7; counter2 >= stop; --counter2) {
			if((m_data[counter] >> counter2) & 1) {
				putchar('1');
			} else {
				putchar('0');
			}
		}
		putchar(' ');
	}
	putchar('\n');
}


// Exposes the data for you to look at, like PrintBits does.
// Data will point to the stream.  Returns the length in bits of the stream.
int CTransferStream::CopyData(unsigned char** data) const
{
	if(NULL == data) {
		return 0;
	}

	if(m_writeOffset < 1) {
#ifdef _DEBUG
		assert(false);
#endif
		return 0;
	}

	uint32_t byteSize = TS_BITS_TO_BYTES(m_writeOffset);
	*data = new unsigned char [byteSize];
	memcpy(*data, m_data, sizeof(unsigned char) * byteSize);
	return m_writeOffset;
}

// Ignore data we don't intend to read
void CTransferStream::IgnoreBits(const int numberOfBits)
{
	m_readOffset += numberOfBits;
}

// Move the write pointer to a position on the array.  Dangerous if you don't know what you are doing!
void CTransferStream::SetWriteOffset(const uint32_t offset)
{
	m_writeOffset = offset;
}

// Returns the length in bits of the stream
uint32_t CTransferStream::GetWriteOffset(void) const
{
	return m_writeOffset;
}

// Returns the length in bytes of the stream
uint32_t CTransferStream::GetNumberOfBytesUsed(void) const
{
	return TS_BITS_TO_BYTES(m_writeOffset);
}

// Move the read pointer to a position on the array.
void CTransferStream::SetReadOffset(const uint32_t offset)
{
	m_readOffset = offset;
}
// Returns the number of bits into the stream that we have read
uint32_t CTransferStream::GetReadOffset(void) const
{
	return m_readOffset;
}

// Returns the number of bits left in the stream that haven't been read
uint32_t CTransferStream::GetNumberOfUnreadBits(void) const
{
	if(m_writeOffset > m_readOffset) {
		return m_writeOffset - m_readOffset;
	} else {
		return 0;
	}
}

// Exposes the internal data
const char* CTransferStream::GetData(void) const
{
	return (char*)m_data;
}

// If we used the constructor version with copy data off, this makes sure it is set to on and the data pointed to is copied.
void CTransferStream::AssertCopyData(void)
{
	if(m_copyData == false) {
		m_copyData = true;

		if(m_allocBitSize > 0) {
			uint32_t allocBitSize = TS_BITS_TO_BYTES(m_allocBitSize);
			unsigned char* newdata = (unsigned char*) malloc(allocBitSize);

#ifdef _DEBUG
			assert(m_data);
#endif
			memcpy(newdata, m_data, allocBitSize);
			m_data = newdata;
		} else {
			m_data = NULL;
		}
	}
}

