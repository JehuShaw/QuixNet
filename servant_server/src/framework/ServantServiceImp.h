/*
 * File:   ServantServiceImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef SERVANTSERVERIMP_H
#define	SERVANTSERVERIMP_H

#include <set>
#include <string>
#include "controlcentre.rpcz.h"
#include "SpinRWLock.h"
#include "ScopedRWLock.h"
#include "AutoPointer.h"
#include "NodeDefines.h"

class CServantServiceImp: public ::node::ControlCentreService
{
public:
	CServantServiceImp(
		util::CAutoPointer<rpcz::rpc_channel> pMasterChannel,
        const std::string& endPoint, uint32_t servantId);

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

	virtual void GetEndPointFromServant(const ::node::EndPointRequest& request,
		::rpcz::reply< ::node::EndPointResponse> response);

	virtual void SeizeServer(const ::node::SeizeRequest& request,
		::rpcz::reply< ::node::SeizeResponse> response);

	virtual void FreeServer(const ::node::FreeRequest& request,
		::rpcz::reply< ::node::FreeResponse> response);

	virtual void GenerateGuid(const ::node::ControlCentreVoid& request,
		::rpcz::reply< ::node::GuidResponse> response);

	static bool IsServerAlive(uint32_t serverId) {
		return FindServerId(serverId);
	}

	static void ClearAllTimer();

private:
	inline static bool InsertServerId(uint32_t serverId) {
		thd::CScopedWriteLock wrLock(m_wrLock);
		std::pair<SERVER_ID_SET_T::iterator, bool> pairIB(
		m_serverIds.insert(serverId));
		return pairIB.second;
	}

	inline static bool FindServerId(uint32_t serverId) {
		thd::CScopedReadLock rdLock(m_wrLock);
		return m_serverIds.end() != m_serverIds.find(serverId);
	}

	inline static void EraseServerId(uint32_t serverId) {
		thd::CScopedWriteLock wrLock(m_wrLock);
		m_serverIds.erase(serverId);
	}

	int RegisterToMaster(
		const std::string& endPoint,
		uint32_t servantId,
		const std::string& serverName,
		uint32_t serverId,
		uint16_t serverRegion,
		const std::string& projectName,
        const std::string& acceptAddress,
		const std::string& processPath,
		int deadline_ms = CALL_DEADLINE_MS);

	static int UnregisterToMaster(
		util::CAutoPointer<rpcz::rpc_channel>& pMasterChannel,
		const std::string& serverName, uint32_t serverId);

	int KeepRegisterToMaster(
		const std::string& serverName,
		uint32_t serverId,
		uint32_t serverLoad,
		int32_t serverStatus,
		const std::string& serverState);

	static void KeepTimeoutCallback(uint32_t& serverId);

private:
	typedef std::set<uint32_t> SERVER_ID_SET_T;
	static SERVER_ID_SET_T m_serverIds;
	static thd::CSpinRWLock m_wrLock;

	util::CAutoPointer<rpcz::rpc_channel> m_pMasterChannel;
    std::string m_endPoint;
	uint32_t m_servantId;
};

#endif /* SERVANTSERVERIMP_H */