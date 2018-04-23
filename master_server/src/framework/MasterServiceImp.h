/*
 * File:   MasterServiceImpEx.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#pragma once

#include <set>
#include <string>
#include "Common.h"
#include "controlcentre.rpcz.h"
#include "SpinRWLock.h"
#include "ScopedRWLock.h"

class DbEnv;

class CMasterServiceImp: public ::node::ControlCentreService
{
public:
	CMasterServiceImp(
		uint16_t serverId,
		const std::string& strCurPath);

	~CMasterServiceImp(void);

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

	static bool IsServerAlive(uint16_t serverId, uint16_t serverType) {
		uint32_t nKey = GetServerKey(serverId, serverType);
		return FindServerKey(nKey);
	}

	static uint16_t RouteGetServerId(const std::string& serverName, uint64_t userId);

	void OnServerPlay(void);

private:
	typedef union {
		uint32_t u32;
		struct
		{
			uint16_t serverId;
			uint16_t serverType;
		}h;
	} server_key_t;

	inline static uint32_t GetServerKey(uint16_t serverId, uint16_t serverType) {
		server_key_t key;
		key.h.serverId = serverId;
		key.h.serverType = serverType;
		return key.u32;
	}

	inline static bool InsertServerKey(uint32_t serverKey) {
		thd::CScopedWriteLock wrLock(m_wrLock);
		std::pair<SERVER_ID_SET_T::iterator, bool> pairIB(
		m_serverIds.insert(serverKey));
		return pairIB.second;
	}

	inline static bool FindServerKey(uint32_t serverKey) {
		thd::CScopedReadLock rdLock(m_wrLock);
		return m_serverIds.end() != m_serverIds.find(serverKey);
	}

	inline static void EraseServerKey(uint32_t serverKey) {
		thd::CScopedWriteLock wrLock(m_wrLock);
		m_serverIds.erase(serverKey);
	}

	static void ClearAllTimer();

private:
	static void AddAllNode();

	static void DelAllNode();

	static void KeepTimeoutCallback(
		const std::string& serverName,
		uint16_t& serverId,
		uint16_t& serverType,
		volatile int32_t*& pCacheCount);

private:
	void RouteCreateTable(const std::string& serverName, uint16_t serverId);
	void RouteDropTable(const std::string& serverName, uint16_t serverId);
	void RouteInsertRecord(const std::string& serverName, uint64_t userId, uint16_t serverId);
	void RouteRemoveRecord(const std::string& serverName, uint64_t userId);

private:
	typedef std::set<uint32_t> SERVER_ID_SET_T;
	static SERVER_ID_SET_T m_serverIds;
	static thd::CSpinRWLock m_wrLock;
	DbEnv* m_pDbEnv;
	volatile int32_t m_cacheCount;
	uint16_t m_serverId;
};

