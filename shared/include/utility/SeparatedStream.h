/*
 * File:   SeparatedStream.h
 * Author: Jehu Shaw
 *
 * Created on 2014_5_8, 14:09
 */

#ifndef SEPARATEDSTREAM_H
#define SEPARATEDSTREAM_H

#include "Common.h"
#include <stdlib.h>
#include <float.h>

namespace util {

enum SeparatedType {
	SEPDTYPE_FIRSTANDLAST_EMPTY,
	SEPDTYPE_ONLYFIRST_DELIM,
	SEPDTYPE_ONLYLAST_DELIM,
	SEPDTYPE_FIRSTANDLAST_DELIM,
};

// Arbitrary size
#define SEPDSTREAM_STACK_ALLOC_SIZE 64

#ifndef CHAR_BIT
#  define CHAR_BIT 8
#endif

/* Nonzero if the integer type T is signed.  */
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))

/* Bound on length of the string representing an integer value of type T.
   Subtract one for the sign bit if T is signed;
   302 / 1000 is log10 (2) rounded up;
   add one for integer division truncation;
   add one more for a minus sign if t is signed.  */
#define INT_STRLEN_BOUND(t) \
  ((sizeof (t) * CHAR_BIT - TYPE_SIGNED (t)) * 302 / 1000 \
   + 1 + TYPE_SIGNED (t))

#ifndef DBL_DECIMAL_DIG
#define DBL_DECIMAL_DIG (DBL_DIG + 2)
#endif // !DBL_DECIMAL_DIG

#ifndef FLT_DECIMAL_DIG
#define FLT_DECIMAL_DIG (FLT_DIG + 2)
#endif


class SHARED_DLL_DECL CSeparatedStream
{
	public:
		CSeparatedStream(char delim, char enclosure_delim = ',',
			SeparatedType sepdType = SEPDTYPE_FIRSTANDLAST_EMPTY,
			int allocBytes = SEPDSTREAM_STACK_ALLOC_SIZE);

		CSeparatedStream(const char* szData, int nBytesLength, bool bCopyData, char delim,
			char enclosure_delim = ',', SeparatedType sepdType = SEPDTYPE_FIRSTANDLAST_EMPTY);

		CSeparatedStream(const std::string& strData, bool bCopyData, char delim, char enclosure_delim = ',',
			SeparatedType sepdType = SEPDTYPE_FIRSTANDLAST_EMPTY);

		~CSeparatedStream();

		void Clear()
		{
            m_writeOffset = 0;
			m_readOffset = 0;
			m_moreData = false;
		}

		void Reset(const char* szData, int nBytesLength, bool bCopyData)
		{
			// The length can set zero
			if(NULL == szData || nBytesLength <= 0) {
				if(m_bCopyData && m_stackData != m_data) {
					free(m_data);
					m_data = NULL;
				}
				m_stackData[0] = '\0';
				m_data = m_stackData;
				m_size = SEPDSTREAM_STACK_ALLOC_SIZE;
				m_writeOffset = 0;
				m_readOffset = 0;
				m_bCopyData = true;
				m_moreData = false;
			} else {
				if(bCopyData) {
					if(!m_bCopyData) {
						m_stackData[0] = '\0';
						m_data = m_stackData;
						m_size = SEPDSTREAM_STACK_ALLOC_SIZE;
						m_writeOffset = 0;
						m_readOffset = 0;
						m_bCopyData = true;
						m_moreData = false;
					}

					AddBytesAndReallocate(nBytesLength + 1);

					memcpy(m_data, szData, nBytesLength);
					m_data[nBytesLength] = '\0';

					m_writeOffset = nBytesLength;
					m_readOffset = 0;
					m_moreData = true;

				} else {
					if(m_bCopyData && m_stackData != m_data) {
						free(m_data);
						m_data = NULL;
					}
					m_data = const_cast<char*>(szData);
					m_size = nBytesLength + 1;
					m_writeOffset = nBytesLength;
					m_readOffset = 0;
					m_moreData = true;
					m_bCopyData = false;
				}
			}
		}

        inline void Reset(const std::string& strData, bool bCopyData)
        {
			Reset(strData.data(), strData.length(), bCopyData);
		}

