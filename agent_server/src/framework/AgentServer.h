/* 
 * File:   AgentServer.h
 * Author: Jehu Shaw
 *
 * Created on 2010_10_7 PM 11:45
 */

#ifndef _AGENTSERVER_H_
#define	_AGENTSERVER_H_

#include <vector>
#include <string>
#include "NodeDefines.h"
#include "CThreads.h"
#include "rpcz.hpp"
#include "AutoPointer.h"
#include "ControlCentreServiceImp.h"
#include "AgentServiceImp.h"
#include "AgentLogic.h"
#include "Singleton.h"
#include "IServerRegister.h"

class CAgentServer
	: public thd::CThread
	, public util::Singleton<CAgentServer>
{
public:
	CAgentServer();

	~CAgentServer();

    bool Init(int argc, char** argv);
    void Dispose();

    virtual bool Run();

	inline bool SendToClient(uint64_t userId, const ::node::DataPacket& message) {
        return m_agentLogic.SendToClient(userId, message);
    }

    inline void CloseClient(uint64_t userId) {
        m_agentLogic.CloseClient(userId);
    }

	inline bool KickLogged(uint64_t userId) {
		return m_agentLogic.KickLogged(userId);
	} 

private:
	void DisposeKeepRegTimer();
	void ConnectServers(
		const std::string& strServerName,
		const std::string& strBind,
		uint16_t u16ServerId,
		uint16_t u16ServerRegion);
	void DisconnectServers();
	void KeepServersRegister(std::string& connect, volatile bool& bRun);

	void ConnectControlServant(
		const std::string& strServant,
		const std::string& strServerName,
		const std::string& strBind,
		uint16_t u16ServerId,
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

#endif	/* _AGENTSERVER_H_ */

