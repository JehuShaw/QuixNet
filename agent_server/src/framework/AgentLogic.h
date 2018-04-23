/* 
 * File:   AgentLogic.h
 * Author: Jehu Shaw
 *
 * Created on 2010_10_6 PM 2:45
 */

#ifndef _AGENTLOGIC_H_
#define	_AGENTLOGIC_H_


#include "NodeDefines.h"
#include "CThreads.h"
#include "AutoPointer.h"
#include "TCPLinkInterface.h"
#include "worker.pb.h"
#include "ReferObject.h"

class CAgentLogic
	: public thd::CThread
	, public ntwk::ILinkEvent
{
public:
    CAgentLogic();

    ~CAgentLogic();

    bool Init(
		const std::string& strAddress,
        uint16_t usMaxLink,
        const std::string& strMasterConnect,
        uint32_t u32MaxPacketSize,
		const std::string& strBind,
		uint16_t u16ServerRegoin);

    void Dispose();

    virtual bool Run();

    virtual void OnShutdown();

	bool SendToClient(uint64_t userId, const ::node::DataPacket& message);

    void CloseClient(uint64_t userId);

    virtual void OnAccept();

    virtual void OnDisconnect();

    virtual void OnReceive();

    long GetLinkCount();

	bool KickLogged(uint64_t userId);

private:

	bool InsertSocketId(uint64_t userId, const ntwk::SocketID& socketId);

	void InsertSocketId(const ntwk::SocketID& socketId);

	bool RemoveSocketId(const ntwk::SocketID& socketId, uint64_t& outUserId);

	bool FindSocketId(uint64_t userId, ntwk::SocketID& outSocketId);

	bool FindUserId(unsigned int socketIdx, uint64_t& outUserId);

	bool SendToClient(const ntwk::SocketID& socketId, const ::node::DataPacket& message);

    bool ReceivePacket(ntwk::SocketID& outSocketId, std::vector<::node::DataPacket>& outPackets);

    inline long GetArrSocketIdSize() {
        thd::CScopedReadLock rdlock(m_socketIdsLock);
        return (long)m_socketIdSet.size();
    }

    inline ntwk::SocketID GetArrSocketId() {
        thd::CScopedReadLock rdlock(m_socketIdsLock);
        long nSize = m_socketIdSet.size();
        if(nSize < 1) {
            return ntwk::SocketID();
        }
        long socketIndex = atomic_inc(&m_sktIdxCount) % nSize;
        return m_socketIdSet[socketIndex];
    }

    void LoginTimeout(unsigned int& socketIdx);

    inline const std::string& GetServantConnect()const {
        return m_strServantConnect;
    }

	inline uint32_t GetServerRegoin()const {
		uint32_t u32ServerRegion = m_u16ServerRegoin;
		return AddFixedRegionFlag(u32ServerRegion);
	}

private:
	void HandleLogin(const ::node::DataPacket& dispatchRequest, const ntwk::SocketID& socketId);
	void HandleDefault(const ::node::DataPacket& dispatchRequest, const ntwk::SocketID& socketId);
	void HandleLogout(const ntwk::SocketID& socketId, ::node::DataPacket& dispatchResponse, int32_t nWhy = 0);

private:
	ntwk::TCPLinkInterface m_tcpServer;

    struct ClientData {
        uint64_t userId;
        long index;
        uint64_t timerId;
    };
	typedef std::map<uint64_t, ntwk::SocketID> USERID_TO_SOCKETID_T;
	typedef std::map<ntwk::SocketID, struct ClientData> SOCKETID_TO_CLIENT_T;
    typedef std::vector<ntwk::SocketID> SOCKETID_SET_T;
	USERID_TO_SOCKETID_T m_socketIds;
	SOCKETID_TO_CLIENT_T m_clients;
    SOCKETID_SET_T m_socketIdSet;
	thd::CSpinRWLock m_socketIdsLock;
    volatile unsigned long m_sktIdxCount;
    std::string m_strServantConnect;
	uint16_t m_u16ServerRegoin;
	std::string m_strBind;

	util::CReferObject<CAgentLogic> m_pThis;
};

#endif	/* _AGENTLOGIC_H_ */

