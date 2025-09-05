/* 
 * File:   MasterLogic.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef MASTERLOGIC_H
#define	MASTERLOGIC_H

#include "NodeDefines.h"
#include "ThreadBase.h"
#include "AutoPointer.h"
#include "TCPLinkInterface.h"
#include "AgentMethod.h"
#include "msg_master_data.pb.h"
#include "ReferObject.h"
#include "CircleQueue.h"

class CMasterLogic 
	: public thd::ThreadBase
	, public ntwk::ILinkEvent
{
public:
	
    CMasterLogic();
    
	~CMasterLogic();

    bool Init(const std::string& strAddress,
        uint16_t usMaxLink, uint32_t u32MaxPacketSize);

    void Dispose();

    virtual bool OnRun();

    virtual void OnShutdown();

    virtual void OnAccept();

    virtual void OnDisconnect();

    virtual void OnReceive();

    long GetLinkCount();

private:

	void InsertSocketId(const ntwk::SocketID& socketId);

	bool RemoveSocketId(const ntwk::SocketID& socketId);

	bool ExistSocketId(const ntwk::SocketID& socketId);

	bool SendToClient(const ntwk::SocketID& socketId, const ::node::MasterDataPacket& message);

    void ReceivePacket();

	bool CheckAdminAuth(const std::string& adminName, const std::string& authCode);

	bool HandlePacket(
		const ::node::MasterDataPacket& dataRequest,
		const ntwk::SocketID& socketId);

private:
	void HandleBegin(
		const ::node::MasterDataPacket& dataRequest,
		::node::MasterDataPacket& dataResponse, 
		const ntwk::SocketID& socketId);

	void HandleStop(
		const ::node::MasterDataPacket& dataRequest,
		::node::MasterDataPacket& dataResponse,
		const ntwk::SocketID& socketId);

	void HandleRestart(
		const ::node::MasterDataPacket& dataRequest,
		::node::MasterDataPacket& dataResponse,
		const ntwk::SocketID& socketId);

	void HandleAutoRestart(
		const ::node::MasterDataPacket& dataRequest,
		::node::MasterDataPacket& dataResponse,
		const ntwk::SocketID& socketId);

	void HandleShutdown(
		const ::node::MasterDataPacket& dataRequest,
		::node::MasterDataPacket& dataResponse,
		const ntwk::SocketID& socketId);

	void HandleErase(
		const ::node::MasterDataPacket& dataRequest,
		::node::MasterDataPacket& dataResponse,
		const ntwk::SocketID& socketId);

	void HandleDefault(
		const ::node::MasterDataPacket& dataRequest,
		::node::MasterDataPacket& dataResponse,
		const ntwk::SocketID& socketId);

private:
	ntwk::TCPLinkInterface m_tcpServer;

	typedef std::set<ntwk::SocketID> SOCKETIDS_T;
	typedef thd::CCircleQueue<ntwk::SocketID> SOCKETID_QUEUE_T;

	SOCKETIDS_T m_clients;
	thd::CSpinRWLock m_socketIdsLock;
	util::CAutoPointer<SOCKETID_QUEUE_T> m_queSocketIds;

	volatile long m_runningCount;
	volatile long m_clientCount;

	util::CReferObject<CMasterLogic> m_pThis;
};

#endif	/* MASTERLOGIC_H */

