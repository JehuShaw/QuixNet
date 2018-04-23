/* 
 * File:   ServantServer.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef _SERVANTSERVER_H_
#define	_SERVANTSERVER_H_

#include <vector>
#include <string>
#include "NodeDefines.h"
#include "CThreads.h"
#include "rpcz.hpp"
#include "AutoPointer.h"
#include "WorkerServiceImp.h"
#include "ServantServiceImp.h"
#include "Singleton.h"
#include "IServerRegister.h"

class CServantServer
	: public thd::CThread
	, public util::Singleton<CServantServer>
{
public:
	CServantServer();

	~CServantServer();

    bool Init(int argc, char** argv);

    void Dispose();

    virtual bool Run();

private:
	void DisposeKeepRegTimer();

	void ConnectControlMaster(
		const std::string& strMaster, 
		const std::string& strServerName,
		const std::string& strBind, 
		uint16_t u16ServerId);

	void DisconnectControlMaster();

	void KeepControlMasterRegister(
		std::string& connect,
		volatile bool& bRun);

private:
    volatile bool m_isStarted;

	util::CAutoPointer<rpcz::server> m_pServer;
	util::CAutoPointer<CServantServiceImp> m_pControlService;
	util::CAutoPointer<CWorkerServiceImp> m_pWorkerService;

	typedef std::vector<uint64_t> INTERVAL_KEYS_T;
	INTERVAL_KEYS_T m_keepRegTimerKeys;
	std::string m_strProcessPath;

	util::CAutoPointer<IServerRegister> m_pServerRegister;
};

#endif	/* _SERVANTSERVER_H_ */

