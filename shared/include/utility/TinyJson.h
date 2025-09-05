/**
*
*	tiny::Json library
*	Copyright 2017 Button
*   Modify by Jehu Shaw
* 
* **********************************************************
*  Read json :
*
*  string jsonstring = "
*  {
*		"name":"zhangsan",
*		"age" : 26,
*		"data" : [
*			{
*				"one":"chenone",
*				"two" : {
*					"love1":"2233",
*					"love2":44444
*				}
*			}
*		]
*	}";
*	
*	tiny::Json json;
*	json.ReadJson(jsonstring);
*	std::string strName;
*	json.Get(strName, "name");
*	JArray data;
*	json.Get(data, "data");
*	for (int i = 0; i < data.Count(); ++i) {
*		std::string strOne;
*		data.GetAt(strOne, i);
*	}
*
* *********************************************************
*	Write json:
*
*	tiny::Json wjson;
*	wjson.Set("liergou", "name");
*	wjson.Set(26, "age");
*	wjson.Set(true, "handsome");
*
*	tiny::Json subjson;
*	subjson.Set("book", "love1");
*	subjson.Set(666, "love2");
*
*	tiny::JArray subjson2;
*	subjson2.Set("book2");
*	subjson2.Set(6662);
*
*	wjson.Set(subjson, "data");
*	wjson.Set(subjson2, "data2");
*
*	string str = wjson.WriteJson();
*	cout << "json string: \r\n" << endl;
*	cout << str << endl;
*/

#ifndef TINY_JSON_H
#define TINY_JSON_H

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <assert.h>

namespace tiny {

	enum eJsonType
	{
		JSON_TYPE_NIL,
		JSON_TYPE_OBJECT,
		JSON_TYPE_ARRAY,
	};

	/**
	* Untyped, confirmed during parsing
	*/
	class Value
	{
	public:
		template<typename R> 
		static void GetAs(R& outVal, const std::string& inVal) {
			std::istringstream iss(inVal);
			iss >> outVal;
		}

		static void GetAs(std::string& outVal, const std::string& inVal) {
			outVal = inVal;
		}

		static void GetAs(bool& outVal, const std::string& inVal) {
			outVal = (inVal == "true");
		}

		template<typename V> 
		static void Set(std::string& outStr, V v) {
			std::ostringstream oss;
			oss << v;
			outStr = oss.str();
		}

		static void Set(std::string& outStr, bool v) {
			std::ostringstream oss;
			if (v) {
				oss << "true";
			} else {
				oss << "false";
			}
			outStr = oss.str();
		}

		template<typename T>
		static void Push(std::string& outStr, const T& v) {
			std::ostringstream oss;
			oss << v.WriteJson();
			outStr = oss.str();
		}
	};

	/**
	* This template class handles the case where
	* the value corresponding to the json key
	* is a nested object or array
	*
	*/
	template<class T>
	class ValueArray : public T
	{
	public:
		ValueArray() {}
		ValueArray(const std::vector<std::string>& vo) : T::KeyVal_(vo) {}
	};

	/**
	* Parsing json string is saved as a sequence of key values, parsing is done layer by layer
	* When parsing, treat json as a combination of object '{}' and array '[]'
	*
	*/
	class ParseJson
	{
	public:
        ParseJson() {}
        ~ParseJson() {}

	public:
		bool ParseArray(std::string json, std::vector<std::string>& vo);

		bool ParseObj(std::string json, std::vector<std::string>& vo);

	protected:
		void Trims(const std::string& s, char lc, char rc, std::string& outputstr);
		int GetFirstNotSpaceChar(const std::string& s, int cur);
		void FetchArrayStr(const std::string& inputstr, int inpos, int& offset, std::string& objstr);
		void FetchObjStr(const std::string& inputstr, int inpos, int& offset, std::string& objstr);
		void FetchStrStr(const std::string& inputstr, int inpos, int& offset, std::string& objstr);
		void FetchNumStr(const std::string& inputstr, int inpos, int& offset, std::string& objstr);
	};

	inline bool ParseJson::ParseArray(std::string json, std::vector<std::string>& vo) {
		if(!vo.empty()) {
			vo.clear();
		}
		Trims(json, '[', ']', json);
		std::string tokens;
		size_t i = 0;
		for (; i < json.size(); ++i) {
			char c = json[i];
			if (isspace(c) || c == '\"') continue;
			if (c == ':' || c == ',' || c == '{' || c == '[') {
				if (!tokens.empty()) {
					vo.push_back(tokens);
					tokens.clear();
				}
				if (c == ',') continue;
				int offset = 0;
				char nextc = c;
				for (; c != '{' && c != '[';) {
					nextc = json[++i];
					if (isspace(nextc)) continue;
					break;
				}
				if (nextc == '{') {
					FetchObjStr(json, i, offset, tokens);
				}
				else if (nextc == '[') {
					FetchArrayStr(json, i, offset, tokens);
				}
				i += offset;
				continue;
			}
			tokens.push_back(c);
		}
		if (!tokens.empty()) {
			vo.push_back(tokens);
		}
		return true;
	}

