/*
 * File:   MasterServiceImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef MASTERSERVERIMP_H
#define	MASTERSERVERIMP_H

#include <set>
#include <string>
#include "Common.h"
#include "controlcentre.rpcz.h"
#include "SpinRWLock.h"
#include "ScopedRWLock.h"

class CMasterServiceImp : public ::node::ControlCentreService
{
public:
	CMasterServiceImp(
		const std::string& strCurPath);

	virtual void RegisterModule(const ::node::RegisterRequest& request,
		::rpcz::reply< ::node::RegisterResponse> response);

	virtual void RemoveModule(const ::node::RemoveRequest& request,
		::rpcz::reply< ::node::RemoveResponse> response);

	virtual void KeepRegister(const ::node::KeepRegisterRequest& request,
		::rpcz::reply< ::node::KeepRegisterResponse> response);

	virtual void UserLogin(const ::node::UserLoginRequest& request,
		::rpcz::reply< ::node::ControlCentreVoid> response);

	virtual void UserLogout(const ::node::UserLogoutRequest& request,
		::rpcz::reply< ::node::ControlCentreVoid> response);

	virtual void GetLowLoadNode(const ::node::LowLoadNodeRequest& request,
		::rpcz::reply< ::node::LowLoadNodeResponse> response);

	virtual void GetRegionLowLoad(const ::node::RegionLowLoadRequest& request,
		::rpcz::reply< ::node::RegionLowLoadResponse> response);

    virtual void GetNodeList(const ::node::NodeListRequest& request,
        ::rpcz::reply< ::node::NodeListResponse> response);

	virtual void GetUsers(const ::node::GetUserRequest& request,
		::rpcz::reply< ::node::GetUserResponse> response);

	virtual void CreateUser(const ::node::CreateUserRequest& request,
		::rpcz::reply< ::node::CreateUserResponse> response);

	virtual void CheckUser(const ::node::CheckUserRequest& request,
		::rpcz::reply< ::node::CheckUserResponse> response);

	virtual void UpdateUser(const ::node::UpdateUserRequest& request,
		::rpcz::reply< ::node::UpdateUserResponse> response);

	virtual void DeleteUser(const ::node::DeleteUserRequest& request,
		::rpcz::reply< ::node::DeleteUserResponse> response);

	virtual void SeizeServer(const ::node::SeizeRequest& request,
		::rpcz::reply< ::node::SeizeResponse> response);

	virtual void FreeServer(const ::node::FreeRequest& request,
		::rpcz::reply< ::node::FreeResponse> response);

	virtual void GenerateGuid(const ::node::ControlCentreVoid& request,
		::rpcz::reply< ::node::GuidResponse> response);

	static bool IsServerAlive(uint32_t serverId, uint16_t serverType) {
		uint64_t nKey = GetServerKey(serverId, serverType);
		return FindServerKey(nKey);
	}

	static void SeizeServerLocal(
		uint32_t& outAgentId,
		uint32_t& outMapId,
		uint64_t userId,
		const std::string& agentServerName);

	static void FreeServerLocal(uint64_t userId);

	void OnServerPlay(void);

	static void ClearAllTimer();

private:
	inline static bool InsertServerKey(uint64_t serverKey) {
		thd::CScopedWriteLock wrLock(m_wrLock);
		std::pair<SERVER_ID_SET_T::iterator, bool> pairIB(
		m_serverIds.insert(serverKey));
		return pairIB.second;
	}

	inline static bool FindServerKey(uint64_t serverKey) {
		thd::CScopedReadLock rdLock(m_wrLock);
		return m_serverIds.end() != m_serverIds.find(serverKey);
	}

	inline static void EraseServerKey(uint64_t serverKey) {
		thd::CScopedWriteLock wrLock(m_wrLock);
		m_serverIds.erase(serverKey);
	}

private:
	static void AddAllNode();

	static void DelAllNode();

	static void KeepTimeoutCallback(
		const std::string& serverName,
		uint32_t& serverId,
		uint16_t& serverType,
		volatile int32_t*& pCacheCount);

private:
	typedef std::set<uint64_t> SERVER_ID_SET_T;
	static SERVER_ID_SET_T m_serverIds;
	static thd::CSpinRWLock m_wrLock;
	volatile int32_t m_cacheCount;
};

#endif /* MASTERSERVERIMP_H */