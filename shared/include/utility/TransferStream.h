
#ifndef __TRANSFERSTREAM_H__
#define __TRANSFERSTREAM_H__

#include "Common.h"
#include "StreamDataType.h"
#ifndef NO_TEMPLATE
#include <string>
#include <vector>
#include <set>
#include <list>
#endif

#include "SeparatedStream.h"

namespace util
{

// Arbitrary size, just picking something likely to be larger than most packets
#define TRANSFERSTREAM_STACK_ALLOCA_SIZE 256

	/**
	 * This macro transforms a bit in byte
	 * @param x Transform a bit to a byte
	 */
#define TS_BITS_TO_BYTES(x) (((x)+7)>>3)

#define TS_BYTES_TO_BITS(x) (x<<3)

#define TS_STRING_MAX_SIZE 65535

#define TS_STRING_DELIM ','

	/**
	 * @brief Packets encoding and decoding facilities
	 *
	 * Helper class to encode and decode packets.
	 *
	 */

	class SHARED_DLL_DECL CTransferStream
	{
	public:
		/**
		 * Default Constructor
		 */
		CTransferStream();
		/**
		 * Preallocate some memory for the construction of the packet
		 * @param initByteSize the amount of byte to pre-allocate.
		 */
		CTransferStream(uint32_t initByteSize);

		/**
		 * Initialize the CStoreStream object using data from the network.
		 * Set copyData to true if you want to make an internal copy of
		 * the data you are passing. You can then Write and do all other
		 * operations Set it to false if you want to just use a pointer to
		 * the data you are passing, in order to save memory and speed.
		 * You should only then do read operations.
		 * @param data An array of bytes.
		 * @param lengthInBytes Size of the @em _data.
		 * @param copyData Does a copy of the input data.
		 */
		CTransferStream(const char* data, unsigned int lengthInBytes, bool copyData);

		CTransferStream(const std::string& data, bool copyData);

		CTransferStream(const CTransferStream& orig);
		/**
		 * Destructor
		 */
		~CTransferStream();
		/**
		 * Reset the stream for reuse
		 */
		void Reset(void);
		//////////////////////////////////////////////////////////////////////////
		inline void Serialize(const char input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_CHAR;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_CHAR_NULL;
			}
		}

		inline void Serialize(const bool input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_BOOL;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_BOOL_NULL;
			}
		}

		inline void Serialize(const uint8_t input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_UINT8;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_UINT8_NULL;
			}
		}

		inline void Serialize(const int8_t input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_INT8;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_INT8_NULL;
			}
		}

		inline void Serialize(const uint16_t input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_UINT16;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_UINT16_NULL;
			}
		}

		inline void Serialize(const int16_t input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_INT16;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_INT16_NULL;
			}
		}

		inline void Serialize(const uint32_t input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_UINT32;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_UINT32_NULL;
			}
		}

		inline void Serialize(const int32_t input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_INT32;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_INT32_NULL;
			}
		}

		inline void Serialize(const uint64_t input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_UINT64;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_UINT64_NULL;
			}
		}

		inline void Serialize(const int64_t input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_INT64;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_INT64_NULL;
			}
		}

		inline void Serialize(const float input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_FLOAT;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_FLOAT_NULL;
			}
		}

		inline void Serialize(const double input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_DOUBLE;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_DOUBLE_NULL;
			}
		}

		inline void Serialize(const char input[], bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_C_STRING;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_C_STRING_NULL;
			}
		}

#ifndef NO_TEMPLATE
		inline void Serialize(const std::string& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_STD_STRING;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_STD_STRING_NULL;
			}
		}
		//////////////////////////////////////////////////////////////////////////
		inline void Serialize(const std::vector<bool>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_VECTOR_BOOL;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_VECTOR_BOOL_NULL;
			}
		}

		inline void Serialize(const std::vector<uint8_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_VECTOR_UINT8;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_VECTOR_UINT8_NULL;
			}

		}

		inline void Serialize(const std::vector<int8_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_VECTOR_INT8;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_VECTOR_INT8_NULL;
			}
		}

		inline void Serialize(const std::vector<uint16_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_VECTOR_UINT16;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_VECTOR_UINT16_NULL;
			}
		}

		inline void Serialize(const std::vector<int16_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_VECTOR_INT16;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_VECTOR_INT16_NULL;
			}
		}

		inline void Serialize(const std::vector<uint32_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_VECTOR_UINT32;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_VECTOR_UINT32_NULL;
			}

		}

		inline void Serialize(const std::vector<int32_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_VECTOR_INT32;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_VECTOR_INT32_NULL;
			}
		}

		inline void Serialize(const std::vector<uint64_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_VECTOR_UINT64;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_VECTOR_UINT64_NULL;
			}
		}

		inline void Serialize(const std::vector<int64_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_VECTOR_INT64;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_VECTOR_INT64_NULL;
			}
		}

		inline void Serialize(const std::vector<float>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_VECTOR_FLOAT;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_VECTOR_FLOAT_NULL;
			}
		}

		inline void Serialize(const std::vector<double>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_VECTOR_DOUBLE;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_VECTOR_DOUBLE_NULL;
			}
		}

		inline void Serialize(const std::vector<std::string>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_VECTOR_STRING;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_VECTOR_STRING_NULL;
			}
		}

//////////////////////////////////////////////////////////////////////////
		inline void Serialize(const std::set<bool>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_SET_BOOL;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_SET_BOOL_NULL;
			}
		}

		inline void Serialize(const std::set<uint8_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_SET_UINT8;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_SET_UINT8_NULL;
			}
		}

		inline void Serialize(const std::set<int8_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_SET_INT8;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_SET_INT8_NULL;
			}
		}

		inline void Serialize(const std::set<uint16_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_SET_UINT16;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_SET_UINT16_NULL;
			}
		}

		inline void Serialize(const std::set<int16_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_SET_INT16;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_SET_INT16_NULL;
			}
		}

		inline void Serialize(const std::set<uint32_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_SET_UINT32;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_SET_UINT32_NULL;
			}
		}

		inline void Serialize(const std::set<int32_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_SET_INT32;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_SET_INT32_NULL;
			}
		}

		inline void Serialize(const std::set<uint64_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_SET_UINT64;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_SET_UINT64_NULL;
			}
		}

		inline void Serialize(const std::set<int64_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_SET_INT64;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_SET_INT64_NULL;
			}
		}

		inline void Serialize(const std::set<float>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_SET_FLOAT;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_SET_FLOAT_NULL;
			}
		}

		inline void Serialize(const std::set<double>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_SET_DOUBLE;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_SET_DOUBLE_NULL;
			}
		}

		inline void Serialize(const std::set<std::string>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_SET_STRING;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_SET_STRING_NULL;
			}
		}