	// Parsing for key-value calls parsing one level at a time
	inline bool ParseJson::ParseObj(std::string json, std::vector<std::string>& vo) {
		if(!vo.empty()) {
			vo.clear();
		}
		auto LastValidChar = [&](int index)->char{
			for (int i = index-1; i >= 0; --i){
				if (isspace(json[i])) continue;
				char tmp = json[i];
				return tmp;
			}
			return '\0';
		};

		Trims(json, '{', '}', json);
		size_t i = 0;
		for (; i < json.size(); ++i) {
			char nextc = json[i];
			if (isspace(nextc)) continue;

			std::string tokens;
			int offset = 0;
			if (nextc == '{') {
				FetchObjStr(json, i, offset, tokens);
			}
			else if (nextc == '[') {
				FetchArrayStr(json, i, offset, tokens);
			}
			else if (nextc == '\"') {
				FetchStrStr(json, i, offset, tokens);
			}
			else if (isdigit(nextc) && LastValidChar(i) == ':')
			{
				FetchNumStr(json, i, offset, tokens);
			}
			else {
				continue;
			}
			vo.push_back(tokens);
			i += offset;
		}
		if(vo.empty() && !json.empty()) {
			vo.push_back(json);
		}
		return true;
	}

	inline void ParseJson::Trims(const std::string& s, char lc, char rc, std::string& outputstr)
	{
		if (s.find(lc) != std::string::npos && s.find(rc) != std::string::npos) {
			size_t b = s.find_first_of(lc);
			size_t e = s.find_last_of(rc);
			outputstr = s.substr(b + 1, e - b - 1);
		} else if(&s != &outputstr) {
			outputstr = s;
		}
	}

	inline int ParseJson::GetFirstNotSpaceChar(const std::string& s, int cur)
	{
		for (size_t i = cur; i < s.size(); i++){
			if (isspace(s[i])) continue;
			return i - cur;
		}
		return 0;
	}

	inline void ParseJson::FetchArrayStr(const std::string& inputstr, int inpos, int& offset, std::string& objstr)
	{
		int tokencount = 0;
		size_t i = inpos + GetFirstNotSpaceChar(inputstr, inpos);
		for (; i < inputstr.size(); i++) {
			char c = inputstr[i];
			if (c == '[') {
				++tokencount;
			}
			if (c == ']') {
				--tokencount;
			}
			objstr.push_back(c);
			if (tokencount == 0) {
				break;
			}
		}
		offset = i - inpos;
	}

	inline void ParseJson::FetchObjStr(const std::string& inputstr, int inpos, int& offset, std::string& objstr)
	{
		int tokencount = 0;
		size_t i = inpos + GetFirstNotSpaceChar(inputstr, inpos);
		for (; i < inputstr.size(); i++) {
			char c = inputstr[i];
			if (c == '{') {
				++tokencount;
			}
			if (c == '}') {
				--tokencount;
			}
			objstr.push_back(c);
			if (tokencount == 0) {
				break;
			}
		}
		offset = i - inpos;
	}

	inline void ParseJson::FetchStrStr(const std::string& inputstr, int inpos, int& offset, std::string& objstr)
	{
		int tokencount = 0;
		size_t i = inpos + GetFirstNotSpaceChar(inputstr, inpos);
		for (; i < inputstr.size(); ++i) {
			char c = inputstr[i];
			if (c == '\"') {
				++tokencount;
			}
			objstr.push_back(c);
			if (tokencount % 2 == 0 && (c == ',' || c == ':')) {
				break;
			}
		}
		offset = i - inpos;

		Trims(objstr, '\"', '\"', objstr);
	}

	inline void ParseJson::FetchNumStr(const std::string& inputstr, int inpos, int& offset, std::string& objstr)
	{
		size_t i = inpos + GetFirstNotSpaceChar(inputstr, inpos);
		for (; i < inputstr.size(); i++) {
			char c = inputstr[i];
			if (c == ',') {
				break;
			}
			objstr.push_back(c);
		}
		offset = i - inpos;
	}

	/**
	* interface
	*
	*/
	class Json;
	typedef ValueArray<Json> JArray;
	typedef Json JObject;

	class Json
	{
	public:
		Json() : json_type_(JSON_TYPE_NIL) {}

        ~Json() {}

	public:
		// read
		inline bool ReadJson(const char* json) {
			return ReadJson(std::string(json));
		}

