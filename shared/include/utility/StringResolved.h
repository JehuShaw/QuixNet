/*
 * File:   StringResolved.h
 * Author: Jehu Shaw
 *
 * Created on 2014_5_8, 14:09
 */

#ifndef _STRINGRESOLVED_H
#define _STRINGRESOLVED_H

#include <string>
#include "Common.h"
#include "SeparatedStream.h"

namespace util {

class CStringResolved
{
	public:
		CStringResolved(char enclosure_delim = ',')
			: m_enclosure_delim(enclosure_delim)
		{
		}

		~CStringResolved() {}

		void CastToType(...)
		{
			throw std::runtime_error("Unknown the type !");
		}

		void CastToType(bool & value, const std::string& str)
		{
            value = (strtoul(str.c_str(), NULL, 10) != 0);
		}

		void CastToType(char & value, const std::string& str)
		{
			value = *str.c_str();
		}
		//unsigned
		void CastToType(uint8_t & value, const std::string& str)
		{
            value = (uint8_t) strtoul(str.c_str(), NULL, 10);
		}
		void CastToType(uint16_t & value, const std::string& str)
		{
            value = (uint16_t) strtoul(str.c_str(), NULL, 10);
		}
		void CastToType(uint32_t & value, const std::string& str)
		{
            value = (uint32_t) strtoul(str.c_str(), NULL, 10);
		}
		void CastToType(uint64_t & value, const std::string& str)
		{
            value = (uint64_t) strtoull(str.c_str(), NULL, 10);
		}
		//signed
		void CastToType(int8_t & value, const std::string& str)
		{
            value = (int8_t) strtol(str.c_str(), NULL, 10);
		}
		void CastToType(int16_t & value, const std::string& str)
		{
            value = (int16_t) strtol(str.c_str(), NULL, 10);
		}
		void CastToType(int32_t & value, const std::string& str)
		{
            value = (int32_t) strtol(str.c_str(), NULL, 10);
		}
		void CastToType(int64_t & value, const std::string& str)
		{
            value = (int64_t) strtoll(str.c_str(), NULL, 10);
		}
		void CastToType(float & value, const std::string& str)
		{
            value = (float) strtod(str.c_str(), NULL);
		}
		void CastToType(double & value, const std::string& str)
		{
            value = (double) strtod(str.c_str(), NULL);
		}
		void CastToType(std::string & value, const std::string& str)
		{
            value = str;
		}
		template<size_t nSize>
		void CastToType(char (&value)[nSize], const std::string& str)
		{
			int nCpyLen = str.length() > nSize ? nSize : str.length();
			memcpy(value, str.data(), nCpyLen);
			value[nCpyLen] = '\0';
		}

        template<class T>
        void CastToType(std::vector<T>& value, const std::string& str)
        {
			CSeparatedStream enclosure(str.data(), str.length(),
				false, m_enclosure_delim, m_enclosure_delim);

			do {
				value.resize(value.size() + 1);
				enclosure >> value.back();
			} while(enclosure.MoreData());
        }

        template<class T>
        void CastToType(std::set<T>& value, const std::string& str)
        {
			CSeparatedStream enclosure(str.data(), str.length(),
				false, m_enclosure_delim, m_enclosure_delim);

			T temp;
			do {
				enclosure >> temp;
				value.insert(temp);
			} while(enclosure.MoreData());
        }

        template<class T>
        void CastToType(std::list<T>& value, const std::string& str)
        {
			CSeparatedStream enclosure(str.data(), str.length(),
				false, m_enclosure_delim, m_enclosure_delim);

			do {
				value.resize(value.size() + 1);
				enclosure >> value.back();
			} while(enclosure.MoreData());
        }

        template<class T, size_t nSize>
        void CastToType(T (&value)[nSize], const std::string& str)
        {
			CSeparatedStream enclosure(str.data(), str.length(),
				false, m_enclosure_delim, m_enclosure_delim);

			for(int i = 0; i < nSize; ++i) {
				enclosure >> value[i];
				if(!enclosure.MoreData()) {
					break;
				}
			}
        }

		//////////////////////////////////////////////////////////////////////////
		void CastToString(std::string& str, ...)
		{
			throw std::runtime_error("Unknown the type !");
		}