//////////////////////////////////////////////////////////////////////////
		inline void Serialize(const std::list<bool>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_LIST_BOOL;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_LIST_BOOL_NULL;
			}
		}

		inline void Serialize(const std::list<uint8_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_LIST_UINT8;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_LIST_UINT8_NULL;
			}
		}

		inline void Serialize(const std::list<int8_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_LIST_INT8;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_LIST_INT8_NULL;
			}
		}

		inline void Serialize(const std::list<uint16_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_LIST_UINT16;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_LIST_UINT16_NULL;
			}
		}

		inline void Serialize(const std::list<int16_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_LIST_INT16;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_LIST_INT16_NULL;
			}
		}

		inline void Serialize(const std::list<uint32_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_LIST_UINT32;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_LIST_UINT32_NULL;
			}
		}

		inline void Serialize(const std::list<int32_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_LIST_INT32;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_LIST_INT32_NULL;
			}
		}

		inline void Serialize(const std::list<uint64_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_LIST_UINT64;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_LIST_UINT64_NULL;
			}
		}

		inline void Serialize(const std::list<int64_t>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_LIST_INT64;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_LIST_INT64_NULL;
			}
		}

		inline void Serialize(const std::list<float>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_LIST_FLOAT;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_LIST_FLOAT_NULL;
			}
		}

		inline void Serialize(const std::list<double>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_LIST_DOUBLE;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_LIST_DOUBLE_NULL;
			}
		}

		inline void Serialize(const std::list<std::string>& input, bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_LIST_STRING;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_LIST_STRING_NULL;
			}
		}
//////////////////////////////////////////////////////////////////////////
		template<size_t nSize>
		void Serialize(bool (&input)[nSize], bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_ARRAY_BOOL;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_ARRAY_BOOL_NULL;
			}
		}

		template<size_t nSize>
		void Serialize(uint8_t (&input)[nSize], bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_ARRAY_UINT8;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_ARRAY_UINT8_NULL;
			}
		}

		template<size_t nSize>
		void Serialize(int8_t (&input)[nSize], bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_ARRAY_INT8;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_ARRAY_INT8_NULL;
			}
		}

		template<size_t nSize>
		void Serialize(uint16_t (&input)[nSize], bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_ARRAY_UINT16;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_ARRAY_UINT16_NULL;
			}
		}

		template<size_t nSize>
		void Serialize(int16_t (&input)[nSize], bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_ARRAY_INT16;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_ARRAY_INT16_NULL;
			}
		}

		template<size_t nSize>
		void Serialize(uint32_t (&input)[nSize], bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_ARRAY_UINT32;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_ARRAY_UINT32_NULL;
			}
		}

		template<size_t nSize>
		void Serialize(int32_t (&input)[nSize], bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_ARRAY_INT32;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_ARRAY_INT32_NULL;
			}
		}

		template<size_t nSize>
		void Serialize(uint64_t (&input)[nSize], bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_ARRAY_UINT64;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_ARRAY_UINT64_NULL;
			}
		}

		template<size_t nSize>
		void Serialize(int64_t (&input)[nSize], bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_ARRAY_INT64;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_ARRAY_INT64_NULL;
			}
		}

		template<size_t nSize>
		void Serialize(float (&input)[nSize], bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_ARRAY_FLOAT;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_ARRAY_FLOAT_NULL;
			}
		}

		template<size_t nSize>
		void Serialize(double (&input)[nSize], bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_ARRAY_DOUBLE;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_ARRAY_DOUBLE_NULL;
			}
		}

		template<size_t nSize>
		void Serialize(std::string (&input)[nSize], bool bChange) {
			if(bChange) {
				*this << (uint8_t)STREAM_DATA_ARRAY_STRING;
				*this << input;
			} else {
				*this << (uint8_t)STREAM_DATA_ARRAY_STRING_NULL;
			}
		}

		template<class T>
		void Parse(T& ouput) {
			uint8_t nType = STREAM_DATA_NIL;
			*this >> nType;
			if(IsUpdateType(nType)) {
				ReadByType(ouput, nType);
			}
		}
