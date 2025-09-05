/*
 * File:   MasterStubImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef MASTERSTUBIMP_H
#define	MASTERSTUBIMP_H

#include <map>
#include "controlcentre.rpcz.h"
#include "rpc_controller.hpp"
#include "NodeDefines.h"
#include "AgentMethod.h"
#include "SpinRWLock.h"
#include "json/json.h"
#include "CacheOperateHelper.h"
#include "TimestampManager.h"
#include "ValueStream.h"
#include "Singleton.h"

class CMasterStubImp
	: public util::Singleton<CMasterStubImp>
{
public:

	void KeepRegister(
		const std::string& serverName,
		const std::string& endPoint,
		uint32_t serverId,
		int32_t serverStatus,
		uint32_t serverLoad)
	{
		util::CValueStream datas;
		CRequestStoredProcs refreshRequest;
		datas.Serialize(NODE_STATE_REFRESH);
		refreshRequest.SetKey(datas);

		datas.Reset();
		std::string strState;
		GetState(strState);
		char szTimeBuf[TIME_BUFFER_SIZE] = {'\0'};
		evt::CTimestampManager::Pointer()->GetLocalDateTimeStr(szTimeBuf, sizeof(szTimeBuf));
		datas.Serialize(serverId);
		datas.Serialize(serverName);
		datas.Serialize(serverLoad);
		datas.Serialize(serverStatus);
		datas.Serialize(strState);
		datas.Serialize(szTimeBuf);
		refreshRequest.SetParams(datas);

		CResponseRows refreshResponse;
		McDBStoredProcHash32Key(serverId, refreshRequest, refreshResponse);
	}

	inline void AddInfoMethod(std::string strName,
		util::CAutoPointer<evt::MethodRSBase> pInfoMethod) {

		thd::CScopedWriteLock scopedWriteLock(m_rwInfoLock);
		if(pInfoMethod.IsInvalid()) {
			return;
		}
		m_infoMethods[strName] = pInfoMethod;
	}

	inline bool RemoveInfoMethod(std::string strName) {

		thd::CScopedWriteLock scopedWriteLock(m_rwInfoLock);
		INFO_METHOD_MAP_T::iterator it = m_infoMethods.find(strName);
		if(m_infoMethods.end() != it) {
			m_infoMethods.erase(it);
			return true;
		}
		return false;
	}

	inline void ClearInfoMethod() {

		thd::CScopedWriteLock scopedWriteLock(m_rwInfoLock);
		m_infoMethods.clear();
	}

private:
	inline void GetState(std::string& outJson) {
		Json::Value json;
		GetJsonValue(json);
		if(json.empty()) {
			return;
		}
		Json::FastWriter fastWriter;
		outJson = fastWriter.write(json);
	}

	void GetJsonValue(Json::Value& json) {

		thd::CScopedReadLock scopedReadLock(m_rwInfoLock);
		INFO_METHOD_MAP_T::iterator it = m_infoMethods.begin();
		for(; m_infoMethods.end() != it; ++it) {
			json[it->first] = it->second->Invoke();
		}
	}

private:
	typedef std::map<std::string, util::CAutoPointer<evt::MethodRSBase> > INFO_METHOD_MAP_T;
	INFO_METHOD_MAP_T m_infoMethods;
	thd::CSpinRWLock m_rwInfoLock;
};

#endif /* MASTERSTUBIMP_H */