		void CastToString(std::string& str, bool value)
		{
			if(value) {
				str = '1';
			} else {
				str = '0';
			}
		}

		void CastToString(std::string& str, char value)
		{
			str = value;
		}

		void CastToString(std::string& str, int8_t value)
		{
			char szBuf[64] = { '\0' };
			ltostr(szBuf, value, 10, 0);
			str = szBuf;
		}

		void CastToString(std::string& str, int16_t value)
		{
			char szBuf[64] = { '\0' };
			ltostr(szBuf, value, 10, 0);
			str = szBuf;
		}

		void CastToString(std::string& str, int32_t value)
		{
			char szBuf[64] = { '\0' };
			ltostr(szBuf, value, 10, 0);
			str = szBuf;
		}

		void CastToString(std::string& str, int64_t value)
		{
			char szBuf[64] = { '\0' };
			lltostr(szBuf, value, 10, 0);
			str = szBuf;
		}

		void CastToString(std::string& str, uint8_t value)
		{
			char szBuf[64] = { '\0' };
			ultostr(szBuf, value, 10, 0);
			str = szBuf;
		}

		void CastToString(std::string& str, uint16_t value)
		{
			char szBuf[64] = { '\0' };
			ultostr(szBuf, value, 10, 0);
			str = szBuf;
		}

		void CastToString(std::string& str, uint32_t value)
		{
			char szBuf[64] = { '\0' };
			ultostr(szBuf, value, 10, 0);
			str = szBuf;
		}

		void CastToString(std::string& str, uint64_t value)
		{
			char szBuf[64] = { '\0' };
			ulltostr(szBuf, value, 10, 0);
			str = szBuf;
		}

		void CastToString(std::string& str, float value)
		{
			char szBuf[64] = { '\0' };
			gcvt(value, FLT_DECIMAL_DIG, szBuf);
			str = szBuf;
		}

		void CastToString(std::string& str, double value)
		{
			char szBuf[64] = { '\0' };
			gcvt(value, DBL_DECIMAL_DIG, szBuf);
			str = szBuf;
		}

		void CastToString(std::string& str, const std::string& value)
		{
			str = value;
		}

		void CastToString(std::string& str, const char* value)
		{
			str = value;
		}

		template<class T>
		void CastToString(std::string& str, const std::vector<T>& value)
		{
			if(value.empty()) {
				if(!str.empty()) {
					str.clear();
				}
				return;
			}

			CSeparatedStream enclosure(m_enclosure_delim, m_enclosure_delim);
			int nSize = (int)value.size();
			for(int i = 0; i < nSize; ++i) {
				enclosure << value[i];
			}
			enclosure.EndLine();

			str = enclosure.Str();
		}

		template<class T>
		void CastToString(std::string& str, const std::set<T>& value)
		{
			if(value.empty()) {
				if(!str.empty()) {
					str.clear();
				}
				return;
			}

			CSeparatedStream enclosure(m_enclosure_delim, m_enclosure_delim);

			typename std::set<T>::const_iterator it = value.begin();
			for(; value.end() != it; ++it) {
				enclosure << *it;
			}
			enclosure.EndLine();

			str = enclosure.Str();
		}

		template<class T>
		void CastToString(std::string& str, const std::list<T>& value)
		{
			if(value.empty()) {
				if(!str.empty()) {
					str.clear();
				}
				return;
			}

			CSeparatedStream enclosure(m_enclosure_delim, m_enclosure_delim);

			typename std::list<T>::const_iterator it = value.begin();
			for(; value.end() != it; ++it) {
				enclosure << *it;
			}
			enclosure.EndLine();

			str = enclosure.Str();
		}

		template<class T, size_t nSize>
		void CastToString(std::string& str, const T (&value)[nSize])
		{
			if(nSize < 1) {
				if(!str.empty()) {
					str.clear();
				}
				return;
			}

			CSeparatedStream enclosure(m_enclosure_delim, m_enclosure_delim);

			for(int i = 0; i < (int)nSize; ++i) {
				enclosure << value[i];
			}
			enclosure.EndLine();

			str = enclosure.Str();
		}

	private:
        char m_enclosure_delim;
};

}

#endif /* _STRINGRESOLVED_H */