#endif
		//////////////////////////////////////////////////////////////////////////
		void ReadToTypeString(std::vector<CTypeString>& outStrings) {
			uint8_t nThisType = STREAM_DATA_NIL;
			while(TS_BITS_TO_BYTES(this->GetNumberOfUnreadBits()) > 0)
			{
				outStrings.resize(outStrings.size() + 1);
				CTypeString& typeString = outStrings.back();

				*this >> nThisType;
				typeString.m_u8Type = nThisType;

				if(nThisType > STREAM_DATA_NIL && nThisType < STREAM_DATA_SIZE) {
					(this->*s_typeOperators[nThisType].m_pReadToString)(typeString.m_str);
				} else {
					assert(false);
				}
			}
		}

		void ReadToTypeString(CTypeString& outString) {
			uint8_t nThisType = STREAM_DATA_NIL;
			*this >> nThisType;
			outString.m_u8Type = nThisType;

			if(nThisType > STREAM_DATA_NIL && nThisType < STREAM_DATA_SIZE) {
				(this->*s_typeOperators[nThisType].m_pReadToString)(outString.m_str);
			} else {
				assert(false);
			}
		}

		void WriteFromTypeString(const char* input, int length, uint8_t u8Type) {

			if(NULL == input || length < 1) {
				*this << ToIgnoreType(u8Type);
				return;
			} else {
				*this << u8Type;
			}

			if(u8Type > STREAM_DATA_NIL && u8Type < STREAM_DATA_SIZE) {
				(this->*s_typeOperators[u8Type].m_pWriteFromString)(input, length);
			} else {
				assert(false);
			}
		}
		//////////////////////////////////////////////////////////////////////////
		void ReadCharToString(std::string& output);

		void ReadBoolToString(std::string& output);

		void ReadUInt8ToString(std::string& output);

		void ReadUInt16ToString(std::string& output);

		void ReadUInt32ToString(std::string& output);

		void ReadUInt64ToString(std::string& output);

		void ReadInt8ToString(std::string& output);

		void ReadInt16ToString(std::string& output);

		void ReadInt32ToString(std::string& output);

		void ReadInt64ToString(std::string& output);

		void ReadFloatToString(std::string& output);

		void ReadDoubleToString(std::string& output);

		void ReadStringToString(std::string& output);

		void ReadBoolSetToString(std::string& output);

		void ReadUInt8SetToString(std::string& output);

		void ReadInt8SetToString(std::string& output);

		void ReadUInt16SetToString(std::string& output);

		void ReadInt16SetToString(std::string& output);

		void ReadUInt32SetToString(std::string& output);

		void ReadInt32SetToString(std::string& output);

		void ReadUInt64SetToString(std::string& output);

		void ReadInt64SetToString(std::string& output);

		void ReadFloatSetToString(std::string& output);

		void ReadDoubleSetToString(std::string& output);

		void ReadStringSetToString(std::string& output);
		//////////////////////////////////////////////////////////////////////////
		void WriteCharFromString(const char* input, int length);

		void WriteBoolFromString(const char* input, int length);

		void WriteUInt8FromString(const char* input, int length);

		void WriteUInt16FromString(const char* input, int length);

		void WriteUInt32FromString(const char* input, int length);

		void WriteUInt64FromString(const char* input, int length);

		void WriteInt8FromString(const char* input, int length);

		void WriteInt16FromString(const char* input, int length);

		void WriteInt32FromString(const char* input, int length);

		void WriteInt64FromString(const char* input, int length);

		void WriteFloatFromString(const char* input, int length);

		void WriteDoubleFromString(const char* input, int length);

		void WriteStringFromString(const char* input, int length);

		void WriteBoolSetFromString(const char* input, int length);

		void WriteUInt8SetFromString(const char* input, int length);

		void WriteInt8SetFromString(const char* input, int length);

		void WriteUInt16SetFromString(const char* input, int length);

		void WriteInt16SetFromString(const char* input, int length);

		void WriteUInt32SetFromString(const char* input, int length);

		void WriteInt32SetFromString(const char* input, int length);

		void WriteUInt64SetFromString(const char* input, int length);

		void WriteInt64SetFromString(const char* input, int length);

		void WriteFloatSetFromString(const char* input, int length);

		void WriteDoubleSetFromString(const char* input, int length);

		void WriteStringSetFromString(const char* input, int length);

		//////////////////////////////////////////////////////////////////////////
		/**
		 * Write the native types to the end of the buffer
		 * without any compression mechanism.
		 * @param input The data
		 */
		CTransferStream& operator<<(const char input);
		/**
		 * Write the native types to the end of the buffer
		 * without any compression mechanism.
		 * @param input The data
		 */
		CTransferStream& operator<<(const bool input);
		/**
		 * Write the native types to the end of the buffer
		 * without any compression mechanism.
		 * @param input The data
		 */
		CTransferStream& operator<<(const uint8_t input);
		/**
		 * Write the native types to the end of the buffer
		 * without any compression mechanism.
		 * @param input The data
		 */
		CTransferStream& operator<<(const int8_t input);
		/**
		 * Write the native types to the end of the buffer
		 * without any compression mechanism.
		 * @param input The data
		 */
		CTransferStream& operator<<(const uint16_t input);
		/**
		 * Write the native types to the end of the buffer
		 * without any compression mechanism.
		 * @param input The data
		 */
		CTransferStream& operator<<(const int16_t input);
		/**
		 * Write the native types to the end of the buffer
		 * without any compression mechanism.
		 * @param input The data
		 */
		CTransferStream& operator<<(const uint32_t input);
		/**
		 * Write the native types to the end of the buffer
		 * without any compression mechanism.
		 * @param input The data
		 */
		CTransferStream& operator<<(const int32_t input);

#ifndef NO_INT64
		/**
		 * Write the native types to the end of the buffer
		 * without any compression mechanism.
		 * @param input The data
		 */
		CTransferStream& operator<<(const uint64_t input);
		/**
		 * Write the native types to the end of the buffer
		 * without any compression mechanism.
		 * @param input The data
		 */
		CTransferStream& operator<<(const int64_t input);
#endif

		/**
		 * Write the native types to the end of the buffer
		 * without any compression mechanism.
		 * @param input The data
		 */
		CTransferStream& operator<<(const float input);
		/**
		 * Write the native types to the end of the buffer
		 * without any compression mechanism.
		 * @param input The data
		 */
		CTransferStream& operator<<(const double input);
		/**
		 * Write an array or casted stream. It is supposed to
		 * be raw data. It is also not possible to deal with endian problem
		 * @param input a byte buffer
		 * @param numberOfBytes the size of the byte buffer
		 */
		void WriteBytes(const char* input, const int numberOfBytes);
        /**
         * Write multi bytes string
         * @param input c like string, end by '\0'
         */
        CTransferStream& operator<<(const char input[]);

#ifndef NO_TEMPLATE
		/**
		 * Write standard string
		 * @param input the standard lib string
		 */
		CTransferStream& operator<<(const std::string& input);
		/**
		 * Write standard vector
		 * @param input the standard lib vector
		 */
		template<class T>
		CTransferStream& operator<<(const std::vector<T>& input);
		/**
		 * Write standard set
		 * @param input the standard lib set
		 */
		template<class T>
		CTransferStream& operator<<(const std::set<T>& input);
		/**
		 * Write standard list
		 * @param input the standard lib list
		 */
		template<class T>
		CTransferStream& operator<<(const std::list<T>& input);
		/**
		 * Write array
		 * @param input an array set
		 */
		template<class T, size_t nSize>
		CTransferStream& operator<<(const T (&input)[nSize]);
#endif

//////////////////////////////////////////////////////////////////////////
		void ReadByType(char& ouput, uint8_t nType);

		void ReadByType(bool& ouput, uint8_t nType);

		void ReadByType(uint8_t& output, uint8_t nType);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		void ReadByType(int8_t& output, uint8_t nType);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		void ReadByType(uint16_t& output, uint8_t nType);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		void ReadByType(int16_t& output, uint8_t nType);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		void ReadByType(uint32_t& output, uint8_t nType);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		void ReadByType(int32_t& output, uint8_t nType);

