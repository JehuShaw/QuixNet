/*
 * File:   ValueStream.h
 * Author: Jehu Shaw
 *
 * Created on 2014_5_8, 14:09
 */

#ifndef __VALUESTREAM_H__
#define __VALUESTREAM_H__

#include "TransferStream.h"

namespace util {

class CValueStream
{
public:
		CValueStream() : m_tsValues() {
		}

		CValueStream(uint32_t initByteSize) : m_tsValues(initByteSize) {
		}

		CValueStream(const char* data, uint32_t lengthInBytes, bool copyData) 
			: m_tsValues(data, lengthInBytes, copyData) {
		}

		CValueStream(const std::string& data, bool copyData) 
			: m_tsValues(data, copyData) {
		}

		CValueStream(const CValueStream& orig)
			: m_tsValues(orig.m_tsValues) {
		}

		~CValueStream() {}

		template<class T>
		void Serialize(T input, bool bChange) {
			m_tsValues.Serialize(input, bChange);
		}

		template<class T>
		void Serialize(T input) {
			m_tsValues.Serialize(input, true);
		}

		template<class T>
		void Parse(T& ouput) {
			m_tsValues.Parse(ouput);
		}

		void WriteBytes(const char* input, const int numberOfBytes) {
			m_tsValues.WriteBytes(input, numberOfBytes);
		}

		const char* GetData() const {
			return m_tsValues.GetData();
		}

		int GetLength() const {
			return m_tsValues.GetNumberOfBytesUsed();
		}

		inline void Reset() {
			m_tsValues.Reset();
		}

private:
	CValueStream& operator = (const CValueStream& right) {
		return *this; 
	}

private:
	CTransferStream m_tsValues;
};

}

#endif /* __VALUESTREAM_H__ */
