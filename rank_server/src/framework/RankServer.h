/* 
 * File:   RankServer.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef _RANKSERVER_H_
#define	_RANKSERVER_H_

#include <vector>
#include <string>
#include "NodeDefines.h"
#include "CThreads.h"
#include "rpcz.hpp"
#include "AutoPointer.h"
#include "WorkerServiceImp.h"
#include "ControlCentreServiceImp.h"
#include "Singleton.h"
#include "IServerRegister.h"

class CRankServer
	: public thd::CThread
	, public util::Singleton<CRankServer>
{
public:
	CRankServer();

	~CRankServer();

    bool Init(int argc, char** argv);

    void Dispose();

    virtual bool Run();

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
		uint16_t u16ServerRegion);
	void DisconnectControlServant();
	void KeepControlServantRegister(std::string& connect, volatile bool& bRun);

private:
    volatile bool m_isStarted;

	util::CAutoPointer<rpcz::server> m_pServer;
	util::CAutoPointer<CControlCentreServiceImp> m_pControlService;
	util::CAutoPointer<CWorkerServiceImp> m_pWorkerService;

	typedef std::vector<uint64_t> INTERVAL_KEYS_T;
	INTERVAL_KEYS_T m_keepRegTimerKeys;
	std::string m_strProcessPath;

	util::CAutoPointer<IServerRegister> m_pServerRegister;
};

#endif	/* _RANKSERVER_H_ */

