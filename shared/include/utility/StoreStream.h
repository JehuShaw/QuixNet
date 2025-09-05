/*
 * File:   StoreStream.h
 * Author: Jehu Shaw
 *
 * Created on 2014_5_8, 14:09
 */

#ifndef STORESTREAM_H
#define STORESTREAM_H

#include "Common.h"
#include "StreamDataType.h"

namespace util {

// Arbitrary size
#define STORESTREAM_STACK_ALLOC_SIZE 64

#define STOREOBJECT_MIN_ALLOC_SIZE 6

#define STORESTREAM_STRING_DELIM ','

class CTransferStream;

class SHARED_DLL_DECL CStoreObject {
public:
	CStoreObject(int allocBytes = STOREOBJECT_MIN_ALLOC_SIZE);

	~CStoreObject();

	void WriteBytes(const char* input, const int numberOfBytes);

	const char* GetData()const { return m_data; }

	int GetLength()const { return m_length; }

private:
	CStoreObject(const CStoreObject& orig) {}
	CStoreObject& operator = (const CStoreObject& right) { return *this; }

	void AddBytesAndReallocate(const int numberOfBytes);

private:
	friend class CStoreStream;
	char* m_data;
	int m_size;
	int m_length;
	int m_preOffset;
	int m_nxtOffset;
};

class SHARED_DLL_DECL CStoreStream
{
	public:

		CStoreStream(int allocBytes = STORESTREAM_STACK_ALLOC_SIZE);

		CStoreStream(const char* szData, int nBytesLength, bool bCopyData);

		CStoreStream(const std::string& strData, bool bCopyData);

		CStoreStream(const CStoreStream& strData);

		CStoreStream(CTransferStream& inStream);

		~CStoreStream();

		inline void Clear()
		{
			CStoreObject* curPtr = ReadObjectType(m_objOffset);
			while(NULL != curPtr) {
				curPtr->~CStoreObject();
				curPtr = ReadObjectType(curPtr->m_preOffset);
			}
			m_objOffset = -1;
			m_writeOffset = 0;
			m_readOffset = 0;
			if(!this->m_bCopyData) {
				m_bCopyData = true;
				m_data = m_stackData;
				m_size = STORESTREAM_STACK_ALLOC_SIZE;
			}
		}

		CStoreStream & operator = (const CStoreStream & right);

		void Reset(const char* szData, int nBytesLength, bool bCopyData);

        inline void Reset(const std::string& strData, bool bCopyData)
        {
			Reset(strData.data(), strData.length(), bCopyData);
		}
		//////////////////////////////////////////////////////////////////////////
		void Parse(CTransferStream& inStream);

		void Serialize(CTransferStream& outStream) const;
		void Serialize(CTransferStream& outStream, const std::vector<bool>& updateFlags) const;
		//////////////////////////////////////////////////////////////////////////
		void ParseResetUpdate(CTransferStream& inStream);
		void ParseSetUpdate(CTransferStream& inStream);
		void SerializeResetUpdate(std::vector<CTypeString>& outStrings) const;
		void RecoverUpdate(const std::vector<CTypeString>& inStrings);
		//////////////////////////////////////////////////////////////////////////
		void ReadByType(CTransferStream& outStream) const;
		//void WriteFromTypeString(const std::vector<CTypeString>& inStrings);
		//void ReadToTypeString(std::vector<CTypeString>& outStrings);
		////////////////////////////////////////////////////////////////////////////
		//void WriteFromTypeString(const char* szInput, int length, uint8_t nDestType);
		//void WriteFromTypeString(const CTypeString& inString);
		//void ReadToTypeString(CTypeString& outString);
		//////////////////////////////////////////////////////////////////////////
		void WriteCharByType(CTransferStream& stream, uint8_t nType);

		void WriteBoolByType(CTransferStream& stream, uint8_t nType);

		void WriteUInt8ByType(CTransferStream& stream, uint8_t nType);

		void WriteUInt16ByType(CTransferStream& stream, uint8_t nType);

		void WriteUInt32ByType(CTransferStream& stream, uint8_t nType);

		void WriteUInt64ByType(CTransferStream& stream, uint8_t nType);

		void WriteInt8ByType(CTransferStream& stream, uint8_t nType);

		void WriteInt16ByType(CTransferStream& stream, uint8_t nType);

		void WriteInt32ByType(CTransferStream& stream, uint8_t nType);

		void WriteInt64ByType(CTransferStream& stream, uint8_t nType);

		void WriteFloatByType(CTransferStream& stream, uint8_t nType);