		// stream like operators for storing data
		CSeparatedStream & operator<<(bool value)
		{
			// STRLEN_BOUND(bool) + if lack delim + %c + '\0'
			int nStrBoundLen = INT_STRLEN_BOUND(bool);
			if(!AddBytesAndReallocate(nStrBoundLen + 3)) {
				return *this;
			}
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
				if(value) {
					*p++ = '1';
				} else {
					*p++ = '0';
				}
				*p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			} else {
				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
				if(value) {
					*p++ = '1';
				} else {
					*p++ = '0';
				}
				*p++ = m_delim;
				*p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
			return *this;
		}
		CSeparatedStream & operator<<(char value)
		{
			// STRLEN_BOUND(char) + if lack delim + %c + '\0'
			int nStrBoundLen = INT_STRLEN_BOUND(char);
			if(!AddBytesAndReallocate(nStrBoundLen + 3)) {
				return *this;
			}
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
				*p++ = value;
				*p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			} else {
				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
				*p++ = value;
				*p++ = m_delim;
				*p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
			return *this;
		}
		// unsigned
		CSeparatedStream & operator<<(uint8_t value)
		{
			// STRLEN_BOUND(uint8_t) + if lack delim + %c + '\0'
			int nStrBoundLen = INT_STRLEN_BOUND(uint8_t);
			if(!AddBytesAndReallocate(nStrBoundLen + 3)) {
				return *this;
			}
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
				int offset = ultostr(p, value, 10, 0);//sprintf(p, "%d", (int)value);
				p += offset;
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			} else {
				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
				int offset = ultostr(p, value, 10, 0);//sprintf(p, "%d", (int)value);
				*(p += offset) = m_delim;
				*++p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
			return *this;
		}
		CSeparatedStream & operator<<(uint16_t value)
		{
			// STRLEN_BOUND(uint16_t) + if lack delim + %c + '\0'
			int nStrBoundLen = INT_STRLEN_BOUND(uint16_t);
			if(!AddBytesAndReallocate(nStrBoundLen + 3)) {
				return *this;
			}
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
				int offset = ultostr(p, value, 10, 0);//sprintf(p, "%hu", value);
				p += offset;
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			} else {
				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
				int offset = ultostr(p, value, 10, 0);//sprintf(p, "%hu", value);
				*(p += offset) = m_delim;
				*++p = '\0';
				m_writeOffset = static_cast<int>((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
			return *this;
		}
		CSeparatedStream & operator<<(uint32_t value)
		{
			// STRLEN_BOUND(uint32_t) + if lack delim + %c + '\0'
			int nStrBoundLen = INT_STRLEN_BOUND(uint32_t);
			if(!AddBytesAndReallocate(nStrBoundLen + 3)) {
				return *this;
			}
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
				int offset = ultostr(p, value, 10, 0);//sprintf(p, "%lu", value);
				p += offset;
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			} else {
				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
				int offset = ultostr(p, value, 10, 0);//sprintf(p, "%lu", value);
				*(p += offset) = m_delim;
				*++p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
			return *this;
		}
		CSeparatedStream & operator<<(uint64_t value)
		{
			// STRLEN_BOUND(uint64_t) + if lack delim + %c + '\0'
			int nStrBoundLen = INT_STRLEN_BOUND(uint64_t);
			if(!AddBytesAndReallocate(nStrBoundLen + 3)) {
				return *this;
			}
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
				int offset = ulltostr(p, value, 10, 0);//sprintf(p, I64FMTD, value);
				p += offset;
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			} else {
				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
				int offset = ulltostr(p, value, 10, 0);//sprintf(p, I64FMTD, value);
				*(p += offset) = m_delim;
				*++p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
			return *this;
		}
		// signed as in 2e complement
		CSeparatedStream & operator<<(int8_t value)
		{
			// STRLEN_BOUND(int8_t) + if lack delim + %c + '\0'
			int nStrBoundLen = INT_STRLEN_BOUND(int8_t);
			if(!AddBytesAndReallocate(nStrBoundLen + 3)) {
				return *this;
			}
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
				int offset = ltostr(p, value, 10, 0);//sprintf(p, "%d", (int)value);
				p += offset;
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			} else {
				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
				int offset = ltostr(p, value, 10, 0);//sprintf(p, "%d", (int)value);
				*(p += offset) = m_delim;
				*++p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
			return *this;
		}
		CSeparatedStream & operator<<(int16_t value)
		{
			// STRLEN_BOUND(int16_t) + if lack delim + %c + '\0'
			int nStrBoundLen = INT_STRLEN_BOUND(int16_t);
			if(!AddBytesAndReallocate(nStrBoundLen + 3)) {
				return *this;
			}
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
				int offset = ltostr(p, value, 10, 0);//sprintf(p, "%hd", value);
				p += offset;
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			} else {
				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
				int offset = ltostr(p, value, 10, 0);//sprintf(p, "%hd", value);
				*(p += offset) = m_delim;
				*++p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
			return *this;
		}
		CSeparatedStream & operator<<(int32_t value)
		{
			// STRLEN_BOUND(int32_t) + if lack delim + %c + '\0'
			int nStrBoundLen = INT_STRLEN_BOUND(int32_t);
			if(!AddBytesAndReallocate(nStrBoundLen + 3)) {
				return *this;
			}
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
				int offset = ltostr(p, value, 10, 0);//sprintf(p, "%ld", value);
				p += offset;
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			} else {
				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
				int offset = ltostr(p, value, 10, 0);//sprintf(p, "%ld", value);
				*(p += offset) = m_delim;
				*++p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
			return *this;
		}
		CSeparatedStream & operator<<(int64_t value)
		{
			// STRLEN_BOUND(int64_t) + if lack delim + %c + '\0'
			int nStrBoundLen = INT_STRLEN_BOUND(int64_t);
			if(!AddBytesAndReallocate(nStrBoundLen + 3)) {
				return *this;
			}
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
				int offset = lltostr(p, value, 10, 0);//sprintf(p, SI64FMTD, value);
				p += offset;
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			} else {
				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
				int offset = lltostr(p, value, 10, 0);//sprintf(p, SI64FMTD, value);
				*(p += offset) = m_delim;
				*++p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
			return *this;
		}
		CSeparatedStream & operator<<(float value)
		{
			// STRLEN_BOUND(float) + if lack delim + %c + '\0'
			if(!AddBytesAndReallocate(INT_STRLEN_BOUND(float) + 3)) {
				return *this;
			}
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2))
				qgcvt(value, FLT_DECIMAL_DIG, p);
#else
				_gcvt(value, FLT_DECIMAL_DIG, p);
#endif
				int offset = strlen(p);//sprintf(p,"%g", value);
				p += offset;
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			} else {
				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2))
				qgcvt(value, FLT_DECIMAL_DIG, p);
