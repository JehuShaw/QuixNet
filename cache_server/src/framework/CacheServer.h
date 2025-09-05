/* 
 * File:   CacheServer.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef CACHESERVER_H
#define	CACHESERVER_H

#include <string>
#include <vector>
#include "NodeDefines.h"
#include "ThreadBase.h"
#include "rpcz.hpp"
#include "AutoPointer.h"
#include "CacheServiceImp.h"
#include "ControlCentreServiceImp.h"
#include "Singleton.h"
#include "IServerRegister.h"

class CCacheServer
	: public thd::ThreadBase
	, public util::Singleton<CCacheServer>
{
public:
	CCacheServer();

	~CCacheServer();

    bool Init(int argc, char** argv);

    void Dispose();

    virtual bool OnRun();

	virtual void OnShutdown() {}

private:
	void DisposeKeepRegTimer();

	void ConnectServers(
		const std::string& strServerName,
		const std::string& endPoint,
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
		const std::string& endPoint,
		uint32_t uServerId,
		uint16_t u16ServerRegion);

	void DisconnectControlServant();

	void KeepControlServantRegister(
		std::string& connect,
		volatile bool& bRun);

private:
    volatile bool m_isStarted;

	util::CAutoPointer<rpcz::server> m_pServer;
	util::CAutoPointer<CCacheServiceImp> m_pCacheService;
	util::CAutoPointer<CControlCentreServiceImp> m_pControlService;

	typedef std::vector<uint64_t> INTERVAL_KEYS_T;
	INTERVAL_KEYS_T m_keepRegTimerKeys;
	std::string m_strProcessPath;

	util::CAutoPointer<IServerRegister> m_pServerRegister;
};

#endif	/* CACHESERVER_H */

