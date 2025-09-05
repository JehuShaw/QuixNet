/*
 * File:   ControlCentreServiceImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef CONTROLCENTRESERVICEIMP_H
#define	CONTROLCENTRESERVICEIMP_H

#include <set>
#include <string>
#include "controlcentre.rpcz.h"
#include "SpinRWLock.h"
#include "ScopedRWLock.h"

namespace mdl {
	class IModule;
}

class CControlCentreServiceImp: public ::node::ControlCentreService
{
public:
	typedef ::google::protobuf::RepeatedField< ::google::protobuf::uint64 > ROUTE_USERIDS_T;

	typedef mdl::IModule* (*CreateMethod)(
		const std::string& moduleName,
		const std::string& endPoint,
		uint32_t serverId,
		bool routeServer,
		uint64_t routeAddressId,
		const ROUTE_USERIDS_T& routeUserIds);

public:
	CControlCentreServiceImp(
		uint16_t serverRegion,
		CreateMethod createWorkerMethod = NULL,
		CreateMethod createCacheMethod = NULL);

	~CControlCentreServiceImp(void);

	virtual void RegisterModule(const ::node::RegisterRequest& request,
		::rpcz::reply< ::node::RegisterResponse> response);

	virtual void RemoveModule(const ::node::RemoveRequest& request,
		::rpcz::reply< ::node::RemoveResponse> response);

	virtual void KeepRegister(const ::node::KeepRegisterRequest& request,
		::rpcz::reply< ::node::KeepRegisterResponse> response);

	static bool IsServerAlive(uint32_t serverId) {
		return FindServerId(serverId);
	}

private:
	inline static bool InsertServerId(uint32_t serverId) {
		thd::CScopedWriteLock wrLock(s_wrLock);
		std::pair<SERVER_ID_SET_T::iterator, bool> pairIB(
		s_serverIds.insert(serverId));
		return pairIB.second;
	}

	inline static bool FindServerId(uint32_t serverId) {
		thd::CScopedReadLock rdLock(s_wrLock);
		return s_serverIds.end() != s_serverIds.find(serverId);
	}

	inline static void EraseServerId(uint32_t serverId) {
		thd::CScopedWriteLock wrLock(s_wrLock);
		s_serverIds.erase(serverId);
	}

	static void ClearAllTimer();

	static void KeepTimeoutCallback(uint32_t& serverId);

private:
	CreateMethod m_createWorkerMethod;
	CreateMethod m_createCacheMethod;
	typedef std::set<uint32_t> SERVER_ID_SET_T;
	static SERVER_ID_SET_T s_serverIds;
	static thd::CSpinRWLock s_wrLock;
	uint16_t m_serverRegion;
};

#endif /* CONTROLCENTRESERVICEIMP_H */