#ifndef NO_INT64
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		void ReadByType(uint64_t& output, uint8_t nType);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		void ReadByType(int64_t& output, uint8_t nType);
#endif
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		void ReadByType(float& output, uint8_t nType);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		void ReadByType(double& output, uint8_t nType);

#ifndef NO_TEMPLATE
		        /**
         * Read standard lib string
         * @param output The c++ standard lib string
         */
		inline void ReadByType(std::string& output, uint8_t u8Type) {
			if(u8Type > STREAM_DATA_NIL && u8Type < STREAM_DATA_SIZE) {
				(this->*s_typeOperators[u8Type].m_pReadToString)(output);
			} else {
				assert(false);
			}
		}
        /**
		 * Read same c like string
		 * @param output The array buffer
		 */
		template<size_t nSize>
        void ReadByType(char (&output)[nSize], uint8_t nType) {

			assert(IsStringType(nType));

			if(nSize < 1) {
				uint16_t len = 0;
				*this >> len;
				IgnoreBits(TS_BYTES_TO_BITS(len));
				return;
			}

			uint16_t len = 0;
			*this >> len;

			if(len < nSize) {
				ReadBytes(output, len);
				output[len] = '\0';
			} else {
				int nReadLen = nSize - 1;
				ReadBytes(output, nReadLen);
				output[nReadLen] = '\0';
				int nLeaveLen = len - nReadLen;
				if(nLeaveLen > 0) {
					IgnoreBits(TS_BYTES_TO_BITS(nLeaveLen));
				}
			}
		}
//////////////////////////////////////////////////////////////////////////
		/**
		 * Read The standard vector of bool
		 * @param output The read value.
		 */
		void ReadByType(std::vector<bool>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}


				output.resize(length, false);
				bool temp = false;
				for(int i = 0; i < length; ++i) {
					this->ReadByType(temp, nInnerType);
					output[i] = temp;
				}
			}
		}
		/**
		 * Read The standard vector of uint8_t
		 * @param output The read value.
		 */
		void ReadByType(std::vector<uint8_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				output.resize(length);
				for(int i = 0; i < length; ++i) {
					this->ReadByType(output[i], nInnerType);
				}
			}
		}
		/**
		 * Read The standard vector of int8_t
		 * @param output The read value.
		 */
		void ReadByType(std::vector<int8_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				output.resize(length);
				for(int i = 0; i < length; ++i) {
					this->ReadByType(output[i], nInnerType);
				}
			}
		}
		/**
		 * Read The standard vector of uint16_t
		 * @param output The read value.
		 */
		void ReadByType(std::vector<uint16_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				output.resize(length);
				for(int i = 0; i < length; ++i) {
					this->ReadByType(output[i], nInnerType);
				}
			}
		}
		/**
		 * Read The standard vector of int16_t
		 * @param output The read value.
		 */
		void ReadByType(std::vector<int16_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				output.resize(length);
				for(int i = 0; i < length; ++i) {
					this->ReadByType(output[i], nInnerType);
				}
			}
		}
		/**
		 * Read The standard vector of uint32_t
		 * @param output The read value.
		 */
		void ReadByType(std::vector<uint32_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				output.resize(length);
				for(int i = 0; i < length; ++i) {
					this->ReadByType(output[i], nInnerType);
				}
			}
		}
		/**
		 * Read The standard vector of int32_t
		 * @param output The read value.
		 */
		void ReadByType(std::vector<int32_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				output.resize(length);
				for(int i = 0; i < length; ++i) {
					this->ReadByType(output[i], nInnerType);
				}
			}
		}
#ifndef NO_INT64
		/**
		 * Read The standard vector of uint64_t
		 * @param output The read value.
		 */
		void ReadByType(std::vector<uint64_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				output.resize(length);
				for(int i = 0; i < length; ++i) {
					this->ReadByType(output[i], nInnerType);
				}
			}
		}
		/**
		 * Read The standard vector of int64_t
		 * @param output The read value.
		 */
		void ReadByType(std::vector<int64_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				output.resize(length);
				for(int i = 0; i < length; ++i) {
					this->ReadByType(output[i], nInnerType);
				}
			}
		}