		void WriteDoubleByType(CTransferStream& stream, uint8_t nType);

		void WriteStringByType(CTransferStream& stream, uint8_t nType);

		void WriteBoolSetByType(CTransferStream& stream, uint8_t nType);

		void WriteUInt8SetByType(CTransferStream& stream, uint8_t nType);

		void WriteInt8SetByType(CTransferStream& stream, uint8_t nType);

		void WriteUInt16SetByType(CTransferStream& stream, uint8_t nType);

		void WriteInt16SetByType(CTransferStream& stream, uint8_t nType);

		void WriteUInt32SetByType(CTransferStream& stream, uint8_t nType);

		void WriteInt32SetByType(CTransferStream& stream, uint8_t nType);

		void WriteUInt64SetByType(CTransferStream& stream, uint8_t nType);

		void WriteInt64SetByType(CTransferStream& stream, uint8_t nType);

		void WriteFloatSetByType(CTransferStream& stream, uint8_t nType);

		void WriteDoubleSetByType(CTransferStream& stream, uint8_t nType);

		void WriteStringSetByType(CTransferStream& stream, uint8_t nType);
		//////////////////////////////////////////////////////////////////////////
		void ReadCharByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadBoolByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadUInt8ByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadUInt16ByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadUInt32ByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadUInt64ByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadInt8ByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadInt16ByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadInt32ByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadInt64ByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadFloatByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadDoubleByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadStringByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadBoolSetByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadUInt8SetByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadInt8SetByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadUInt16SetByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadInt16SetByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadUInt32SetByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadInt32SetByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadUInt64SetByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadInt64SetByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadFloatSetByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadDoubleSetByType(int& readOffset, CTransferStream& stream, bool bChange) const;

		void ReadStringSetByType(int& readOffset, CTransferStream& stream, bool bChange) const;
		//////////////////////////////////////////////////////////////////////////
		void WriteCharFromString(const char* szInput, int length, uint8_t nType);

		void WriteBoolFromString(const char* szInput, int length, uint8_t nType);

		void WriteUInt8FromString(const char* szInput, int length, uint8_t nType);

		void WriteUInt16FromString(const char* szInput, int length, uint8_t nType);

		void WriteUInt32FromString(const char* szInput, int length, uint8_t nType);

		void WriteUInt64FromString(const char* szInput, int length, uint8_t nType);

		void WriteInt8FromString(const char* szInput, int length, uint8_t nType);

		void WriteInt16FromString(const char* szInput, int length, uint8_t nType);

		void WriteInt32FromString(const char* szInput, int length, uint8_t nType);

		void WriteInt64FromString(const char* szInput, int length, uint8_t nType);

		void WriteFloatFromString(const char* szInput, int length, uint8_t nType);

		void WriteDoubleFromString(const char* szInput, int length, uint8_t nType);

		void WriteStringFromString(const char* szInput, int length, uint8_t nType);

		void WriteBoolSetFromString(const char* szInput, int length, uint8_t nType);

		void WriteUInt8SetFromString(const char* szInput, int length, uint8_t nType);

		void WriteInt8SetFromString(const char* szInput, int length, uint8_t nType);

		void WriteUInt16SetFromString(const char* szInput, int length, uint8_t nType);

		void WriteInt16SetFromString(const char* szInput, int length, uint8_t nType);

		void WriteUInt32SetFromString(const char* szInput, int length, uint8_t nType);

		void WriteInt32SetFromString(const char* szInput, int length, uint8_t nType);

		void WriteUInt64SetFromString(const char* szInput, int length, uint8_t nType);

		void WriteInt64SetFromString(const char* szInput, int length, uint8_t nType);

		void WriteFloatSetFromString(const char* szInput, int length, uint8_t nType);

		void WriteDoubleSetFromString(const char* szInput, int length, uint8_t nType);

		void WriteStringSetFromString(const char* szInput, int length, uint8_t nType);
		//////////////////////////////////////////////////////////////////////////
		void ReadCharToString(int& readOffset, std::string& output) const;

		void ReadBoolToString(int& readOffset, std::string& output) const;

		void ReadUInt8ToString(int& readOffset, std::string& output) const;

		void ReadUInt16ToString(int& readOffset, std::string& output) const;

		void ReadUInt32ToString(int& readOffset, std::string& output) const;

		void ReadUInt64ToString(int& readOffset, std::string& output) const;

		void ReadInt8ToString(int& readOffset, std::string& output) const;

		void ReadInt16ToString(int& readOffset, std::string& output) const;

		void ReadInt32ToString(int& readOffset, std::string& output) const;

