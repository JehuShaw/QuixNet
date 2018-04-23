/* 
 * File:   LoginServer.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef _LOGINSERVER_H_
#define	_LOGINSERVER_H_

#include <vector>
#include <map>
#include "NodeDefines.h"
#include "CThreads.h"
#include "rpcz.hpp"
#include "AutoPointer.h"
#include "TCPPacketInterface.h"
#include "ControlCentreServiceImp.h"
#include "LoginServiceImp.h"
#include "Singleton.h"
#include "IServerRegister.h"


class CLoginServer
	: public thd::CThread
	, public util::Singleton<CLoginServer>
{
public:
	CLoginServer();

	~CLoginServer();

    bool Init(int argc, char** argv);

    void Dispose();

    virtual bool Run();

    bool InsertSocketId(uint64_t userId, int socketIdx);

	bool SendToClient(uint64_t userId, const ::node::DataPacket& message);

	bool SendToClientByIdx(unsigned int socketIdx, const ::node::DataPacket& message);

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

	void InsertSocketId(const ntwk::SocketID& socketId);

	uint64_t RemoveSocketId(const ntwk::SocketID& socketId);

	bool FindSocketId(uint64_t userId, ntwk::SocketID& outSocketId);

	bool FindUserId(unsigned int socketIdx, uint64_t& outUserId);

	bool SendToClient(const ntwk::SocketID& socketId, const ::node::DataPacket& message);

    void LoginTimeout(unsigned int& socketIdx);

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
        uint64_t userId;
        uint64_t timerId;
    };
	typedef std::map<uint64_t, ntwk::SocketID> USERID_TO_SOCKETID_T;
	typedef std::map<ntwk::SocketID, struct ClientData> SOCKETID_TO_CLIENT_T;
	USERID_TO_SOCKETID_T m_socketIds;
	SOCKETID_TO_CLIENT_T m_clients;
	thd::CSpinRWLock m_socketIdsLock;

	util::CAutoPointer<IServerRegister> m_pServerRegister;
};

#endif	/* _LOGINSERVER_H_ */

