/* 
 * File:   MasterServer.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef _MASTERSERVER_H_
#define	_MASTERSERVER_H_

#include <vector>
#include <string>
#include "NodeDefines.h"
#include "CThreads.h"
#include "rpcz.hpp"
#include "AutoPointer.h"
#include "MasterServiceImp.h"
#include "WorkerServiceImp.h"
#include "Singleton.h"
#include "IServerRegister.h"

class CMasterServer
	: public thd::CThread
	, public util::Singleton<CMasterServer>
{
public:
	CMasterServer();

	~CMasterServer();

    bool Init(int argc, char** argv);

    void Dispose();

    virtual bool Run();

	void OnServerPlay();

	static void RemoveAutoPlayTimer();

	const std::string& GetProcessPath() const {
		return m_strProcessPath;
	}

private:
	void DisposeKeepRegTimer();
	void ConnectServers(const std::string& strServerName,
		const std::string& strBind, unsigned short u16ServerId);
	void DisconnectServers();
	void KeepServersRegister(std::string& connect, volatile bool& bRun);

	friend class CMasterModule; // only call RegistControlCentre();
	void RegistControlCentre();
	void UnregistControlCentre();
	void KeepControlCentreRegister(volatile bool& bRun);

	static void SetAutoPlayTimer();	
	static void AutoPlayCallback();

private:
    volatile bool m_isStarted;

	util::CAutoPointer<rpcz::server> m_pServer;
	util::CAutoPointer<CMasterServiceImp> m_pMasterService;
	util::CAutoPointer<CWorkerServiceImp> m_pWorkerService;

	typedef std::vector<uint64_t> INTERVAL_KEYS_T;
	INTERVAL_KEYS_T m_keepRegTimerKeys;
	std::string m_strProcessPath;
	uint64_t m_keepCentreTimerKey;
	volatile bool m_bRegistCentre;
	volatile static uint64_t s_autoPlayTimerKey;

	util::CAutoPointer<IServerRegister> m_pServerRegister;
};

#endif	/* _MASTERSERVER_H_ */

