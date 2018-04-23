/*
 * File:   StreamDataType.h
 * Author: Jehu Shaw
 *
 * Created on 2014_5_8, 14:09
 */

#ifndef __STREAMDATATYPE_H__
#define __STREAMDATATYPE_H__

namespace util {

enum eStreamDataType {
	STREAM_DATA_NIL,
	STREAM_DATA_CHAR,
	STREAM_DATA_CHAR_NULL,
	STREAM_DATA_BOOL,
	STREAM_DATA_BOOL_NULL,
	STREAM_DATA_INT8,
	STREAM_DATA_INT8_NULL,
	STREAM_DATA_UINT8,
	STREAM_DATA_UINT8_NULL,
	STREAM_DATA_INT16,
	STREAM_DATA_INT16_NULL,
	STREAM_DATA_UINT16,
	STREAM_DATA_UINT16_NULL,
	STREAM_DATA_INT32,
	STREAM_DATA_INT32_NULL,
	STREAM_DATA_UINT32,
	STREAM_DATA_UINT32_NULL,
	STREAM_DATA_FLOAT,
	STREAM_DATA_FLOAT_NULL,
	STREAM_DATA_INT64,
	STREAM_DATA_INT64_NULL,
	STREAM_DATA_UINT64,
	STREAM_DATA_UINT64_NULL,
	STREAM_DATA_DOUBLE,
	STREAM_DATA_DOUBLE_NULL,
	STREAM_DATA_STD_STRING,
	STREAM_DATA_STD_STRING_NULL,
	STREAM_DATA_C_STRING,
	STREAM_DATA_C_STRING_NULL,
	////////////////////////////
	STREAM_DATA_VECTOR_BOOL,
	STREAM_DATA_VECTOR_BOOL_NULL,
	STREAM_DATA_VECTOR_INT8,
	STREAM_DATA_VECTOR_INT8_NULL,
	STREAM_DATA_VECTOR_UINT8,
	STREAM_DATA_VECTOR_UINT8_NULL,
	STREAM_DATA_VECTOR_INT16,
	STREAM_DATA_VECTOR_INT16_NULL,
	STREAM_DATA_VECTOR_UINT16,
	STREAM_DATA_VECTOR_UINT16_NULL,
	STREAM_DATA_VECTOR_INT32,
	STREAM_DATA_VECTOR_INT32_NULL,
	STREAM_DATA_VECTOR_UINT32,
	STREAM_DATA_VECTOR_UINT32_NULL,
	STREAM_DATA_VECTOR_FLOAT,
	STREAM_DATA_VECTOR_FLOAT_NULL,
	STREAM_DATA_VECTOR_INT64,
	STREAM_DATA_VECTOR_INT64_NULL,
	STREAM_DATA_VECTOR_UINT64,
	STREAM_DATA_VECTOR_UINT64_NULL,
	STREAM_DATA_VECTOR_DOUBLE,
	STREAM_DATA_VECTOR_DOUBLE_NULL,
	STREAM_DATA_VECTOR_STRING,
	STREAM_DATA_VECTOR_STRING_NULL,
	///////////////////////////////
	STREAM_DATA_SET_BOOL,
	STREAM_DATA_SET_BOOL_NULL,
	STREAM_DATA_SET_INT8,
	STREAM_DATA_SET_INT8_NULL,
	STREAM_DATA_SET_UINT8,
	STREAM_DATA_SET_UINT8_NULL,
	STREAM_DATA_SET_INT16,
	STREAM_DATA_SET_INT16_NULL,
	STREAM_DATA_SET_UINT16,
	STREAM_DATA_SET_UINT16_NULL,
	STREAM_DATA_SET_INT32,
	STREAM_DATA_SET_INT32_NULL,
	STREAM_DATA_SET_UINT32,
	STREAM_DATA_SET_UINT32_NULL,
	STREAM_DATA_SET_FLOAT,
	STREAM_DATA_SET_FLOAT_NULL,
	STREAM_DATA_SET_INT64,
	STREAM_DATA_SET_INT64_NULL,
	STREAM_DATA_SET_UINT64,
	STREAM_DATA_SET_UINT64_NULL,
	STREAM_DATA_SET_DOUBLE,
	STREAM_DATA_SET_DOUBLE_NULL,
	STREAM_DATA_SET_STRING,
	STREAM_DATA_SET_STRING_NULL,
	////////////////////////////
	STREAM_DATA_LIST_BOOL,
	STREAM_DATA_LIST_BOOL_NULL,
	STREAM_DATA_LIST_INT8,
	STREAM_DATA_LIST_INT8_NULL,
	STREAM_DATA_LIST_UINT8,
	STREAM_DATA_LIST_UINT8_NULL,
	STREAM_DATA_LIST_INT16,
	STREAM_DATA_LIST_INT16_NULL,
	STREAM_DATA_LIST_UINT16,
	STREAM_DATA_LIST_UINT16_NULL,
	STREAM_DATA_LIST_INT32,
	STREAM_DATA_LIST_INT32_NULL,
	STREAM_DATA_LIST_UINT32,
	STREAM_DATA_LIST_UINT32_NULL,
	STREAM_DATA_LIST_FLOAT,
	STREAM_DATA_LIST_FLOAT_NULL,
	STREAM_DATA_LIST_INT64,
	STREAM_DATA_LIST_INT64_NULL,
	STREAM_DATA_LIST_UINT64,
	STREAM_DATA_LIST_UINT64_NULL,
	STREAM_DATA_LIST_DOUBLE,
	STREAM_DATA_LIST_DOUBLE_NULL,
	STREAM_DATA_LIST_STRING,
	STREAM_DATA_LIST_STRING_NULL,
	/////////////////////////////
	STREAM_DATA_ARRAY_BOOL,
	STREAM_DATA_ARRAY_BOOL_NULL,
	STREAM_DATA_ARRAY_INT8,
	STREAM_DATA_ARRAY_INT8_NULL,
	STREAM_DATA_ARRAY_UINT8,
	STREAM_DATA_ARRAY_UINT8_NULL,
	STREAM_DATA_ARRAY_INT16,
	STREAM_DATA_ARRAY_INT16_NULL,
	STREAM_DATA_ARRAY_UINT16,
	STREAM_DATA_ARRAY_UINT16_NULL,
	STREAM_DATA_ARRAY_INT32,
	STREAM_DATA_ARRAY_INT32_NULL,
	STREAM_DATA_ARRAY_UINT32,
	STREAM_DATA_ARRAY_UINT32_NULL,
	STREAM_DATA_ARRAY_FLOAT,
	STREAM_DATA_ARRAY_FLOAT_NULL,
	STREAM_DATA_ARRAY_INT64,
	STREAM_DATA_ARRAY_INT64_NULL,
	STREAM_DATA_ARRAY_UINT64,
	STREAM_DATA_ARRAY_UINT64_NULL,
	STREAM_DATA_ARRAY_DOUBLE,
	STREAM_DATA_ARRAY_DOUBLE_NULL,
	STREAM_DATA_ARRAY_STRING,
	STREAM_DATA_ARRAY_STRING_NULL,
	STREAM_DATA_SIZE,
};

extern size_t GetStoreStreamSize(uint8_t nType);

inline static uint8_t GetInnerType(uint8_t nType) {
	if(nType < STREAM_DATA_VECTOR_BOOL) {
		return nType;
	}
	return (nType - 5) % 24 + 3;
}

inline static bool IsIgnoreType(uint8_t nType) {
	return !(nType & 0x1);
}

inline static bool IsUpdateType(uint8_t nType) {
	return nType & 0x1;
}

inline static uint8_t ToUpdateType(uint8_t nType) {
	if(IsUpdateType(nType)) {
		return nType;
	}
	return nType - 1;
}

inline static uint8_t ToIgnoreType(uint8_t nType) {
	if(IsIgnoreType(nType)) {
		return nType;
	}
	return nType + 1;
}

inline static bool IsSingleType(uint8_t nType) {
	if(nType > STREAM_DATA_C_STRING_NULL
		|| STREAM_DATA_NIL == nType) 
	{
		return false;
	}
	return true;
}

inline static bool IsFixedSizeType(uint8_t nType) {
	if(nType > STREAM_DATA_DOUBLE_NULL
		|| STREAM_DATA_NIL == nType) 
	{
		return false;
	}
	return true;
}

inline static bool IsVariableLengthType(uint8_t nType) {
	if(nType <= STREAM_DATA_DOUBLE_NULL
		|| STREAM_DATA_SIZE == nType) 
	{
		return false;
	}
	return true;
}

inline static bool IsStringType(uint8_t nType) {
	if(nType == STREAM_DATA_C_STRING
		|| nType == STREAM_DATA_C_STRING_NULL
		|| nType == STREAM_DATA_STD_STRING
		|| nType == STREAM_DATA_STD_STRING_NULL)
	{
		return true;
	}
	return false;
}

inline static bool IsContainerType(uint8_t nType) {
	if(nType <= STREAM_DATA_C_STRING_NULL 
		|| STREAM_DATA_SIZE == nType) 
	{
		return false;
	}
	return true;
}

class CTypeString
{
public:
	CTypeString() : m_u8Type(false){}
	CTypeString(uint8_t u8Type, const std::string& str) : m_u8Type(u8Type), m_str(str){}

	uint8_t GetType() const { return m_u8Type; }

	bool IsIgnore() const { return IsIgnoreType(m_u8Type); }

	const std::string& GetString() const { return m_str; }

private:
	friend class CStoreStream;
	friend class CTransferStream;
	uint8_t m_u8Type;
	std::string m_str;
};

}

#endif /* __STREAMDATATYPE_H__ */