#endif
		/**
		 * Read The standard vector of float
		 * @param output The read value.
		 */
		void ReadByType(std::vector<float>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				output.resize(length);
				for(int i = 0; i < length; ++i) {
					this->ReadByType(output[i], nInnerType);
				}
			}
		}
		/**
		 * Read The standard vector of double
		 * @param output The read value.
		 */
		void ReadByType(std::vector<double>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				output.resize(length);
				for(int i = 0; i < length; ++i) {
					this->ReadByType(output[i], nInnerType);
				}
			}
		}
		/**
		 * Read The standard vector of std::string
		 * @param output The read value.
		 */
		void ReadByType(std::vector<std::string>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				output.resize(length);
				for(int i = 0; i < length; ++i) {
					this->ReadByType(output[i], nInnerType);
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////
		/**
		 * Read The standard set of bool
		 * @param output The c++ standard set
		 */
		void ReadByType(std::set<bool>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				bool temp = false;
				for(int i = 0; i < length; ++i) {
					this->ReadByType(temp, nInnerType);
					output.insert(temp);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard set
		 */
		void ReadByType(std::set<uint8_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				uint8_t temp = 0;
				for(int i = 0; i < length; ++i) {
					this->ReadByType(temp, nInnerType);
					output.insert(temp);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard set
		 */
		void ReadByType(std::set<int8_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				int8_t temp = 0;
				for(int i = 0; i < length; ++i) {
					this->ReadByType(temp, nInnerType);
					output.insert(temp);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard set
		 */
		void ReadByType(std::set<uint16_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				uint16_t temp = 0;
				for(int i = 0; i < length; ++i) {
					this->ReadByType(temp, nInnerType);
					output.insert(temp);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard set
		 */
		void ReadByType(std::set<int16_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				int16_t temp = 0;
				for(int i = 0; i < length; ++i) {
					this->ReadByType(temp, nInnerType);
					output.insert(temp);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard set
		 */
		void ReadByType(std::set<uint32_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				uint32_t temp = 0;
				for(int i = 0; i < length; ++i) {
					this->ReadByType(temp, nInnerType);
					output.insert(temp);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard set
		 */
		void ReadByType(std::set<int32_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				int32_t temp = 0;
				for(int i = 0; i < length; ++i) {
					this->ReadByType(temp, nInnerType);
					output.insert(temp);
				}
			}
		}
#ifndef NO_INT64
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard set
		 */
		void ReadByType(std::set<uint64_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				uint64_t temp = 0;
				for(int i = 0; i < length; ++i) {
					this->ReadByType(temp, nInnerType);
					output.insert(temp);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard set
		 */
		void ReadByType(std::set<int64_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				int64_t temp = 0;
				for(int i = 0; i < length; ++i) {
					this->ReadByType(temp, nInnerType);
					output.insert(temp);
				}
			}
		}
#endif
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard set
		 */
		void ReadByType(std::set<float>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				float temp = 0.0f;
				for(int i = 0; i < length; ++i) {
					this->ReadByType(temp, nInnerType);
					output.insert(temp);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard set
		 */
		void ReadByType(std::set<double>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				double temp = 0.;
				for(int i = 0; i < length; ++i) {
					this->ReadByType(temp, nInnerType);
					output.insert(temp);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard set
		 */
		void ReadByType(std::set<std::string>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				std::string temp;
				for(int i = 0; i < length; ++i) {
					this->ReadByType(temp, nInnerType);
					output.insert(temp);
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard list
		 */
		void ReadByType(std::list<bool>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				for(int i = 0; i < length; ++i) {
					output.resize(output.size() + 1);
					this->ReadByType(output.back(), nInnerType);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard list
		 */
		void ReadByType(std::list<uint8_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				for(int i = 0; i < length; ++i) {
					output.resize(output.size() + 1);
					this->ReadByType(output.back(), nInnerType);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard list
		 */
		void ReadByType(std::list<int8_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				for(int i = 0; i < length; ++i) {
					output.resize(output.size() + 1);
					this->ReadByType(output.back(), nInnerType);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard list
		 */
		void ReadByType(std::list<uint16_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				for(int i = 0; i < length; ++i) {
					output.resize(output.size() + 1);
					this->ReadByType(output.back(), nInnerType);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard list
		 */
		void ReadByType(std::list<int16_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				for(int i = 0; i < length; ++i) {
					output.resize(output.size() + 1);
					this->ReadByType(output.back(), nInnerType);
				}
			}
		}
		/**
		 * Read the native types from the front of the buffer
		 * @param output The c++ standard list
		 */
		void ReadByType(std::list<uint32_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				for(int i = 0; i < length; ++i) {
					output.resize(output.size() + 1);
					this->ReadByType(output.back(), nInnerType);
				}
			}
		}
		/**
		 * Read the int32_t list from the front of the buffer
		 * @param output The c++ standard list
		 */
		void ReadByType(std::list<int32_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				for(int i = 0; i < length; ++i) {
					output.resize(output.size() + 1);
					this->ReadByType(output.back(), nInnerType);
				}
			}
		}
#ifndef NO_INT64
		/**
		 * Read the uint64_t list from the front of the buffer
		 * @param output The c++ standard list
		 */
		void ReadByType(std::list<uint64_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				for(int i = 0; i < length; ++i) {
					output.resize(output.size() + 1);
					this->ReadByType(output.back(), nInnerType);
				}
			}
		}
		/**
		 * Read the int64_t list from the front of the buffer
		 * @param output The c++ standard list
		 */
		void ReadByType(std::list<int64_t>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				for(int i = 0; i < length; ++i) {
					output.resize(output.size() + 1);
					this->ReadByType(output.back(), nInnerType);
				}
			}
		}
#endif
		/**
		 * Read the float types from the front of the buffer
		 * @param output The c++ standard list
		 */
		void ReadByType(std::list<float>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				for(int i = 0; i < length; ++i) {
					output.resize(output.size() + 1);
					this->ReadByType(output.back(), nInnerType);
				}
			}
		}
		/**
		 * Read the double types from the front of the buffer
		 * @param output The c++ standard list
		 */
		void ReadByType(std::list<double>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				for(int i = 0; i < length; ++i) {
					output.resize(output.size() + 1);
					this->ReadByType(output.back(), nInnerType);
				}
			}
		}
		/**
		 * Read the std::string types from the front of the buffer
		 * @param output The c++ standard list
		 */
		void ReadByType(std::list<std::string>& output, uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(!output.empty()) {
				output.clear();
			}

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;
				if(length < 1) {
					return;
				}

				for(int i = 0; i < length; ++i) {
					output.resize(output.size() + 1);
					this->ReadByType(output.back(), nInnerType);
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////
		/**
		 * Read an Array of bool from the front of the buffer
		 * @param output An array
		 */
		template<size_t nSize>
		void ReadByType(bool (&output)[nSize], uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;

				if(length > (int32_t)nSize) {
					for(int32_t i = 0; i < (int32_t)nSize; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
					int32_t leaveSize = length - (int32_t)nSize;
					if(leaveSize > 0) {
						bool temp = false;
						while(leaveSize-- > 0) {
							this->ReadByType(temp, nInnerType);
						}
					}
				} else {
					for(int32_t i = 0; i < length; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
				}
			}
		}
		/**
		 * Read an Array of uint8_t from the front of the buffer
		 * @param output An array
		 */
		template<size_t nSize>
		void ReadByType(uint8_t (&output)[nSize], uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;

				if(length > (int32_t)nSize) {
					for(int32_t i = 0; i < (int32_t)nSize; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
					int32_t leaveSize = length - (int32_t)nSize;
					if(leaveSize > 0) {
						uint8_t temp = 0;
						while(leaveSize-- > 0) {
							this->ReadByType(temp, nInnerType);
						}
					}
				} else {
					for(int32_t i = 0; i < length; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
				}
			}
		}
		/**
		 * Read an Array of int8_t from the front of the buffer
		 * @param output An array
		 */
		template<size_t nSize>
		void ReadByType(int8_t (&output)[nSize], uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;

				if(length > (int32_t)nSize) {
					for(int32_t i = 0; i < (int32_t)nSize; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
					int32_t leaveSize = length - (int32_t)nSize;
					if(leaveSize > 0) {
						int8_t temp = 0;
						while(leaveSize-- > 0) {
							this->ReadByType(temp, nInnerType);
						}
					}
				} else {
					for(int32_t i = 0; i < length; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
				}
			}
		}
		/**
		 * Read an Array of uint16_t from the front of the buffer
		 * @param output An array
		 */
		template<size_t nSize>
		void ReadByType(uint16_t (&output)[nSize], uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;

				if(length > (int32_t)nSize) {
					for(int32_t i = 0; i < (int32_t)nSize; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
					int32_t leaveSize = length - (int32_t)nSize;
					if(leaveSize > 0) {
						uint16_t temp = 0;
						while(leaveSize-- > 0) {
							this->ReadByType(temp, nInnerType);
						}
					}
				} else {
					for(int32_t i = 0; i < length; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
				}
			}
		}
		/**
		 * Read an Array of int16_t from the front of the buffer
		 * @param output An array
		 */
		template<size_t nSize>
		void ReadByType(int16_t (&output)[nSize], uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;

				if(length > (int32_t)nSize) {
					for(int32_t i = 0; i < (int32_t)nSize; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
					int32_t leaveSize = length - (int32_t)nSize;
					if(leaveSize > 0) {
						int16_t temp = 0;
						while(leaveSize-- > 0) {
							this->ReadByType(temp, nInnerType);
						}
					}
				} else {
					for(int32_t i = 0; i < length; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
				}
			}
		}
		/**
		 * Read an Array of uint32_t from the front of the buffer
		 * @param output An array
		 */
		template<size_t nSize>
		void ReadByType(uint32_t (&output)[nSize], uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;

				if(length > (int32_t)nSize) {
					for(int32_t i = 0; i < (int32_t)nSize; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
					int32_t leaveSize = length - (int32_t)nSize;
					if(leaveSize > 0) {
						uint32_t temp = 0;
						while(leaveSize-- > 0) {
							this->ReadByType(temp, nInnerType);
						}
					}
				} else {
					for(int32_t i = 0; i < length; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
				}
			}
		}
		/**
		 * Read an Array of int32_t from the front of the buffer
		 * @param output An array
		 */
		template<size_t nSize>
		void ReadByType(int32_t (&output)[nSize], uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;

				if(length > (int32_t)nSize) {
					for(int32_t i = 0; i < (int32_t)nSize; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
					int32_t leaveSize = length - (int32_t)nSize;
					if(leaveSize > 0) {
						int32_t temp = 0;
						while(leaveSize-- > 0) {
							this->ReadByType(temp, nInnerType);
						}
					}
				} else {
					for(int32_t i = 0; i < length; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
				}
			}
		}
#ifndef NO_INT64
		/**
		 * Read an Array of uint64_t from the front of the buffer
		 * @param output An array
		 */
		template<size_t nSize>
		void ReadByType(uint64_t (&output)[nSize], uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;

				if(length > (int32_t)nSize) {
					for(int32_t i = 0; i < (int32_t)nSize; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
					int32_t leaveSize = length - (int32_t)nSize;
					if(leaveSize > 0) {
						uint64_t temp = 0;
						while(leaveSize-- > 0) {
							this->ReadByType(temp, nInnerType);
						}
					}
				} else {
					for(int32_t i = 0; i < length; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
				}
			}
		}
		/**
		 * Read an Array of int64_t from the front of the buffer
		 * @param output An array
		 */
		template<size_t nSize>
		void ReadByType(int64_t (&output)[nSize], uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;

				if(length > (int32_t)nSize) {
					for(int32_t i = 0; i < (int32_t)nSize; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
					int32_t leaveSize = length - (int32_t)nSize;
					if(leaveSize > 0) {
						int64_t temp = 0;
						while(leaveSize-- > 0) {
							this->ReadByType(temp, nInnerType);
						}
					}
				} else {
					for(int32_t i = 0; i < length; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
				}
			}
		}
#endif
		/**
		 * Read an Array of float from the front of the buffer
		 * @param output An array
		 */
		template<size_t nSize>
		void ReadByType(float (&output)[nSize], uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;

				if(length > (int32_t)nSize) {
					for(int32_t i = 0; i < (int32_t)nSize; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
					int32_t leaveSize = length - (int32_t)nSize;
					if(leaveSize > 0) {
						float temp = 0.0f;
						while(leaveSize-- > 0) {
							this->ReadByType(temp, nInnerType);
						}
					}
				} else {
					for(int32_t i = 0; i < length; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
				}
			}
		}
		/**
		 * Read an Array of double from the front of the buffer
		 * @param output An array
		 */
		template<size_t nSize>
		void ReadByType(double (&output)[nSize], uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;

				if(length > (int32_t)nSize) {
					for(int32_t i = 0; i < (int32_t)nSize; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
					int32_t leaveSize = length - (int32_t)nSize;
					if(leaveSize > 0) {
						double temp = 0.;
						while(leaveSize-- > 0) {
							this->ReadByType(temp, nInnerType);
						}
					}
				} else {
					for(int32_t i = 0; i < length; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
				}
			}
		}
		/**
		 * Read an Array of std::string from the front of the buffer
		 * @param output An array
		 */
		template<size_t nSize>
		void ReadByType(std::string (&output)[nSize], uint8_t nType) {

			assert(IsContainerType(nType));

			uint8_t nInnerType = GetInnerType(nType);

			if(IsStringType(nType)) {
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
			} else {

				int32_t length = 0;
				*this >> length;

				if(length > (int32_t)nSize) {
					for(int32_t i = 0; i < (int32_t)nSize; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
					int32_t leaveSize = length - (int32_t)nSize;
					if(leaveSize > 0) {
						uint16_t len = 0;
						while(leaveSize-- > 0) {
							*this >> len;
							IgnoreBits(TS_BYTES_TO_BITS(len));
						}
					}
				} else {
					for(int32_t i = 0; i < length; ++i) {
						this->ReadByType(output[i], nInnerType);
					}
				}
			}
		}
#endif

//////////////////////////////////////////////////////////////////////////
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		CTransferStream& operator>>(std::vector<bool>::reference output);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		CTransferStream& operator>>(bool& output);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		CTransferStream& operator>>(char& output);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		CTransferStream& operator>>(uint8_t& output);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		CTransferStream& operator>>(int8_t& output);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		CTransferStream& operator>>(uint16_t& output);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		CTransferStream& operator>>(int16_t& output);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		CTransferStream& operator>>(uint32_t& output);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		CTransferStream& operator>>(int32_t& output);

#ifndef NO_INT64
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		CTransferStream& operator>>(uint64_t& output);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		CTransferStream& operator>>(int64_t& output);
#endif
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		CTransferStream& operator>>(float& output);
		/**
		 * Read the native types from the front of the buffer
		 * @param output The read value.
		 */
		CTransferStream& operator>>(double& output);
		/**
		 * Read an array or casted stream of byte. The array
		 * is raw data. There is no automatic conversion on
		 * big endian arch
		 * @param output The result byte array. It should be larger than @em numberOfBytes.
		 * @param numberOfBytes The number of byte to read
		 */
		bool ReadBytes(char* output, const int numberOfBytes);
#ifndef NO_TEMPLATE
		/**
         * Read standard lib string
         * @param output The c++ standard lib string
         */
		CTransferStream& operator>>(std::string& output);
        /**
		 * Read same c like string
		 * @param output The array buffer
		 */
		template<size_t nSize>
        CTransferStream& operator>>(char (&output)[nSize]);
		/**
		 * Read standard vector
		 * @param output The c++ standard vector
		 */
		template<class T>
		CTransferStream& operator>>(std::vector<T>& output);
		/**
		 * Read standard set
		 * @param output The c++ standard set
		 */
		template<class T>
		CTransferStream& operator>>(std::set<T>& output);
		/**
		 * Read standard list
		 * @param output The c++ standard list
		 */
		template<class T>
		CTransferStream& operator>>(std::list<T>& output);
		/**
		 * Read array
		 * @param output The c array
		 */
		template<class T, size_t nSize>
		CTransferStream& operator>>(T (&output)[nSize]);
#endif
		/**
		 * Sets the read pointer back to the beginning of your data.
		 */
		void ResetReadPointer(void);
		/**
		* Sets the write pointer back to the beginning of your data.
		*/
		void ResetWritePointer(void);
		/**
		 * This is good to call when you are done with the stream to make
		 * sure you didn't leave any data left over void
		 */
		void AssertStreamEmpty(void);
		/**
		 * print to the standard output the state of the stream bit by bit
		 */
		void PrintBits(void) const;

		/**
		 * Ignore data we don't intend to read
		 * @param numberOfBits The number of bits to ignore
		 */
		void IgnoreBits(const int numberOfBits);

		/**
		 * Move the write pointer to a position on the array.
		 * @param offset the offset from the start of the array.
		 * @attention
		 * Dangerous if you don't know what you are doing!
		 *
		 */
		void SetWriteOffset(const uint32_t offset);
		/**
		 * Returns the length in bits of the stream
		 */
		uint32_t GetWriteOffset(void) const;
		/**
		 * Returns the length in bytes of the stream
		 */
		uint32_t GetNumberOfBytesUsed(void) const;
        /**
         * Move the read pointer to a position on the array.
         * @param offset
         */
        void SetReadOffset(const uint32_t offset);
		/**
		 * Returns the number of bits into the stream that we have read
		 */
		uint32_t GetReadOffset(void) const;

		/**
		 * Returns the number of bits left in the stream that haven't been read
		 */
		uint32_t GetNumberOfUnreadBits(void) const;
		/**
		 * Makes a copy of the internal data for you Data will point to
		 * the stream. Returns the length in bits of the stream. Partial
		 * bytes are left aligned
		 * @param _data the resulting byte copy of the internal state.
		 */
		int CopyData(unsigned char** _data) const;
		/**
		 * Set the stream to some initial data.  For internal use
		 * Partial bytes are left aligned
		 * @param input The data
		 * @param numberOfBits the number of bits set in the data buffer
		 */
		void SetData(const char* input, const int numberOfBits);
		/**
		 * Exposes the internal data.
		 * Partial bytes are left aligned.
		 * @return A pointer to the internal state
		 */
		const char* GetData(void) const;
		/**
		 * Write numberToWrite bits from the input source Right aligned
		 * data means in the case of a partial byte, the bits are aligned
		 * from the right (bit 0) rather than the left (as in the normal
		 * internal representation) You would set this to true when
		 * writing user data, and false when copying bit stream data, such
		 * as writing one bit stream to another
		 * @param input The data
		 * @param numberOfBitsToWrite The number of bits to write
		 * @param rightAlignedBits if true data will be right aligned
		 */
		void WriteBits(const unsigned char* input,
			int numberOfBitsToWrite, const bool rightAlignedBits = true);
		/**
		 * Align the bit stream to the byte boundary and then write the
		 * specified number of bits.  This is faster than WriteBits but
		 * wastes the bits to do the alignment and requires you to call
		 * ReadAlignedBits at the corresponding read position.
		 * @param input The data
		 * @param numberOfBytesToWrite The size of data.
		 */
		void WriteAlignedBytes(const unsigned char* input,
			const int numberOfBytesToWrite);
		/**
		 * Read bits, starting at the next aligned bits. Note that the
		 * modulus 8 starting offset of the sequence must be the same as
		 * was used with WriteBits. This will be a problem with packet
		 * coalescence unless you byte align the coalesced packets.
		 * @param output The byte array larger than @em numberOfBytesToRead
		 * @param numberOfBytesToRead The number of byte to read from the internal state
		 * @return true if there is enough byte.
		 */
		bool ReadAlignedBytes(unsigned char* output,
			const int numberOfBytesToRead);
		/**
		 * Align the next write and/or read to a byte boundary.  This can
		 * be used to 'waste' bits to byte align for efficiency reasons It
		 * can also be used to force coalesced bit streams to start on byte
		 * boundaries so so WriteAlignedBits and ReadAlignedBits both
		 * calculate the same offset when aligning.
		 */
		void AlignWriteToByteBoundary(void);
		/**
		 * Align the next write and/or read to a byte boundary.  This can
		 * be used to 'waste' bits to byte align for efficiency reasons It
		 * can also be used to force coalesced bit streams to start on byte
		 * boundaries so so WriteAlignedBits and ReadAlignedBits both
		 * calculate the same offset when aligning.
		 */
		void AlignReadToByteBoundary(void);

		/**
		 * Read numberOfBitsToRead bits to the output source
		 * alignBitsToRight should be set to true to convert internal
		 * bit stream data to user data It should be false if you used
		 * WriteBits with rightAlignedBits false
		 * @param output The resulting bits array
		 * @param numberOfBitsToRead The number of bits to read
		 * @param alignsBitsToRight if true bits will be right aligned.
		 * @return true if there is enough bits to read
		 */
		bool ReadBits(unsigned char* output, int numberOfBitsToRead,
			const bool alignBitsToRight = true);

		/**
		 * --- Low level functions ---
		 * These are for when you want to deal
		 * with bits and don't care about type checking
		 * Write a 0
		 */
		void Write0(void);
		/**
		 * --- Low level functions ---
		 * These are for when you want to deal
		 * with bits and don't care about type checking
		 * Write a 1
		 */
		void Write1(void);
		/**
		 * --- Low level functions ---
		 * These are for when you want to deal
		 * with bits and don't care about type checking
		 * Reads 1 bit and returns true if that bit is 1 and false if it is 0
		 */
		bool ReadBit(void);
		/**
		 * If we used the constructor version with copy data off, this
		 * makes sure it is set to on and the data pointed to is copied.
		 */
		void AssertCopyData(void);
		/**
		 * Use this if you pass a pointer copy to the constructor
		 * (copyData==false) and want to overallocate to prevent
		 * reallocation
		 */
		void SetNumberOfBitsAllocated(const uint32_t lengthInBits);

	private:

		/**
		 * Reallocates (if necessary) in preparation of writing
		 * numberOfBitsToWrite
		 */
		void AddBitsAndReallocate(const uint32_t writeBitSize);

		/**
		 * Number of bits currently used
		 */
		uint32_t m_writeOffset;
		/**
		 * Number of bits currently allocated
		 */
		uint32_t m_allocBitSize;
		/**
		 * Current readOffset
		 */
		uint32_t m_readOffset;
		/**
		 * array of byte storing the data.  Points to stackData or if is bigger than that then is allocated
		 */
		unsigned char* m_data;
		/**
		 * true if the internal buffer is copy of the data passed to the
		 * constructor
		 */
		bool m_copyData;
		/**
		 * Whether the local cup is little endian?
		 */
		bool m_isLittleEndian;
		/*
		* stack buffer
		*/
		unsigned char m_stackData[TRANSFERSTREAM_STACK_ALLOCA_SIZE];

	private:
		CTransferStream& operator = (const CTransferStream& right) { return *this; }

		struct TypeOperatorSet {
			void (util::CTransferStream::*m_pReadToString)(std::string& output);
			void (util::CTransferStream::*m_pWriteFromString)(const char* input, int length);
		};
		static bool s_bInitTypeOperator;
		static struct TypeOperatorSet s_typeOperators[STREAM_DATA_SIZE];

		void InitTypeOperator();
	};



#ifndef NO_TEMPLATE

	template<class T>
	CTransferStream& CTransferStream::operator<<(const std::vector<T>& input)
	{
		int32_t length = (int32_t)input.size();
		*this << length;

		for(int i = 0; i < length; ++i) {
			*this << (T)input[i];
		}
		return *this;
	}

	template<class T>
	CTransferStream& CTransferStream::operator<<(const std::set<T>& input)
	{
		int32_t length = (int32_t)input.size();
		*this << length;

		typename std::set<T>::const_iterator it = input.begin();
		for(; input.end() != it; ++it) {
			*this << *it;
		}
		return *this;
	}

	template<class T>
	CTransferStream& CTransferStream::operator<<(const std::list<T>& input)
	{
		int32_t length = (int32_t)input.size();
		*this << length;

		typename std::list<T>::const_iterator it = input.begin();
		for(; input.end() != it; ++it) {
			*this << *it;
		}
		return *this;
	}

	template<class T, size_t nSize>
	CTransferStream& CTransferStream::operator<<(const T (&input)[nSize])
	{
		int32_t length = (int32_t)nSize;
		*this << length;

		for(int i = 0; i < length; ++i) {
			*this << input[i];
		}
		return *this;
	}

	// read c chars
	template<size_t nSize>
    CTransferStream& CTransferStream::operator>>(char (&output)[nSize]) {
		if(nSize < 1) {
			uint16_t len = 0;
			*this >> len;
			IgnoreBits(TS_BYTES_TO_BITS(len));
			return *this;
		}

		uint16_t len = 0;
		*this >> len;

		if(len < nSize) {
			ReadBytes(output, len);
			output[len] = '\0';
		} else {
			int nReadLen = nSize - 1;
			ReadBytes(output, nReadLen);
			output[nReadLen] = '\0';
			int nLeaveLen = len - nReadLen;
			if(nLeaveLen > 0) {
				IgnoreBits(TS_BYTES_TO_BITS(nLeaveLen));
			}
		}
		return *this;
	}

	template<class T>
	CTransferStream& CTransferStream::operator>>(std::vector<T>& output)
	{
		if(!output.empty()) {
			output.clear();
		}

		int32_t length = 0;
		*this >> length;
		if(length < 1) {
			return *this;
		}

		output.resize(length);

        for(int i = 0; i < length; ++i) {
            *this >> output[i];
        }

		return *this;
	}

	template<class T>
	CTransferStream& CTransferStream::operator>>(std::set<T>& output)
	{
		if(!output.empty()) {
			output.clear();
		}

		int32_t length = 0;
		*this >> length;
		if(length < 1) {
			return *this;
		}

		T temp;
		for(int i = 0; i < length; ++i) {
			*this >> temp;
			output.insert(temp);
		}

		return *this;
	}

	template<class T>
	CTransferStream& CTransferStream::operator>>(std::list<T>& output)
	{
		if(!output.empty()) {
			output.clear();
		}

		int32_t length = 0;
		*this >> length;
		if(length < 1) {
			return *this;
		}

		for(int i = 0; i < length; ++i) {
			output.resize(output.size() + 1);
			*this >> output.back();
		}

		return *this;
	}

	template<class T, size_t nSize>
	CTransferStream& CTransferStream::operator>>(T (&output)[nSize])
	{

		int32_t length = 0;
		*this >> length;

		for(int i = 0; i < length; ++i) {
			*this >> output[i];
		}

		int32_t leave = length - nSize;
		if(leave > 0) {
			IgnoreBits(TS_BYTES_TO_BITS(leave * sizeof(T)));
		}
		return *this;
	}
#endif
}

#endif /* __TRANSFERSTREAM_H__ */
