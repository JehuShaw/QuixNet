/* 
 * File:   MasterServer.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef MASTERSERVER_H
#define	MASTERSERVER_H

#include <vector>
#include <string>
#include "NodeDefines.h"
#include "ThreadBase.h"
#include "rpcz.hpp"
#include "AutoPointer.h"
#include "MasterServiceImp.h"
#include "WorkerServiceImp.h"
#include "MasterLogic.h"
#include "Singleton.h"
#include "IServerRegister.h"
#include "HttpThreadHold.h"

class CMasterServer
	: public thd::ThreadBase
	, public util::Singleton<CMasterServer>
{
public:
	CMasterServer();

	~CMasterServer();

    bool Init(int argc, char** argv);

    void Dispose();

    virtual bool OnRun();

	virtual void OnShutdown() {}

	void OnServerPrepare();

	void OnServerPlay();

	static void RemoveAutoPlayTimer();

	const std::string& GetProcessPath() const {
		return m_strProcessPath;
	}

private:

	void DisposeKeepRegTimer();

	void ConnectServers(const std::string& strServerName,
		const std::string& strBind, uint32_t serverId);

	void DisconnectServers();

	void KeepServersRegister(
		std::string& connect,
		volatile bool& bRun,
		volatile long& nTimeoutCount);

	friend class CMasterModule; // only call RegistControlCentre();
	void RegistControlCentre();

	void UnregistControlCentre();

	void KeepControlCentreRegister(volatile bool& bRun);

	static void SetAutoPlayTimer();

	static void PrepareCallback();
	static void PlayCallback();

private:
    volatile bool m_isStarted;

	util::CAutoPointer<rpcz::server> m_pServer;
	util::CAutoPointer<CMasterServiceImp> m_pMasterService;
	util::CAutoPointer<CWorkerServiceImp> m_pWorkerService;

	typedef std::vector<uint64_t> INTERVAL_KEYS_T;
	INTERVAL_KEYS_T m_keepRegTimerKeys;

	CMasterLogic m_masterLogic;
	std::string m_strProcessPath;
	uint64_t m_keepCentreTimerKey;
	volatile bool m_bRegistCentre;
	volatile static uint64_t s_prepareTimerKey;
	volatile static uint64_t s_autoPlayTimerKey;

	util::CAutoPointer<IServerRegister> m_pServerRegister;

	CHttpThreadHold m_httpThreadHold;
};

#endif	/* MASTERSERVER_H */

