/*
 * File:   DatabaseEnv.h
 * Author: Jehu Shaw
 *
 */

#ifndef FIELD_H
#define FIELD_H

#include "Common.h"

namespace db {

class Field
{
public:
	Field() : mValue(NULL), mField(NULL) {}  

    INLINE void SetField(char* szField) { mField = szField; }
    INLINE char* GetField() { return mField; }
	INLINE void SetValue(char* value) { mValue = value; }

	INLINE const char *GetString() { return mValue; }
	INLINE float GetFloat() { return (NULL != mValue) ? static_cast<float>(atof(mValue)) : 0; }
    INLINE double GetDouble() { return (NULL != mValue) ? atof(mValue) : 0; }
	INLINE bool GetBool() { return (NULL != mValue) ? atoi(mValue) != 0 : false; }
	INLINE uint8_t GetUInt8() { return (NULL != mValue) ? static_cast<uint8_t>(atol(mValue)) : 0; }
	INLINE int8_t GetInt8() { return (NULL != mValue) ? static_cast<int8_t>(atol(mValue)) : 0; }
	INLINE uint16_t GetUInt16() { return (NULL != mValue) ? static_cast<uint16_t>(atol(mValue)) : 0; }
	INLINE int16_t GetInt16() { return (NULL != mValue) ? static_cast<int16_t>(atol(mValue)) : 0; }
	INLINE uint32_t GetUInt32() { return (NULL != mValue) ? static_cast<uint32_t>(atol(mValue)) : 0; }
	INLINE int32_t GetInt32() { return (NULL != mValue) ? static_cast<int32_t>(atol(mValue)) : 0; }
	uint64_t GetUInt64() {
		if(NULL != mValue) {
			uint64_t value;
			#if !defined(_WIN32) && !defined(_WIN64)	// Make GCC happy.
			    sscanf(mValue, I64FMTD, (long long unsigned int*)&value);
			#else
			    sscanf(mValue, I64FMTD, &value);
			#endif
			return value;
		} else {
			return 0;
		}
	}

private:
	char* mValue;
	char* mField;
};

}

#endif  /* FIELD_H */