		bool ReadJson(const std::string& json) {
			if(json.empty()) {
				return true;
			}
			char nextc = 0;
			for (size_t i = 0; i < json.size(); ++i) {
				nextc = json[i];
				if (isspace(nextc)) continue;
				break;
			}

			ParseJson p;
			if (nextc == '[') {
				json_type_ = JSON_TYPE_ARRAY;
				p.ParseArray(json, KeyVal_);
			} else {
				json_type_ = JSON_TYPE_OBJECT;
				p.ParseObj(json, KeyVal_);
			}
			return true;
		}

		template<typename R>
		bool Get(R& outVal, const char* key) const {
			return Get(outVal, std::string(key));
		}

		template<typename R>
		bool Get(R& outVal, const std::string& key) const {
			int nSize = (int)KeyVal_.size();
			int i = 0;
			while(i < nSize) {
				if(KeyVal_[i] == key) {
					break;
				}
				i = i + 2;
			}
			if(i + 1 >= nSize) {
				return false;
			}
			Value::GetAs<R>(outVal, KeyVal_[i + 1]);
			return true;
		}

		template<typename R, typename K>
		inline bool Get(R& outVal, K key) const {
			std::ostringstream oss;
			oss << key;
			std::string strKey(oss.str());
			return Get(outVal, strKey);
		}

		inline bool GetAt(JObject& outVal, int index) const;

		inline bool Get(JObject& outVal, const char* key) const;

		inline bool Get(JObject& outVal, const std::string& key) const;

		inline bool GetAt(JArray& outVal, int index) const;

		inline bool Get(JArray& outVal, const char* key) const;

		inline bool Get(JArray& outVal, const std::string& key) const;

        template<typename R>
        bool GetAt(R& outVal, int index) const {
			if(index < 0 || index >= (int)KeyVal_.size()) {
				return false;
			}
            Value::GetAs<R>(outVal, KeyVal_[index]);
			return true;
        }



		template<typename T>
		inline bool SetAt(T input, int index) {
			if(index < 0 || index >= (int)KeyVal_.size()) {
				return false;
			}
			if (JSON_TYPE_NIL == json_type_) {
				json_type_ = JSON_TYPE_ARRAY;
			} else {
				assert(json_type_ == JSON_TYPE_ARRAY);
			}
			Value::Set(KeyVal_[index], input);
			return true;
		}

		template<typename T>
		inline void Set(T input) {
			if (JSON_TYPE_NIL == json_type_) {
				json_type_ = JSON_TYPE_ARRAY;
			} else {
				assert(json_type_ == JSON_TYPE_ARRAY);
			}
			KeyVal_.resize(KeyVal_.size() + 1);
			Value::Set(KeyVal_.back(), input);
		}

		template<typename T>
		inline void Set(T input, const char* k) {
			if (JSON_TYPE_NIL == json_type_) {
				json_type_ = JSON_TYPE_OBJECT;
			} else {
				assert(json_type_ == JSON_TYPE_OBJECT);
			}
			
			int valueIndex = FindValueIndex(k);
			if (valueIndex > -1) {
				Value::Set(KeyVal_[valueIndex], input);
			} else {
				int nSize = (int)KeyVal_.size() + 2;
				KeyVal_.resize(nSize);
				KeyVal_[nSize - 2] = k;
				Value::Set(KeyVal_[nSize - 1], input);
			}
		}

		template<typename T>
		inline void Set(T input, const std::string& k) {
			if (JSON_TYPE_NIL == json_type_) {
				json_type_ = JSON_TYPE_OBJECT;
			} else {
				assert(json_type_ == JSON_TYPE_OBJECT);
			}
			int valueIndex = FindValueIndex(k);
			if (valueIndex > -1) {
				Value::Set(KeyVal_[valueIndex], input);
			} else {
				int nSize = (int)KeyVal_.size() + 2;
				KeyVal_.resize(nSize);
				KeyVal_[nSize - 2] = k;
				Value::Set(KeyVal_[nSize - 1], input);
			}
		}

		template<typename T>
		inline void Set(T input, JObject key) {
			throw std::runtime_error("The key can't be JObject type.");
		}

		template<typename T>
		inline void Set(T input, JArray key) {
			throw std::runtime_error("The key can't be JArray type.");
		}

		template<typename T, typename K>
		inline void Set(T input, K key) {
			std::ostringstream oss;
			oss << key;
			std::string strKey(oss.str());
			Set(input, strKey);
		}

		bool SetAt(const Json& input, int index) {
			if(index < 0 || index >= (int)KeyVal_.size()) {
				return false;
			}
			if (JSON_TYPE_NIL == json_type_) {
				json_type_ = JSON_TYPE_ARRAY;
			} else {
				assert(json_type_ == JSON_TYPE_ARRAY);
			}
			Value::Push(KeyVal_[index], input);
			return true;
		}

