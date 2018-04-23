/*
 * File:   ServantServiceImp.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#pragma once

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
        const std::string& strBind, uint16_t servantId);

	~CServantServiceImp(void);

	virtual void RegisterModule(const ::node::RegisterRequest& request,
		::rpcz::reply< ::node::OperateResponse> response);

	virtual void RemoveModule(const ::node::RemoveRequest& request,
		::rpcz::reply< ::node::OperateResponse> response);

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

	virtual void CreateUserId(const ::node::CreateIdRequest& request,
		::rpcz::reply< ::node::CreateIdResponse> response);

	virtual void CheckUserId(const ::node::CheckIdRequest& request,
		::rpcz::reply< ::node::CheckIdResponse> response);

	virtual void UpdateUserRegion(const ::node::UpdateRegionRequest& request,
		::rpcz::reply< ::node::UpdateRegionResponse> response);

	virtual void CacheServerStore(const ::node::CacheStoreRequest& request,
		::rpcz::reply< ::node::CacheStoreResponse> response);

	static bool IsServerAlive(uint16_t serverId) {
		return FindServerId(serverId);
	}

private:
	inline static bool InsertServerId(uint16_t serverId) {
		thd::CScopedWriteLock wrLock(m_wrLock);
		std::pair<SERVER_ID_SET_T::iterator, bool> pairIB(
		m_serverIds.insert(serverId));
		return pairIB.second;
	}

	inline static bool FindServerId(uint16_t serverId) {
		thd::CScopedReadLock rdLock(m_wrLock);
		return m_serverIds.end() != m_serverIds.find(serverId);
	}

	inline static void EraseServerId(uint16_t serverId) {
		thd::CScopedWriteLock wrLock(m_wrLock);
		m_serverIds.erase(serverId);
	}

	static void ClearAllTimer();

	int RegisterToMaster(
		const std::string& endPoint,
		uint16_t servantId,
		const std::string& serverName,
		uint16_t serverId,
		uint16_t serverRegion,
		const std::string& projectName,
        const std::string& acceptAddress,
		const std::string& processPath,
		int deadline_ms = CALL_DEADLINE_MS);

	static int UnregisterToMaster(
		util::CAutoPointer<rpcz::rpc_channel>& pMasterChannel,
		const std::string& serverName, uint16_t serverId);

	bool KeepRegisterToMaster(
		const std::string& serverName,
		uint16_t serverId,
		uint32_t serverLoad,
		int32_t serverStatus,
		const std::string& serverState);

	static void KeepTimeoutCallback(uint16_t& serverId);

private:
	typedef std::set<uint16_t> SERVER_ID_SET_T;
	static SERVER_ID_SET_T m_serverIds;
	static thd::CSpinRWLock m_wrLock;

	util::CAutoPointer<rpcz::rpc_channel> m_pMasterChannel;
    std::string m_strBind;
	uint16_t m_servantId;
};

