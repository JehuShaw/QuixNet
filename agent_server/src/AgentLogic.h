/* 
 * File:   AgentLogic.h
 * Author: Jehu Shaw
 *
 * Created on 2010_10_6 PM 2:45
 */

#ifndef AGENTLOGIC_H
#define AGENTLOGIC_H


#include "NodeDefines.h"
#include "ThreadBase.h"
#include "AutoPointer.h"
#include "TCPLinkInterface.h"
#include "worker.pb.h"
#include "ReferObject.h"
#include "CircleQueue.h"

class CAgentLogic
	: public thd::ThreadBase
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
		const std::string& endPoint,
		uint16_t u16ServerRegoin);

    void Dispose();

    virtual bool OnRun();

    virtual void OnShutdown();

	bool SendToClient(uint64_t userId, const std::string& bytes);

	void BroadcastToClient(const std::string& bytes, const std::set<uint64_t>& excludeId);

    void CloseClient(uint64_t userId, int nWhy);

	void CloseAllClients(const std::set<uint64_t>& excludeId, int nWhy);

    virtual void OnAccept();

    virtual void OnDisconnect();

    virtual void OnReceive();

    long GetClientCount();

	bool KickLogged(uint64_t userId);

private:
	bool ReplaceSocketId(const ntwk::SocketID& socketId, uint64_t userId, uint32_t mapId);

	void InsertSocketId(const ntwk::SocketID& socketId);

	bool RemoveSocketId(const ntwk::SocketID& socketId, uint64_t& outUserId, uint32_t& outMapId);

	bool FindSocketId(uint64_t userId, ntwk::SocketID& outSocketId);

	bool FindSocketId(uint64_t userId, ntwk::SocketID& outSocketId, uint32_t& outMapId);

	bool FindUserId(const ntwk::SocketID& socketId, uint64_t& outUserId, uint32_t& outMapId);

	bool SendToClient(uint64_t userId, const ::node::DataPacket& message);

	bool SendToClient(const ntwk::SocketID& socketId, const ::node::DataPacket& message);

    void ReceivePacket();

	bool ChangeMapId(const ntwk::SocketID& socketId, uint32_t mapId);

	void IteratorSocketId(std::vector<ntwk::SocketID>& outSocketIds, const std::set<uint64_t>& excludeId);


    void OnLoginTimeout(ntwk::SocketID& socketId, int& nWhy);

    inline const std::string& GetServantConnect()const {
        return m_strServantConnect;
    }

	inline uint32_t GetServerRegoin()const {
		uint32_t u32ServerRegion = m_u16ServerRegoin;
		return AddFixedRegionFlag(u32ServerRegion);
	}

	void SetDelayLogout(const ntwk::SocketID& socketId, int nWhy);

	void OnLogoutCallback(ntwk::SocketID& socketId, int& nWhy);

	bool HandlePacket(const ::node::DataPacket& dispatchRequest, const ntwk::SocketID& socketId);

private:
	bool HandleKeepAlive(const ::node::DataPacket& dispatchRequest, const ntwk::SocketID& socketId);
	bool HandleLogin(const ::node::DataPacket& dispatchRequest, const ntwk::SocketID& socketId);
	bool HandleLogout(const ::node::DataPacket& dispatchRequest, const ntwk::SocketID& socketId);
	bool HandleSwitchNode(const ::node::DataPacket& dispatchRequest, const ntwk::SocketID& socketId);
	bool HandleSwitchMap(const ::node::DataPacket& dispatchRequest, const ntwk::SocketID& socketId);
	bool HandleDefault(const ::node::DataPacket& dispatchRequest, const ntwk::SocketID& socketId);

	inline void HandleLogout(const ntwk::SocketID& socketId, int nWhy = 0) {
		if (LOGOUT_OFFLINE == nWhy) {
			SetDelayLogout(socketId, nWhy);
		} else {
			OnLogoutCallback(const_cast<ntwk::SocketID&>(socketId), nWhy);
		}
	}

	void SwitchMapThroughNodes(
		const ntwk::SocketID& socketId,
		uint64_t userId,
		uint32_t oldMapId,
		uint32_t newMapId,
		::node::DataPacket& outResponse);

private:
	ntwk::TCPLinkInterface m_tcpServer;

	struct SClientData {
		ntwk::SocketID socketId;
		uint32_t mapId;
		uint64_t logoutTimerId;

		SClientData(const ntwk::SocketID& sktId, uint32_t mpId)
			: socketId(sktId), mapId(mpId), logoutTimerId(ID_NULL) {}
	};

    struct SPlayerData {
        uint64_t userId;
		uint32_t mapId;
        long index;
        uint64_t timerId;
    };
	typedef std::map<uint64_t, struct SClientData> USERID_TO_CLIENT_T;
	typedef std::map<ntwk::SocketID, struct SPlayerData> SOCKETID_TO_PLAYER_T;
	typedef thd::CCircleQueue<ntwk::SocketID> SOCKETID_QUEUE_T;

	USERID_TO_CLIENT_T m_mapClients;
	SOCKETID_TO_PLAYER_T m_mapPlayers;
	thd::CSpinRWLock m_socketIdsLock;
	util::CAutoPointer<SOCKETID_QUEUE_T> m_queSocketIds;

	volatile long m_runningCount;
	volatile long m_clientCount;
    std::string m_strServantConnect;
	uint16_t m_u16ServerRegoin;
	std::string m_endPoint;

	util::CReferObject<CAgentLogic> m_pThis;
};

#endif	/* AGENTLOGIC_H */