#else
				_gcvt(value, FLT_DECIMAL_DIG, p);
#endif
				int offset = strlen(p);//sprintf(p,"%g", value);
				*(p += offset) = m_delim;
				*++p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
			return *this;
		}
		CSeparatedStream & operator<<(double value)
		{
			// STRLEN_BOUND(double) + if lack delim + '\0' + %c
			if(!AddBytesAndReallocate(INT_STRLEN_BOUND(double) + 3)) {
				return *this;
			}
			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2))
				qgcvt(value, FLT_DECIMAL_DIG, p);
#else
				_gcvt(value, FLT_DECIMAL_DIG, p);
#endif
				int offset = strlen(p);//sprintf(p, "%lg", value);
				p += offset;
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			} else {
				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2))
				qgcvt(value, FLT_DECIMAL_DIG, p);
#else
				_gcvt(value, FLT_DECIMAL_DIG, p);
#endif
				int offset = strlen(p);//sprintf(p,"%lg", value);
				*(p += offset) = m_delim;
				*++p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
			return *this;
		}
		CSeparatedStream & operator<<(const std::string & value)
		{
			int count = (int)value.length();
			const char* szData = value.c_str();
			bool bHasDelim = false;
			if(*szData == '\0') {
				bHasDelim = true;
			} else {
				for(int i = 0; i < count; ++i) {
					if(IsDelimChar(szData[i])) {
						bHasDelim = true;
						break;
					}
				}
			}

			if(bHasDelim) {
				// length + %c + '\0' + '\"' + '\"' + if lack delim
				if(!AddBytesAndReallocate(count + 5)) {
					return *this;
				}
			} else {
				// length + %c + '\0' + if lack delim
				if(!AddBytesAndReallocate(count + 3)) {
					return *this;
				}
			}

			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				if(bHasDelim) {
					char* p = m_data + m_writeOffset;
					*p++ = m_delim;
					*p++ = '\"';
					strcpy(p, szData);
					*(p += count) = '\"';
					*++p = '\0';
					m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
				} else {
					char* p = m_data + m_writeOffset;
					*p++ = m_delim;
					strcpy(p, szData);
					*(p += count) = '\0';
					m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
				}
			} else {
				if(bHasDelim) {
					char* p = m_data + m_writeOffset;
					if(0 != m_writeOffset && *(p - 1) != m_delim) {
						*p++ = m_delim;
					}
					*p++ = '\"';
					strcpy(p, szData);
					*(p += count) = '\"';
					*++p = m_delim;
					*++p = '\0';
					m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
				} else {
					char* p = m_data + m_writeOffset;
					if(0 != m_writeOffset && *(p - 1) != m_delim) {
						*p++ = m_delim;
					}
					strcpy(p, szData);
					*(p += count) = m_delim;
					*++p = '\0';
					m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
				}
			}
			m_moreData = true;
			return *this;
		}
		CSeparatedStream & operator<<(const char* szData)
		{
			if(NULL == szData) {
				return *this;
			}
			bool bHasDelim = false;
			int count = 0;
			if(*szData == '\0') {
				bHasDelim = true;
			} else {
				for(; szData[count] != '\0'; ++count) {
					if(!bHasDelim && IsDelimChar(szData[count])) {
						bHasDelim = true;
						// this count the char number, so can't break;
					}
				}
			}

			if(bHasDelim) {
				// length + %c + '\0' + '\"' + '\"' + if lack delim
				if(!AddBytesAndReallocate(count + 5)) {
					return *this;
				}
			} else {
				// length + %c + '\0' + if lack delim
				if(!AddBytesAndReallocate(count + 3)) {
					return *this;
				}
			}

			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				if(bHasDelim) {
					char* p = m_data + m_writeOffset;
					*p++ = m_delim;
					*p++ = '\"';
					strcpy(p, szData);
					*(p += count) = '\"';
					*++p = '\0';
					m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
				} else {
					char* p = m_data + m_writeOffset;
					*p++ = m_delim;
					strcpy(p, szData);
					*(p += count) = '\0';
					m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
				}
			} else {
				if(bHasDelim) {
					char* p = m_data + m_writeOffset;
					if(0 != m_writeOffset && *(p - 1) != m_delim) {
						*p++ = m_delim;
					}
					*p++ = '\"';
					strcpy(p, szData);
					*(p += count) = '\"';
					*++p = m_delim;
					*++p = '\0';
					m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
				} else {
					char* p = m_data + m_writeOffset;
					if(0 != m_writeOffset && *(p - 1) != m_delim) {
						*p++ = m_delim;
					}
					strcpy(p, szData);
					*(p += count) = m_delim;
					*++p = '\0';
					m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
				}
			}
			m_moreData = true;
			return *this;
		}
        CSeparatedStream & operator<<(CSeparatedStream& value)
        {
			if(this == &value) {
				return *this;
			}

			int count = value.m_writeOffset;
			const char* szData = value.Str();

			// length + %c + '\0' + if lack delim
			if(!AddBytesAndReallocate(count + 3)) {
				return *this;
			}

			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
				strcpy(p, szData);
				*(p += count) = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);

			} else {

				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
				strcpy(p, szData);
				*(p += count) = m_delim;
				*++p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
			return *this;
        }

        template<class T>
        CSeparatedStream & operator<<(const std::vector<T>& value)
        {
            CSeparatedStream enclosure(m_enclosure_delim,
				m_enclosure_delim, (SeparatedType)m_sepdType);

            for(int i = 0; i < (int)value.size(); ++i) {
                enclosure << (T)value[i];
            }
			enclosure.EndLine();

            *this << enclosure;
            return *this;
        }
        template<class T>
        CSeparatedStream & operator<<(const std::set<T>& value)
        {
            CSeparatedStream enclosure(m_enclosure_delim,
				m_enclosure_delim, (SeparatedType)m_sepdType);

            typename std::set<T>::const_iterator it = value.begin();
            for(; value.end() != it; ++it) {
                enclosure << *it;
            }
			enclosure.EndLine();

            *this << enclosure;
            return *this;
        }
        template<class T>
        CSeparatedStream & operator<<(const std::list<T>& value)
        {
            CSeparatedStream enclosure(m_enclosure_delim,
				m_enclosure_delim, (SeparatedType)m_sepdType);

            typename std::list<T>::const_iterator it = value.begin();
            for(; value.end() != it; ++it) {
                enclosure << *it;
            }
			enclosure.EndLine();

            *this << enclosure;
            return *this;
        }
        template<class T, size_t nSize>
        CSeparatedStream & operator<<(const T (&value)[nSize])
        {
            CSeparatedStream enclosure(m_enclosure_delim,
				m_enclosure_delim, (SeparatedType)m_sepdType);

            for(int i = 0; i < (int)nSize; ++i) {
                enclosure << value[i];
            }
			enclosure.EndLine();

            *this << enclosure;
            return *this;
        }

		void WriteNull()
		{
			// %c + '\0' + if lack delim
			if(!AddBytesAndReallocate(3)) {
				return;
			}

			if(SEPDTYPE_ONLYFIRST_DELIM == m_sepdType
				|| SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType)
			{
				char* p = m_data + m_writeOffset;
				*p++ = m_delim;
				*p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			} else {

				char* p = m_data + m_writeOffset;
				if(0 != m_writeOffset && *(p - 1) != m_delim) {
					*p++ = m_delim;
				}
				*p++ = m_delim;
				*p = '\0';
				m_writeOffset = ((uint64_t)p - (uint64_t)m_data);
			}
			m_moreData = true;
		}

		// stream like operators for reading data
		CSeparatedStream & operator>>(std::vector<bool>::reference value)
		{
            const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
			value = (strtoul(pData, NULL, 10) != 0);
			return *this;
		}
		CSeparatedStream & operator>>(bool & value)
		{
            const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
			value = (strtoul(pData, NULL, 10) != 0);
			return *this;
		}
		CSeparatedStream & operator>>(char & value)
		{
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
			value = *pData;
			return *this;
		}
		//unsigned
		CSeparatedStream & operator>>(uint8_t & value)
		{
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
			value = (uint8_t)strtoul(pData, NULL, 10);
			return *this;
		}
		CSeparatedStream & operator>>(uint16_t & value)
		{
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
			value = (uint16_t)strtoul(pData, NULL, 10);
			return *this;
		}
		CSeparatedStream & operator>>(uint32_t & value)
		{
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
			value = (uint32_t)strtoul(pData, NULL, 10);
			return *this;
		}
		CSeparatedStream & operator>>(uint64_t & value)
		{
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
			value = (uint64_t)strtoull(pData, NULL, 10);
			return *this;
		}
		//signed as in 2e complement
		CSeparatedStream & operator>>(int8_t & value)
		{
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
			value = (int8_t)strtol(pData, NULL, 10);
			return *this;
		}
		CSeparatedStream & operator>>(int16_t & value)
		{
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
			value = (int16_t)strtol(pData, NULL, 10);
			return *this;
		}
		CSeparatedStream & operator>>(int32_t & value)
		{
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
			value = strtol(pData, NULL, 10);
			return *this;
		}
		CSeparatedStream & operator>>(int64_t & value)
		{
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
			value = strtoll(pData, NULL, 10);
			return *this;
		}
		CSeparatedStream & operator>>(float & value)
		{
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
			value = (float)strtod(pData, NULL);
			return *this;
		}
		CSeparatedStream & operator>>(double & value)
		{
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
			value = strtod(pData, NULL);
			return *this;
		}
		template<size_t nSize>
		CSeparatedStream & operator>>(char (&value)[nSize])
		{
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);

			int nCpyLen = nLength > nSize ? nSize : nLength;
			memcpy(value, pData, nCpyLen);
			value[nCpyLen] = '\0';
			return *this;
		}
		CSeparatedStream & operator>>(std::string & value)
		{
			if(!value.empty()) {
				value.clear();
			}

			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);

			value.append(pData, nLength);
			return *this;
		}

        CSeparatedStream & operator>>(CSeparatedStream & value)
        {
			if(this == &value) {
				return *this;
			}
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);

            value.Reset(pData, nLength, true);
            return *this;
        }

        template<class T>
        CSeparatedStream & operator>>(std::vector<T>& value)
        {
			if(!value.empty()) {
				value.clear();
			}

			if(m_delim == m_enclosure_delim) {

                do {
                    value.resize(value.size() + 1);
                    *this >> value.back();
                } while(this->MoreData());

			} else {
				const char* pData = NULL;
				int nLength = 0;
				GetLine(pData, nLength);
				if(nLength < 1) {
					return *this;
				}

				CSeparatedStream enclosure(pData, nLength, false, m_enclosure_delim,
					m_enclosure_delim, (SeparatedType)m_sepdType);

                do {
                    value.resize(value.size() + 1);
                    enclosure >> value.back();
                } while(enclosure.MoreData());

			}
            return *this;
        }

        template<class T>
        CSeparatedStream & operator>>(std::set<T>& value)
        {
			if(!value.empty()) {
				value.clear();
			}

			if(m_delim == m_enclosure_delim) {
				T temp;
				do {
					this->operator>>(temp);
					value.insert(temp);
				} while(this->MoreData());
			} else {
				const char* pData = NULL;
				int nLength = 0;
				GetLine(pData, nLength);
				if(nLength < 1) {
					return *this;
				}

				CSeparatedStream enclosure(pData, nLength, false, m_enclosure_delim,
					m_enclosure_delim, (SeparatedType)m_sepdType);

				T temp;
				do {
					enclosure >> temp;
					value.insert(temp);
				} while(enclosure.MoreData());
			}
            return *this;
        }

        template<class T>
        CSeparatedStream & operator>>(std::list<T>& value)
        {
			if(!value.empty()) {
				value.clear();
			}

			if(m_delim == m_enclosure_delim) {
				do {
					value.resize(value.size() + 1);
					this->operator>>(value.back());
				} while(this->MoreData());
			} else {
				const char* pData = NULL;
				int nLength = 0;
				GetLine(pData, nLength);
				if(nLength < 1) {
					return *this;
				}

				CSeparatedStream enclosure(pData, nLength, false, m_enclosure_delim,
					m_enclosure_delim, (SeparatedType)m_sepdType);

				do {
					value.resize(value.size() + 1);
					enclosure >> value.back();
				} while(enclosure.MoreData());
			}
            return *this;
        }

        template<class T, size_t nSize>
        CSeparatedStream & operator>>(T (&value)[nSize])
        {
			if(m_delim == m_enclosure_delim) {
				for(int i = 0; i < nSize; ++i) {
					this->operator>>(value[i]);
					if(!this->MoreData()) {
						break;
					}
				}
			} else {
				const char* pData = NULL;
				int nLength = 0;
				GetLine(pData, nLength);

				if(nLength < 1) {
					return *this;
				}

				CSeparatedStream enclosure(pData, nLength, false, m_enclosure_delim,
					m_enclosure_delim, (SeparatedType)m_sepdType);

				for(int i = 0; i < nSize; ++i) {
					enclosure >> value[i];
					if(!enclosure.MoreData()) {
						break;
					}
				}
			}
            return *this;
        }

		void Ignore() {
			const char* pData = NULL;
			int nLength = 0;
			GetLine(pData, nLength);
		}

		bool IsNull() {
			if(m_moreData) {
				char c = m_data[m_readOffset];
				if(c == m_delim || c == '\0') {
					return true;
				}
			}
			return false;
		}

		void EndLine() {
			if(m_writeOffset < 1) {
				return;
			}
			if(SEPDTYPE_FIRSTANDLAST_DELIM == m_sepdType) {
				if(AddBytesAndReallocate(1)) {
					m_data[m_writeOffset] = m_delim;
					m_data[++m_writeOffset] = '\0';
				}
			} else if(SEPDTYPE_FIRSTANDLAST_EMPTY == m_sepdType) {
				if(m_bCopyData) {
					if(m_data[m_writeOffset - 1] == m_delim) {
						m_data[--m_writeOffset] = '\0';
						if(m_readOffset > m_writeOffset) {
							m_readOffset = m_writeOffset;
						}
					}
				}
#ifdef _DEBUG
				else {
					assert(false);
				}
#endif
			}
		}

        const char* Str() const {
            return m_data;
        }

		int GetUnreadSize() const {
			return m_writeOffset - m_readOffset;
		}

		int GetReadOffset() const {
			return m_readOffset;
		}

		int GetWriteOffset() const {
			return m_writeOffset;
		}

		bool MoreData() const {
			return m_moreData;
		}

		void SetReadOffset(int nReadOffset) {
			if(nReadOffset < 0) {
				return;
			}
			m_readOffset = nReadOffset;
		}

		void SetWriteOffset(int nWriteOffset) {
			if(nWriteOffset < 0) {
				return;
			}
			m_writeOffset = nWriteOffset;
		}

		void GetLine(const char*& szData, int& nBytesLength);

    private:
		CSeparatedStream(const CSeparatedStream & orig){}
		CSeparatedStream & operator = (const CSeparatedStream & right) { return *this; }

		bool AddBytesAndReallocate(const int numberOfBytes);
		inline bool IsDelimChar(char data){
			return data == m_delim;
		}

	private:
		char m_stackData[SEPDSTREAM_STACK_ALLOC_SIZE];
        char* m_data;
		int m_size;
		int m_writeOffset;
		int m_readOffset;
		bool m_bCopyData;
		char m_delim;
		char m_enclosure_delim;
		char m_sepdType;
		bool m_moreData;
};

}

#endif /* SEPARATEDSTREAM_H */