		void ReadInt64ToString(int& readOffset, std::string& output) const;

		void ReadFloatToString(int& readOffset, std::string& output) const;

		void ReadDoubleToString(int& readOffset, std::string& output) const;

		void ReadStringToString(int& readOffset, std::string& output) const;

		void ReadBoolSetToString(int& readOffset, std::string& output) const;

		void ReadUInt8SetToString(int& readOffset, std::string& output) const;

		void ReadInt8SetToString(int& readOffset, std::string& output) const;

		void ReadUInt16SetToString(int& readOffset, std::string& output) const;

		void ReadInt16SetToString(int& readOffset, std::string& output) const;

		void ReadUInt32SetToString(int& readOffset, std::string& output) const;

		void ReadInt32SetToString(int& readOffset, std::string& output) const;

		void ReadUInt64SetToString(int& readOffset, std::string& output) const;

		void ReadInt64SetToString(int& readOffset, std::string& output) const;

		void ReadFloatSetToString(int& readOffset, std::string& output) const;

		void ReadDoubleSetToString(int& readOffset, std::string& output) const;

		void ReadStringSetToString(int& readOffset, std::string& output) const;

		// stream like operators for storing data
		CStoreStream& operator<<(char input);

		CStoreStream& operator<<(bool input);

		// unsigned
		CStoreStream& operator<<(uint8_t input);

		CStoreStream& operator<<(uint16_t input);

		CStoreStream& operator<<(uint32_t input);

		CStoreStream& operator<<(uint64_t input);

		// signed as in 2e complement
		CStoreStream& operator<<(int8_t input);

		CStoreStream& operator<<(int16_t input);

		CStoreStream& operator<<(int32_t input);

		CStoreStream& operator<<(int64_t input);

		CStoreStream& operator<<(float input);

		CStoreStream& operator<<(double input);

		CStoreStream& operator<<(const std::string& input);

		CStoreStream& operator<<(const char input[])
		{
			uint16_t length = (uint16_t)strlen(input);
			*this << length;

			if(length > 0) {
#ifdef SS_DANGER_AREA_CHECKING
				CheckDangerArea(m_writeOffset);
#endif
				AddBytesAndReallocate(length);
				memcpy((m_data + m_writeOffset), input, length);
				m_writeOffset += length;
			}
			return *this;
		}

        template<class T>
        CStoreStream & operator<<(const std::vector<T>& input)
        {
			int32_t length = (int32_t)input.size();
			*this << length;

			for(int i = 0; i < length; ++i) {
				*this << (T)input[i];
			}
			return *this;
        }

        template<class T>
        CStoreStream & operator<<(const std::set<T>& input)
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
        CStoreStream & operator<<(const std::list<T>& input)
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
        CStoreStream & operator<<(const T (&input)[nSize])
        {
			int32_t length = (int32_t)nSize;
			*this << length;

			for(int i = 0; i < length; ++i) {
				*this << input[i];
			}
			return *this;
        }

		void WriteObject();

		void WriteObject(const CStoreStream& input);

		// stream like operators for reading data
		CStoreStream& operator>>(std::vector<bool>::reference output);

		CStoreStream& operator>>(bool& output);

        CStoreStream& operator>>(char& output);

		//unsigned
		CStoreStream& operator>>(uint8_t& output);

		CStoreStream& operator>>(uint16_t& output);

		CStoreStream& operator>>(uint32_t& output);

		CStoreStream& operator>>(uint64_t& output);

		//signed as in 2e complement
		CStoreStream& operator>>(int8_t& output);

		CStoreStream& operator>>(int16_t& output);

		CStoreStream& operator>>(int32_t& output);

		CStoreStream& operator>>(int64_t& output);

		CStoreStream& operator>>(float& output);

		CStoreStream& operator>>(double& output);

		CStoreStream& operator>>(std::string& output);

		template<size_t nSize>
		CStoreStream& operator>>(char (&output)[nSize])
		{
			if(nSize < 1) {
				uint16_t length = 0;
				*this >> length;

				if(length + m_readOffset > m_writeOffset) {
					return *this;
				}

				IgnoreBytes(length);
				return *this;
			}

			uint16_t length = 0;
			*this >> length;

			if(length + m_readOffset > m_writeOffset) {
				return *this;
			}

			if(length < nSize) {
				memcpy(output, (m_data + m_readOffset), length);
				m_readOffset += length;
			} else {
				memcpy(output, (m_data + m_readOffset), nSize);
				m_readOffset += nSize;

				int nLeaveLen = length - nSize;
				if(nLeaveLen > 0) {
					IgnoreBytes(nLeaveLen);
				}
			}
			return *this;
		}

