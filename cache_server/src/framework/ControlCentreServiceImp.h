/*
 * File:   ControlCentreServiceImp.h
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

class DbEnv;

class CControlCentreServiceImp: public ::node::ControlCentreService
{
public:
	CControlCentreServiceImp(
		uint16_t serverRegion,
		uint16_t serverId,
		const std::string& strCurPath);

	~CControlCentreServiceImp(void);

	virtual void RegisterModule(const ::node::RegisterRequest& request,
		::rpcz::reply< ::node::OperateResponse> response);

	virtual void RemoveModule(const ::node::RemoveRequest& request,
		::rpcz::reply< ::node::OperateResponse> response);

	virtual void KeepRegister(const ::node::KeepRegisterRequest& request,
		::rpcz::reply< ::node::KeepRegisterResponse> response);

	static bool IsServerAlive(uint16_t serverId) {
		return FindServerId(serverId);
	}

private:
	inline static bool InsertServerId(uint16_t serverId) {
		thd::CScopedWriteLock wrLock(s_wrLock);
		std::pair<SERVER_ID_SET_T::iterator, bool> pairIB(
		s_serverIds.insert(serverId));
		return pairIB.second;
	}

	inline static bool FindServerId(uint16_t serverId) {
		thd::CScopedReadLock rdLock(s_wrLock);
		return s_serverIds.end() != s_serverIds.find(serverId);
	}

	inline static void EraseServerId(uint16_t serverId) {
		thd::CScopedWriteLock wrLock(s_wrLock);
		s_serverIds.erase(serverId);
	}

	static void ClearAllTimer();

	static void KeepTimeoutCallback(uint16_t& serverId);

private:
	typedef std::set<uint16_t> SERVER_ID_SET_T;
	static SERVER_ID_SET_T s_serverIds;
	static thd::CSpinRWLock s_wrLock;
	uint16_t m_serverRegion;
	uint16_t m_serverId;
	DbEnv* m_pDbEnv;
};

