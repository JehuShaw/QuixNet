/* 
 * File:   GameClient.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef _GAMECLIENT_H_
#define	_GAMECLIENT_H_

#include <set>
#include <map>

#include "NodeDefines.h"
#include "CThreads.h"
#include "AutoPointer.h"
#include "TCPLinkInterface.h"
#include "AgentMethod.h"
#include <google/protobuf/message.h>
#include "Singleton.h"

namespace node {
	class DataPacket;
}

class CGameClient
	: public thd::CThread
	, public util::Singleton<CGameClient>
{
public:
	CGameClient();

	~CGameClient();

    bool Init();
    void Dispose();

    virtual bool Run();

	void LoginWebSuccess(const char* loginIp, uint64_t account, const char* sessionKey);
	void LoginWebFail();

	void LoginProxy(const char* proxyIp, uint64_t account, uint64_t userId, const char* sessionKey);

	bool SendToServer(int cmd, const ::google::protobuf::Message& message);



private:
	void RegistModule();
	void UnregistModule();
	void RegisterCommand();
private:

	void LoginTimeout(const unsigned int& socketIdx);
	bool Connect(const char* szAddress, uint32_t u32PacketLimit);

private:
	int CommandDirectConnect(const util::CWeakPointer<evt::ArgumentBase>& arg);
	int CommandConnect(const util::CWeakPointer<evt::ArgumentBase>& arg);

	void LoginWeb(const char* szUserName, const char* szPassword);
private:
    volatile bool m_isStarted;

	ntwk::TCPLinkInterface m_tcpClient;
	ntwk::SocketID m_socketId;
};

#endif	/* _GAMECLIENT_H_ */

