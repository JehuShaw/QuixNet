/* 
 * File:   MasterLogic.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef _MASTERLOGIC_H
#define	_MASTERLOGIC_H

#include "NodeDefines.h"
#include "CThreads.h"
#include "AutoPointer.h"
#include "TCPLinkInterface.h"
#include "AgentMethod.h"
#include "msg_master_data.pb.h"
#include "ReferObject.h"


class CMasterLogic 
	: public thd::CThread
	, public ntwk::ILinkEvent
{
public:
	
    CMasterLogic();
    
	~CMasterLogic();

    bool Init(const std::string& strAddress,
        uint16_t usMaxLink, uint32_t u32MaxPacketSize);

    void Dispose();

    virtual bool Run();

    virtual void OnShutdown();

	bool SendToClient(uint32_t account, const ::node::MasterDataPacket& message);

    void CloseClient(uint32_t account);

    virtual void OnAccept();

    virtual void OnDisconnect();

    virtual void OnReceive();

    long GetLinkCount();

private:

	bool InsertSocketId(uint32_t account, const ntwk::SocketID& socketId);

	void InsertSocketId(const ntwk::SocketID& socketId);

	bool RemoveSocketId(const ntwk::SocketID& socketId, uint32_t& outAccount);

	bool FindSocketId(uint32_t account, ntwk::SocketID& outSocketId);

	bool FindUserId(unsigned int socketIdx, uint32_t& outAccount);

	bool SendToClient(const ntwk::SocketID& socketId, const ::node::MasterDataPacket& message);

    bool ReceivePacket(ntwk::SocketID& outSocketId, std::vector<::node::MasterDataPacket>& outPackets);

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

private:
	void HandleLogin(const ::node::MasterDataPacket& dataRequest, const ntwk::SocketID& socketId);
	void HandleRestart(const ::node::MasterDataPacket& dataRequest, const ntwk::SocketID& socketId);
	void HandleAutoRestart(const ::node::MasterDataPacket& dataRequest, const ntwk::SocketID& socketId);
	void HandleShutdown(const ::node::MasterDataPacket& dataRequest, const ntwk::SocketID& socketId);
	void HandleErase(const ::node::MasterDataPacket& dataRequest, const ntwk::SocketID& socketId);
	void HandleDefault(const ::node::MasterDataPacket& dataRequest, const ntwk::SocketID& socketId);
	void HandleLogout(const ntwk::SocketID& socketId);

    int HandleSessionResult(const util::CWeakPointer<evt::ArgumentBase>& arg);

private:

	ntwk::TCPLinkInterface m_tcpServer;

    struct ClientData {
        uint32_t account;
        long index;
        uint64_t timerId;
    };
	typedef std::map<uint32_t, ntwk::SocketID> ACCOUNT_TO_SOCKETID_T;
	typedef std::map<ntwk::SocketID, struct ClientData> SOCKETID_TO_CLIENT_T;
    typedef std::vector<ntwk::SocketID> SOCKETID_SET_T;
	ACCOUNT_TO_SOCKETID_T m_socketIds;
	SOCKETID_TO_CLIENT_T m_clients;
    SOCKETID_SET_T m_socketIdSet;
	thd::CSpinRWLock m_socketIdsLock;
    volatile unsigned long m_sktIdxCount;

	util::CReferObject<CMasterLogic> m_pThis;
};

#endif	/* _MASTERLOGIC_H */

