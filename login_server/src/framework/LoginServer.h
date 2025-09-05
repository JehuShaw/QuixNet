/* 
 * File:   LoginServer.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef LOGINSERVER_H
#define	LOGINSERVER_H

#include <vector>
#include <map>
#include "NodeDefines.h"
#include "ThreadBase.h"
#include "rpcz.hpp"
#include "AutoPointer.h"
#include "TCPPacketInterface.h"
#include "ControlCentreServiceImp.h"
#include "LoginServiceImp.h"
#include "Singleton.h"
#include "IServerRegister.h"
#include "HttpThreadHold.h"


class CLoginServer
	: public thd::ThreadBase
	, public util::Singleton<CLoginServer>
{
public:
	CLoginServer();

	~CLoginServer();

    bool Init(int argc, char** argv);

    void Dispose();

    virtual bool OnRun();

	virtual void OnShutdown() {}

    bool InsertSocketId(const ntwk::SocketID& socketId, uint64_t account);

	bool SendToClient(uint64_t account, const ::node::DataPacket& message);

	bool SendToClientBySocketID(const ntwk::SocketID& socketId, const ::node::DataPacket& message);

	bool IsLoggedAccount(const ntwk::SocketID& socketId);

	uint64_t GetLoggedAccount(const ntwk::SocketID& socketId);

	bool FindSocketId(uint64_t account, ntwk::SocketID& outSocketId);

	void CloseSocket(ntwk::SocketID& socketId, int nWhy);

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
		uint16_t u16ServerRegion);

	void DisconnectControlServant();

	void KeepControlServantRegister(std::string& connect, volatile bool& bRun);

private:
	void InsertSocketId(const ntwk::SocketID& socketId);

	uint64_t RemoveSocketId(const ntwk::SocketID& socketId);

	bool FindAccount(const ntwk::SocketID& socketId, uint64_t& outAccount);

	bool SendToClient(const ntwk::SocketID& socketId, const ::node::DataPacket& message);

    void LoginTimeout(ntwk::SocketID& socketId, int& nWhy);

private:
    volatile bool m_isStarted;

	util::CAutoPointer<rpcz::server> m_pServer;
	util::CAutoPointer<CControlCentreServiceImp> m_pControlService;
	util::CAutoPointer<CLoginServiceImp> m_pWorkerService;
	std::string m_strProcessPath;

	typedef std::vector<uint64_t> INTERVAL_KEYS_T;
	INTERVAL_KEYS_T m_keepRegTimerKeys;

	ntwk::TCPPacketInterface m_tcpServer;
	thd::CSpinLock m_tcpSpinLock;

    struct ClientData {
        uint64_t account;
        uint64_t timerId;
    };
	typedef std::map<uint64_t, ntwk::SocketID> ACCOUNT_TO_SOCKETID_T;
	typedef std::map<ntwk::SocketID, struct ClientData> SOCKETID_TO_CLIENT_T;
	ACCOUNT_TO_SOCKETID_T m_socketIds;
	SOCKETID_TO_CLIENT_T m_clients;
	thd::CSpinRWLock m_socketIdsLock;

	util::CAutoPointer<IServerRegister> m_pServerRegister;

	CHttpThreadHold m_httpThreadHold;
};

#endif	/* LOGINSERVER_H */

