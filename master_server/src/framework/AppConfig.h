/*
 * File:   AppConfig.h
 * Author: Jehu Shaw
 *
 * Created on 2014_3_1 AM 10:32
 */

#ifndef APPCONFIG_H
#define	APPCONFIG_H

#include "Common.h"
#include "tinyxml/tinyxml.h"
#include <string>
#include <map>
#include "Singleton.h"

class AppConfig : public util::Singleton<AppConfig> {
public:
	typedef bool (*ReviseMethod)(std::string& outStr);
	typedef std::map<std::string, ReviseMethod> REVISE_SET_T;

	AppConfig() {}

	bool LoadFile(const char* fileName){

		TiXmlDocument tiXmlDocument;
		if(!tiXmlDocument.LoadFile(fileName)) {
			throw std::runtime_error("Can't load file.");
			return false;
		}

		TiXmlElement* pXmlElement = (TiXmlElement*)tiXmlDocument.FirstChildElement("configuration");
		if(NULL == pXmlElement) {
			throw std::runtime_error("Can't find the \"configuration\" element.");
			return false;
		}

		pXmlElement = (TiXmlElement*)pXmlElement->FirstChildElement("appSettings");
		if(NULL == pXmlElement) {
			throw std::runtime_error("Can't find the \"appSettings\" element.");
			return false;
		}

		pXmlElement = pXmlElement->FirstChildElement();
		while(NULL != pXmlElement) {

			if(strcmp(pXmlElement->Value(),"add") == 0) {
				std::string strKey(pXmlElement->Attribute("key"));
				std::string strValue(pXmlElement->Attribute("value"));
				m_configMap[strKey] = strValue;

			} else if(strcmp(pXmlElement->Value(),"remove") == 0) {
				std::string strKey(pXmlElement->Attribute("key"));
				m_configMap.erase(strKey);

			} else if(strcmp(pXmlElement->Value(),"clear") == 0) {
				m_configMap.clear();
			}

			pXmlElement = pXmlElement->NextSiblingElement();
		}
		return true;
	}

    bool LoadFile(const char* fileName, REVISE_SET_T& reviseSet){

        TiXmlDocument tiXmlDocument;
		if(!tiXmlDocument.LoadFile(fileName)) {
			throw std::runtime_error("Can't load file.");
			return false;
		}

		TiXmlElement* pXmlElement = (TiXmlElement*)tiXmlDocument.FirstChildElement("configuration");
		if(NULL == pXmlElement) {
			throw std::runtime_error("Can't find the \"configuration\" element.");
			return false;
		}

		pXmlElement = (TiXmlElement*)pXmlElement->FirstChildElement("appSettings");
		if(NULL == pXmlElement) {
			throw std::runtime_error("Can't find the \"appSettings\" element.");
			return false;
		}

		pXmlElement = pXmlElement->FirstChildElement();
		while(NULL != pXmlElement) {

			if(strcmp(pXmlElement->Value(),"add") == 0) {
				std::string strKey(pXmlElement->Attribute("key"));
				std::string strValue(pXmlElement->Attribute("value"));
				if(!reviseSet.empty()) {
					REVISE_SET_T::const_iterator it(reviseSet.lower_bound(strKey));
					if(reviseSet.end() == it) {
						--it;
					}
					if(NULL != it->second && strstr(strKey.c_str(),
						it->first.c_str()) != NULL)
					{
						if(!it->second(strValue)) {
							assert(false);
						}
					}
				}
				m_configMap[strKey] = strValue;

			} else if(strcmp(pXmlElement->Value(),"remove") == 0) {
				std::string strKey(pXmlElement->Attribute("key"));
				m_configMap.erase(strKey);

			} else if(strcmp(pXmlElement->Value(),"clear") == 0) {
				m_configMap.clear();
			}

			pXmlElement = pXmlElement->NextSiblingElement();
		}
		return true;
    }

    double GetNumber(std::string key, double dDefault = 0.0f) const {

		CONFIG_MAP_T::const_iterator it = m_configMap.find(key);
		if(it != m_configMap.end()) {
			return atof(it->second.c_str());
		} else {
			return dDefault;
		}
    }

	int GetInt(std::string key, int nDefault = 0) const {

		CONFIG_MAP_T::const_iterator it = m_configMap.find(key);
		if(it != m_configMap.end()) {
			return atoi(it->second.c_str());
		} else {
			return nDefault;
		}
	}

	int64_t GetInt64(std::string key, int64_t nDefault = 0) const {

		CONFIG_MAP_T::const_iterator it = m_configMap.find(key);
		if(it != m_configMap.end()) {
			return atoll(it->second.c_str());
		} else {
			return nDefault;
		}
	}

	std::string GetString(std::string key) const {

		CONFIG_MAP_T::const_iterator it(m_configMap.find(key));
		if(it != m_configMap.end()) {
			return it->second;
		} else {
			return std::string();
		}
	}

private:
	typedef std::map<std::string, std::string> CONFIG_MAP_T;
	CONFIG_MAP_T m_configMap;
};
#endif	/* APPCONFIG_H */

