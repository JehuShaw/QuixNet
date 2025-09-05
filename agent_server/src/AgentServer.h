/* 
 * File:   AgentServer.h
 * Author: Jehu Shaw
 *
 * Created on 2010_10_7 PM 11:45
 */

#ifndef AGENTSERVER_H
#define	AGENTSERVER_H

#include <vector>
#include <string>
#include "NodeDefines.h"
#include "ThreadBase.h"
#include "rpcz.hpp"
#include "AutoPointer.h"
#include "ControlCentreServiceImp.h"
#include "AgentServiceImp.h"
#include "AgentLogic.h"
#include "Singleton.h"
#include "IServerRegister.h"
#include "ChannelManager.h"

class CAgentServer
	: public thd::ThreadBase
	, public util::Singleton<CAgentServer>
{
public:
	CAgentServer();

	~CAgentServer();

    bool Init(int argc, char** argv);
    void Dispose();

    virtual bool OnRun();

	virtual void OnShutdown() {}

	inline bool SendToClient(uint64_t userId, const std::string& bytes) {
        return m_agentLogic.SendToClient(userId, bytes);
    }

	inline void BroadcastToClient(const std::string& bytes, const std::set<uint64_t>& excludeId) {
		return m_agentLogic.BroadcastToClient(bytes, excludeId);
	}

    inline void CloseClient(uint64_t userId, int nWhy) {
        m_agentLogic.CloseClient(userId, nWhy);
    }

	inline void CloseAllClients(const std::set<uint64_t>& excludeId, int nWhy) {
		m_agentLogic.CloseAllClients(excludeId, nWhy);
	}

	inline bool KickLogged(uint64_t userId) {
		return m_agentLogic.KickLogged(userId);
	}

private:
	void DisposeKeepRegTimer();

	void ConnectServers(
		const std::string& strServerName,
		const std::string& strBind,
		uint32_t uServerId,
		uint16_t u16ServerRegion);

	void DisconnectServers();

	void KeepServersRegister(
		std::string& connect,
		volatile bool& bRun,
		volatile long& nTimeoutCount);

	void ConnectControlServant(
		const std::string& strServant,
		const std::string& strServerName,
		const std::string& strBind,
		uint32_t uServerId,
		uint16_t u16ServerRegion,
		const std::string& acceptAddress);

	void DisconnectControlServant();

	void KeepControlServantRegister(std::string& connect, volatile bool& bRun);

private:
    volatile bool m_isStarted;

	util::CAutoPointer<rpcz::server> m_pServer;
	util::CAutoPointer<CControlCentreServiceImp> m_pControlService;
	util::CAutoPointer<CAgentServiceImp> m_pWorkerService;

	typedef std::vector<uint64_t> INTERVAL_KEYS_T;
	INTERVAL_KEYS_T m_keepRegTimerKeys;

    CAgentLogic m_agentLogic;
	std::string m_strProcessPath;
	bool m_bConnectServant;

	util::CAutoPointer<IServerRegister> m_pServerRegister;
};

#endif	/* AGENTSERVER_H */