		void Set(const Json& input) {
			if (JSON_TYPE_NIL == json_type_) {
				json_type_ = JSON_TYPE_ARRAY;
			} else {
				assert(json_type_ == JSON_TYPE_ARRAY);
			}
			KeyVal_.resize(KeyVal_.size() + 1);
			Value::Push(KeyVal_.back(), input);
		}

		void Set(const Json& input, const char* k) {
			if (JSON_TYPE_NIL == json_type_) {
				json_type_ = JSON_TYPE_OBJECT;
			} else {
				assert(json_type_ == JSON_TYPE_OBJECT);
			}
			int valueIndex = FindValueIndex(k);
			if (valueIndex > -1) {
				Value::Push(KeyVal_[valueIndex], input);
			} else {
				int nSize = (int)KeyVal_.size() + 2;
				KeyVal_.resize(nSize);
				KeyVal_[nSize - 2] = k;
				Value::Push(KeyVal_[nSize - 1], input);
			}
		}

		void Set(const Json& input, const std::string& k) {
			if (JSON_TYPE_NIL == json_type_) {
				json_type_ = JSON_TYPE_OBJECT;
			} else {
				assert(json_type_ == JSON_TYPE_OBJECT);
			}
			int valueIndex = FindValueIndex(k);
			if (valueIndex > -1) {
				Value::Push(KeyVal_[valueIndex], input);
			} else {
				int nSize = (int)KeyVal_.size() + 2;
				KeyVal_.resize(nSize);
				KeyVal_[nSize - 2] = k;
				Value::Push(KeyVal_[nSize - 1], input);
			}
		}

        std::string WriteJson() const;

		inline int Count() const { 
			return KeyVal_.size();
		}

		void Clear() {
			if (!KeyVal_.empty()) {
				KeyVal_.clear();
			}
			json_type_ = JSON_TYPE_NIL;
		}
	protected:
		int FindValueIndex(const std::string& strKey) {
			int nSize = (int)KeyVal_.size();
			for (int i = 0; i < nSize; i += 2) {
				if (KeyVal_[i] == strKey) {
					return i + 1;
				}
			}
			return -1;
		}

		inline int FindValueIndex(const char* szKey) {
			std::string strKey(szKey);
			return FindValueIndex(strKey);
		}

	protected:
		std::vector<std::string> KeyVal_;
		int json_type_;
	};

	inline bool Json::GetAt(JObject& outVal, int index) const {
		if(index < 0 || index >= (int)KeyVal_.size()) {
			return false;
		}
		ParseJson p;
		if (p.ParseObj(KeyVal_[index], outVal.KeyVal_)) {
			return true;
		}
		return false;
	}

	inline bool Json::Get(JObject& outVal, const char* key) const {
		return Get(outVal,std::string(key));
	}

	inline bool Json::Get(JObject& outVal, const std::string& key) const {
		std::string val;
		if (!Get<std::string>(val, key)) {
			return false;
		}
		ParseJson p;
		if (p.ParseObj(val, outVal.KeyVal_)) {
			return true;
		}
		return false;
	}

	inline bool Json::GetAt(JArray& outVal, int index) const {
		if(index < 0 || index >= (int)KeyVal_.size()) {
			return false;
		}
		ParseJson p;
		return p.ParseArray(KeyVal_[index], outVal.KeyVal_);
	}

	inline bool Json::Get(JArray& outVal, const char* key) const {
		return Get(outVal, std::string(key));
	}

    inline bool Json::Get(JArray& outVal, const std::string& key) const {
		std::string val;
		if (!Get<std::string>(val, key)) {
			return false;
		}
        ParseJson p;
        return p.ParseArray(val, outVal.KeyVal_);
    }

    inline std::ostream & operator << (std::ostream& os, const Json& ob)
	{
		os << ob.WriteJson();
		return os;
	}

    inline std::string Json::WriteJson() const
	{
		std::ostringstream oss;
		if (JSON_TYPE_ARRAY == json_type_) {
			int size = KeyVal_.size();
			if (size > 0) {
				oss << '[';
				int i = 0;
				int count = size - 1;
				for (; i < count; ++i) {
					oss << "\"" << KeyVal_[i] << "\",";
				}
				oss << "\"" << KeyVal_[i] << "\"";
				oss << ']';
			}
		} else {
			int size = KeyVal_.size();
			if (size > 1) {
				oss << '{';
				int i = 0;
				int count = size - 2;
				for (; i < count; i += 2) {
					oss << "\"" << KeyVal_[i] << "\"" << ":" << "\"" << KeyVal_[i + 1] << "\",";
				}
				oss << "\"" << KeyVal_[i] << "\"" << ":" << "\"" << KeyVal_[i + 1] << "\"";
				oss << '}';
			}
		}
		return oss.str();
	}

}  // end namespace

#endif  // TINY_JSON_H