        template<class T>
        CStoreStream& operator>>(std::vector<T>& output)
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
        CStoreStream& operator>>(std::set<T>& output)
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
        CStoreStream& operator>>(std::list<T>& output)
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
        CStoreStream& operator>>(T (&output)[nSize])
        {
			int32_t length = 0;
			*this >> length;

			for(int i = 0; i < length; ++i) {
				*this >> output[i];
			}

			int32_t leave = length - nSize;
			if(leave > 0) {
				IgnoreBytes(leave * sizeof(T));
			}
			return *this;
        }

		template<class T>
		void ReadObject(int& readOffset, std::vector<T>& output) const
		{
			int nSize = sizeof(util::CStoreObject);
			if(nSize + readOffset > m_writeOffset) {
				return;
			}

			if(IsObjectType(readOffset)) {
				CStoreObject* pObject = (CStoreObject*)(m_data + readOffset);
				readOffset += nSize;
				util::CStoreStream object(pObject->GetData(), pObject->GetLength(), false);
				object >> output;
			} else {
				assert(false);
			}
		}

		void ReadObject(int& readOffset, std::string& output) const;

		void ReadObject(int& readOffset, CStoreStream& output) const;

		void IgnoreBytes(const int numberOfBytes) {
			if(numberOfBytes < 1) {
				assert(false);
				return;
			}
			m_readOffset += numberOfBytes;
		}

        const char* GetData() const {
            return m_data;
        }

		int GetLength() const {
			return m_writeOffset;
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

		void SetReadOffset(int nReadOffset) {
			if(nReadOffset < 0) {
				assert(nReadOffset);
				return;
			}
			m_readOffset = nReadOffset;
		}

		void SetWriteOffset(int nWriteOffset) {
			if(nWriteOffset < 0) {
				assert(nWriteOffset);
				return;
			}
			m_writeOffset = nWriteOffset;
		}

    private:
		void AddBytesAndReallocate(const int numberOfBytes);
		bool IsObjectArea(int pos);

		inline CStoreObject* ReadObjectType(int objOffset) const {

			if(!IsObjectType(objOffset)) {
				return NULL;
			}
			return (CStoreObject*)(m_data + objOffset);
		}

		inline bool IsObjectType(int objOffset) const {

			if(objOffset < 0) {
				return false;
			}
			
			if(objOffset > this->m_objOffset) {
				return false;
			}

			CStoreObject* curPtr = (CStoreObject*)(m_data + objOffset);;
			if(NULL == curPtr) {
				return false;
			}

			if(curPtr->m_preOffset < 0) {
				if (curPtr->m_nxtOffset < 0) {
					if (objOffset == this->m_objOffset) {
						return true;
					}
				} else if(curPtr->m_nxtOffset <= this->m_objOffset){
					CStoreObject* nxtPtr = (CStoreObject*)(m_data + curPtr->m_nxtOffset);
					if (NULL != nxtPtr && nxtPtr->m_preOffset == objOffset) {
						return true;
					}
				}
			} else {
				if (curPtr->m_preOffset <= this->m_objOffset) {
					CStoreObject* perPtr = (CStoreObject*)(m_data + curPtr->m_preOffset);
					if (NULL != perPtr && perPtr->m_nxtOffset == objOffset) {
						return true;
					}
				}
			}		
			return false;
		}

		inline void CheckDangerArea(int pos) {
			if(-1 != m_objOffset) {
				if(pos == this->m_objOffset) {
					assert(false);
				} else if(pos < this->m_objOffset) {
					assert(!IsObjectArea(pos));
				}
			}
		}

	private:
		int m_objOffset;
        char* m_data;
		int m_size;
		int m_writeOffset;
		int m_readOffset;
		bool m_bCopyData;
		char m_stackData[STORESTREAM_STACK_ALLOC_SIZE];

	private:
		struct TypeOperatorSet {
			void (util::CStoreStream::*m_pWriteByType)(CTransferStream& stream, uint8_t nType);
			void (util::CStoreStream::*m_pReadByType)(int& readOffset, CTransferStream& stream, bool bChange) const;
			void (util::CStoreStream::*m_pWriteFromString)(const char* szInput, int length, uint8_t nType);
			void (util::CStoreStream::*m_pReadToString)(int& readOffset, std::string& output) const;
		};
		static struct TypeOperatorSet s_typeOperators[STREAM_DATA_SIZE];
};

}

#endif /* STORESTREAM_H */
