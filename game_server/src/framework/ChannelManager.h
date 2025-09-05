/*
 * File:   ChannelManager.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 11:37
 */

#ifndef CHANNELMANAGER_H
#define CHANNELMANAGER_H

#include "rpc_channel.hpp"
#include "rpc_controller.hpp"
#include "WeakPointer.h"
#include "SpinRWLock.h"
#include "NodeDefines.h"
#include "PlayerBase.h"
#include "Singleton.h"
#include "PoolBase.h"
#include "ObjectSet.h"

class CRpczChannel : public util::IObject<uint64_t>, public util::PoolBase<CRpczChannel> {
public:
	CRpczChannel(uint64_t n64Key, const rpcz::rpc_channel* pChl, bool bDel = true)
		: m_n64Key(n64Key), m_pChannel(pChl, bDel), m_nUserCount(0), m_nAgentSize(0) {}

	CRpczChannel(const CRpczChannel& orig)
		: m_n64Key(orig.m_n64Key)
		, m_pChannel(orig.m_pChannel)
		, m_nUserCount(orig.m_nUserCount)
		, m_nAgentSize(0) {}

	CRpczChannel& operator=(const CRpczChannel& right) {
		m_pChannel = right.m_pChannel;
		m_nUserCount = right.m_nUserCount;
	}

	virtual uint64_t GetID() const {
		return m_n64Key;
	}

	inline const util::CAutoPointer<rpcz::rpc_channel>& GetChannel() const {
		return m_pChannel;
	}

	inline void SetAgentSize(long nSize) {
		atomic_xchg(&m_nAgentSize, nSize);
	}

	inline long GetAgentSize() const {
		return m_nAgentSize;
	}

private:
	inline long GetUserCount() const {
		return m_nUserCount;
	}

	inline void IncrUserCount() {
		atomic_inc(&m_nUserCount);
	}

	void DecrUserCount() {
		// Decrement until zero
		long nOld;
		long nNew;
		do {
			nOld = m_nUserCount;
			if (nOld > 0) {
				nNew = nOld - 1;
			} else {
				return;
			}
		} while (atomic_cmpxchg(&m_nUserCount, nOld, nNew) != nOld);
	}

private:
	friend class CChannelManager;
	util::CAutoPointer<rpcz::rpc_channel> m_pChannel;
	volatile long m_nUserCount;
	volatile long m_nAgentSize;
	uint64_t m_n64Key;
};

class CClientChannel : public util::PoolBase<CClientChannel> {
public:
	CClientChannel(): m_routeCount(0), m_timeoutCount(0) {}

	uint32_t GetRouteCount() const { return m_routeCount; }

	util::CAutoPointer<rpcz::rpc_channel> GetChannel() const {
		if (m_pServer.IsInvalid()) {
			return util::CAutoPointer<rpcz::rpc_channel>();
		}
		return m_pServer->GetChannel(); 
	}

	void SetPlayer(const util::CAutoPointer<CWrapPlayer>& pPlayer) {
		m_pPlayer = pPlayer;
	}

    void RemovePlayer() {
		if (m_pPlayer.IsInvalid()) {
			return;
		}
		m_pPlayer->Stop(true);
    }

	inline bool RemoveInvalidPlayer() {
		if (m_pPlayer.IsInvalid()) {
			return true;
		}
		if (m_pPlayer->IsObjectInvalid()) {
			m_pPlayer->Stop(true);
			return true;
		}
		return false;
	}

	const util::CAutoPointer<CWrapPlayer>& GetPlayer() const {
		return m_pPlayer;
	}

	uint64_t GetAddressId() const {
		if (m_pServer.IsInvalid()) {
			return 0;
		}
		return m_pServer->GetID();
	}

private:
	friend class CChannelManager;
	uint32_t m_routeCount;
	volatile unsigned long m_timeoutCount;
	util::CAutoPointer<CRpczChannel> m_pServer;

	util::CAutoPointer<CWrapPlayer> m_pPlayer;
};

struct UserIdAndChannel {
	uint64_t userId;
	util::CAutoPointer<rpcz::rpc_channel> pChannel;
};

class CChannelManager
	: public util::Singleton<CChannelManager>
{
public:
	typedef util::CObjectSet<uint64_t> RPCZ_CHANNEL_MAP_T;
	typedef std::map<uint64_t, util::CAutoPointer<CClientChannel> > CLIENT_CHANNEL_MAP_T;
	typedef std::map<uint32_t, uint64_t> AGENT_KEY_MAP_T;
	typedef std::set<uint64_t> CHANNEL_KEY_SET_T;
	typedef std::set<uint32_t> AGENT_ID_SET_T;
	typedef std::map<uint64_t, AGENT_ID_SET_T> CHLKEY_AGTID_MAP_T;
	typedef std::set<uint64_t> ROUTE_SERVID_SET_T;

	typedef util::CAutoPointer<CPlayerBase>(*CreatePlayerMethod)(uint64_t userId);

	CChannelManager() : m_userCount(0) {}

    void Dispose()
    {
        do {
            thd::CScopedWriteLock scopedWriteLock(m_rwClientLock);
            m_clientChannels.clear();
        } while(false);

        do {
            thd::CScopedWriteLock scopedWriteLock(m_rwRpczLock);
            m_rpczChannels.Clear();
        } while(false);
    }

	util::CAutoPointer<CRpczChannel> FindRpczChannel(uint64_t n64Key)
	{
		thd::CScopedReadLock scopedReadLock(m_rwRpczLock);
		if (m_rpczChannels.Empty()) {
			return util::CAutoPointer<CRpczChannel>();
		}
		return m_rpczChannels.Find(n64Key);
	}

	inline util::CAutoPointer<rpcz::rpc_channel> FindRpczChannel(const std::string& connect) {
		uint64_t n64Key = AddressToInteger(connect.c_str());
		if (0 == n64Key) {
			return util::CAutoPointer<rpcz::rpc_channel>();
		}
		util::CAutoPointer<CRpczChannel> pRpczChannel(FindRpczChannel(n64Key));
		if (pRpczChannel.IsInvalid()) {
			return util::CAutoPointer<rpcz::rpc_channel>();
		}
		return pRpczChannel->m_pChannel;
	}

	inline bool AddRpczChannel(const std::string& connect) {
		uint64_t n64Key = AddressToInteger(connect.c_str());
		if (0 == n64Key) {
			return false;
		}
		return !CreateRpczChannel(n64Key, connect).IsInvalid();
	}

	util::CAutoPointer<rpcz::rpc_channel> GetRpczChannel(const std::string& connect) {
		uint64_t n64Key = AddressToInteger(connect.c_str());
		if (0 == n64Key) {
			return util::CAutoPointer<rpcz::rpc_channel>();
		}
		util::CAutoPointer<CRpczChannel> pRpczChannel(FindRpczChannel(n64Key));
		if (pRpczChannel.IsInvalid()) {
			pRpczChannel = CreateRpczChannel(n64Key, connect);
		}
		return pRpczChannel->m_pChannel;
	}

	util::CAutoPointer<CRpczChannel> GetRpczChannelEx(const std::string& connect) {
		uint64_t n64Key = AddressToInteger(connect.c_str());
		if (0 == n64Key) {
			return util::CAutoPointer<CRpczChannel>();
		}
		util::CAutoPointer<CRpczChannel> pRpczChannel(FindRpczChannel(n64Key));
		if (pRpczChannel.IsInvalid()) {
			pRpczChannel = CreateRpczChannel(n64Key, connect);
		}
		return pRpczChannel;
	}

	inline bool RemoveRpczChannel(const std::string& connect)
	{
		uint64_t n64Key = AddressToInteger(connect.c_str());
		if (0 == n64Key) {
			return false;
		}

		thd::CScopedWriteLock scopedWriteLock(m_rwRpczLock);

		if (m_rpczChannels.Empty()) {
			return false;
		}
		long userCount = 0;
		util::CAutoPointer<CRpczChannel> pRpczChannel(m_rpczChannels.EraseEx(n64Key));
		if (pRpczChannel.IsInvalid()) {
			return false;
		} else {
			userCount = pRpczChannel->GetUserCount();
		}
		atomic_xadd(&m_userCount, -userCount);
		return true;
	}

	void AddAgentChannel(
		const ::google::protobuf::RepeatedField< ::google::protobuf::uint32 >& inIds,
		const std::string& connect)
	{
		thd::CScopedWriteLock scopedWriteLock(m_rwAgentLock);
		int nSize = inIds.size();
		if (nSize > 0) {
			uint64_t n64Key = AddressToInteger(connect.c_str());
			if (0 == n64Key) {
				return;
			}
			AGENT_KEY_MAP_T::const_iterator it(m_agentChannels.begin());
			while (m_agentChannels.end() != it) {
				if (it->second == n64Key) {
					it = m_agentChannels.erase(it);
				} else {
					++it;
				}
			}
			for (int i = 0; i < nSize; ++i) {
				m_agentChannels.insert(AGENT_KEY_MAP_T::value_type(inIds.Get(i), n64Key));
			}
		}
	}

	void AddAgentChannel(uint32_t agentId)
	{
		thd::CScopedWriteLock scopedWriteLock(m_rwAgentLock);
		m_agentChannels.insert(AGENT_KEY_MAP_T::value_type(agentId, ID_NULL));
	}

	bool IteratorAgentChannelKey(CHLKEY_AGTID_MAP_T& outChannelKeys, const AGENT_ID_SET_T* agentIds) {
		if (NULL != agentIds && !agentIds->empty()) {
			thd::CScopedReadLock scopedReadLock(m_rwAgentLock);
			if (m_agentChannels.empty()) {
				return false;
			}

			AGENT_KEY_MAP_T::iterator it(m_agentChannels.begin());
			for (; m_agentChannels.end() != it; ++it) {
				if (agentIds->find(it->first) != agentIds->end()) {
					std::pair<CHLKEY_AGTID_MAP_T::iterator, bool> pairIB(
						outChannelKeys.insert(CHLKEY_AGTID_MAP_T::value_type(
							it->second, AGENT_ID_SET_T())));
					pairIB.first->second.insert(it->first);
				}
			}
		} else {
			thd::CScopedReadLock scopedReadLock(m_rwAgentLock);
			if (m_agentChannels.empty()) {
				return false;
			}

			AGENT_KEY_MAP_T::iterator it(m_agentChannels.begin());
			for (; m_agentChannels.end() != it; ++it) {
				std::pair<CHLKEY_AGTID_MAP_T::iterator, bool> pairIB(
					outChannelKeys.insert(CHLKEY_AGTID_MAP_T::value_type(
						it->second, AGENT_ID_SET_T())));
				pairIB.first->second.insert(it->first);
			}
		}

		return true;
	}

	bool CheckAndGetAgentIDs(size_t nCheckSize, ::google::protobuf::RepeatedField< ::google::protobuf::uint32 >* outIds) {
		if (NULL == outIds) {
			return false;
		}
		thd::CScopedReadLock scopedReadLock(m_rwAgentLock);
		if (m_agentChannels.empty()) {
			return false;
		}

		if (m_agentChannels.size() == nCheckSize) {
			return false;
		}

		AGENT_KEY_MAP_T::iterator it(m_agentChannels.begin());
		for (; m_agentChannels.end() != it; ++it) {
			outIds->Add(it->first);
		}
		return true;
	}

	size_t GetAgentChannelSize() const {
		thd::CScopedReadLock scopedReadLock(m_rwAgentLock);
		return m_agentChannels.size();
	}

	void SetClientChannel(uint64_t userId, const std::string& connect,
		uint32_t routeCount)
	{
		uint64_t n64Key = AddressToInteger(connect.c_str());
		if (0 == n64Key) {
			return;
		}
		util::CAutoPointer<CRpczChannel> pServer(FindRpczChannel(n64Key));
		if (pServer.IsInvalid()) {
			RemoveClientChannel(userId);
			return;
		}

		thd::CScopedWriteLock scopedWriteLock(m_rwClientLock);

		util::CAutoPointer<CClientChannel> pClientChannel;
		std::pair<CLIENT_CHANNEL_MAP_T::iterator, bool> pairIB(
		m_clientChannels.insert(CLIENT_CHANNEL_MAP_T::value_type(
		userId, pClientChannel)));
		if (pairIB.second) {
			pairIB.first->second.SetRawPointer(new CClientChannel);
			pClientChannel = pairIB.first->second;
			pClientChannel->m_pServer = pServer;
			pClientChannel->m_routeCount = routeCount;
			atomic_inc(&m_userCount);
			pServer->IncrUserCount();
		} else {
			pClientChannel = pairIB.first->second;
			if (pClientChannel->m_pServer.IsInvalid()) {
				pClientChannel->m_pServer = pServer;
				pClientChannel->m_routeCount = routeCount;
				atomic_inc(&m_userCount);
				pServer->IncrUserCount();
			} else {
				if (routeCount < pClientChannel->GetRouteCount()) {
					pClientChannel->m_pServer->DecrUserCount();
					pClientChannel->m_pServer = pServer;
					pClientChannel->m_routeCount = routeCount;
					pServer->IncrUserCount();
				}
			}
		}
	}

	bool CheckAndCreatePlayer(util::CAutoPointer<CWrapPlayer> &outPlayer, uint64_t userId, CreatePlayerMethod createPlayerMethod = NULL)
	{
		thd::CScopedWriteLock scopedWriteLock(m_rwClientLock);

		CLIENT_CHANNEL_MAP_T::iterator it(m_clientChannels.lower_bound(userId));
		if (m_clientChannels.end() == it || it->first != userId) {
			util::CAutoPointer<CClientChannel> pClientChannel(new CClientChannel);
			if (createPlayerMethod != NULL) {
				util::CAutoPointer<CPlayerBase> pPlayerBase((*createPlayerMethod)(userId));
				outPlayer.SetRawPointer(new CWrapPlayer(pPlayerBase));
				pClientChannel->SetPlayer(outPlayer);
			} else {
				outPlayer.SetRawPointer(new CWrapPlayer);
				pClientChannel->SetPlayer(outPlayer);
			}
			m_clientChannels.insert(it, CLIENT_CHANNEL_MAP_T::value_type(userId, pClientChannel));
			return true;
		} else {
			outPlayer = it->second;
		}
		return false;
	}

	util::CAutoPointer<rpcz::rpc_channel> GetRpczChannel(uint64_t userId) {

        util::CAutoPointer<CClientChannel> pClientChannel(GetClientChannel(userId));
        if (pClientChannel.IsInvalid()) {
            return util::CAutoPointer<rpcz::rpc_channel>();
        }
		return pClientChannel->GetChannel();
	}

    void RemoveClientChannel(uint64_t userId)
    {
        thd::CScopedWriteLock scopedWriteLock(m_rwClientLock);
        CLIENT_CHANNEL_MAP_T::iterator it(m_clientChannels.find(userId));
        if (m_clientChannels.end() != it) {
            it->second->RemovePlayer();
			util::CWeakPointer<CRpczChannel> 
				pServer(it->second->m_pServer);
            m_clientChannels.erase(it);
			
			if (!pServer.IsInvalid()) {
				atomic_dec(&m_userCount);
				pServer->DecrUserCount();
			}
        }
    }

	void RemoveWrapPlayer(uint64_t userId)
	{
		thd::CScopedWriteLock scopedWriteLock(m_rwClientLock);
		CLIENT_CHANNEL_MAP_T::iterator it(m_clientChannels.find(userId));
		if (m_clientChannels.end() == it) {
			return;
		}
		if (!it->second->RemoveInvalidPlayer()) {
			return;
		}
		m_clientChannels.erase(it);
	}

	bool CheckClientChannel(uint64_t userId, bool bTimeout) {

        util::CAutoPointer<CClientChannel> pClientChannel(GetClientChannel(userId));
        if(pClientChannel.IsInvalid()) {
            return false;
        }

		if(bTimeout) {
			if(atomic_inc(&pClientChannel->m_timeoutCount)
				>= TIMEOUT_MAX_TIMES_REMOVE_CHANNEL)
			{
				RemoveClientChannel(userId);
			}
		} else {
			atomic_xchg(&pClientChannel->m_timeoutCount, 0);
		}
        return true;
	}

	bool IteratorClientChannel(std::vector<struct UserIdAndChannel>& outChannels) {

		thd::CScopedReadLock scopedReadLock(m_rwClientLock);
		if(m_clientChannels.empty()) {
			return false;
		}

		CLIENT_CHANNEL_MAP_T::iterator itUOA(m_clientChannels.begin());
		for (; m_clientChannels.end() != itUOA; ++itUOA) {
			util::CAutoPointer<CClientChannel> pClientChannel(itUOA->second);
			if (pClientChannel.IsInvalid()) {
				continue;
			}
			struct UserIdAndChannel idAndChan;
			idAndChan.pChannel = pClientChannel->GetChannel();
			if (!idAndChan.pChannel.IsInvalid()) {
				idAndChan.userId = itUOA->first;
				outChannels.push_back(idAndChan);
			}
		}
        return true;
	}

	bool IteratorClient(std::vector<uint64_t>& outClients) {

		thd::CScopedReadLock scopedReadLock(m_rwClientLock);
		if(m_clientChannels.empty()) {
			return false;
		}

		CLIENT_CHANNEL_MAP_T::iterator itUOA(m_clientChannels.begin());
		for(; m_clientChannels.end() != itUOA; ++itUOA) {
			util::CAutoPointer<CClientChannel> pClientChannel(itUOA->second);
			if(pClientChannel.IsInvalid()) {
				continue;
			}
			if(!pClientChannel->GetChannel().IsInvalid()) {
				outClients.push_back(itUOA->first);
			}
		}
		return true;
	}

	bool IteratorClient(::google::protobuf::RepeatedField< ::google::protobuf::uint64 >* outIds) {
		if (NULL == outIds) {
			return false;
		}
		thd::CScopedReadLock scopedReadLock(m_rwClientLock);
		if (m_clientChannels.empty()) {
			return false;
		}

		CLIENT_CHANNEL_MAP_T::iterator itUOA(m_clientChannels.begin());
		for (; m_clientChannels.end() != itUOA; ++itUOA) {
			util::CAutoPointer<CClientChannel> pClientChannel(itUOA->second);
			if (pClientChannel.IsInvalid()) {
				continue;
			}
			if (!pClientChannel->GetChannel().IsInvalid()) {
				outIds->Add(itUOA->first);
			}
		}
		return true;
	}

	bool IteratorClient(::google::protobuf::RepeatedField< ::google::protobuf::uint64 >* outIds,
		const std::string& whichConnect)
	{
		if (NULL == outIds) {
			return false;
		}
		uint64_t n64Key = AddressToInteger(whichConnect.c_str());
		if (0 == n64Key) {
			return false;
		}

		thd::CScopedReadLock scopedReadLock(m_rwClientLock);
		if (m_clientChannels.empty()) {
			return false;
		}

		CLIENT_CHANNEL_MAP_T::iterator itUOA(m_clientChannels.begin());
		for (; m_clientChannels.end() != itUOA; ++itUOA) {
			util::CAutoPointer<CClientChannel> pClientChannel(itUOA->second);
			if (pClientChannel.IsInvalid()) {
				continue;
			}
			if (pClientChannel->GetAddressId() != n64Key) {
				continue;
			}
			if (pClientChannel->GetChannel().IsInvalid()) {
				continue;
			}
			outIds->Add(itUOA->first);
		}
		return true;
	}

	//bool SetPlayer(uint64_t userId, const util::CAutoPointer<CWrapPlayer>& pPlayer) {

 //       if(pPlayer.IsInvalid()) {
 //           return false;
 //       }

	//	util::CAutoPointer<CClientChannel> pClientChannel(GetClientChannel(userId));
	//	if(pClientChannel.IsInvalid()) {
	//		return false;
	//	}
	//	pClientChannel->SetPlayer(pPlayer);
	//	return true;
	//}

	util::CAutoPointer<CWrapPlayer> GetPlayer(uint64_t userId) {

        util::CAutoPointer<CClientChannel> pClientChannel(GetClientChannel(userId));

		if(pClientChannel.IsInvalid()) {
			return util::CAutoPointer<CWrapPlayer>();
		}
		return pClientChannel->GetPlayer();
	}

	bool ExistRouteServer(uint64_t routeAddressId) {
		if (routeAddressId == ID_NULL) {
			return false;
		}
		thd::CScopedReadLock scopedReadLock(m_rwRouteIdsLock);
		return m_routeAddressIds.find(routeAddressId) != m_routeAddressIds.end();
	}

	void AddRouteServer(uint64_t routeAddressId) {
		if (routeAddressId == ID_NULL) {
			return;
		}
		thd::CScopedReadLock scopedReadLock(m_rwRouteIdsLock);
		ROUTE_SERVID_SET_T::iterator it(m_routeAddressIds.lower_bound(routeAddressId));
		if (m_routeAddressIds.end() != it && *it == routeAddressId) {
			return;
		}
		thd::CScopedWriteLock scopedWriteLock(scopedReadLock);
		m_routeAddressIds.insert(it, routeAddressId);
	}

    long GetClientSize() const {
        return m_userCount;
    }

private:

	inline util::CAutoPointer<CRpczChannel> CreateRpczChannel(
		uint64_t n64Key, const std::string& connect)
	{
		thd::CScopedWriteLock scopedWriteLock(m_rwRpczLock);

		RPCZ_CHANNEL_MAP_T::iterator it;
		if(m_rpczChannels.LowerBound(n64Key, it)) {
			util::CAutoPointer<CRpczChannel> pChannel(new CRpczChannel(n64Key, ::rpcz::rpc_channel::create(connect)));
			it = m_rpczChannels.Insert(it, pChannel);
		}
		return *it;
	}

	inline util::CAutoPointer<CClientChannel> GetClientChannel(uint64_t userId)
	{
		thd::CScopedReadLock scopedReadLock(m_rwClientLock);

        if(m_clientChannels.empty()) {
            return util::CAutoPointer<CClientChannel>();
        }

		CLIENT_CHANNEL_MAP_T::const_iterator itUOA(m_clientChannels.find(userId));
		if(m_clientChannels.end() == itUOA) {
			return util::CAutoPointer<CClientChannel>();
		}

        return itUOA->second;
	}

private:
	RPCZ_CHANNEL_MAP_T m_rpczChannels;
	thd::CSpinRWLock m_rwRpczLock;

	CLIENT_CHANNEL_MAP_T m_clientChannels;
	thd::CSpinRWLock m_rwClientLock;

	AGENT_KEY_MAP_T m_agentChannels;
	thd::CSpinRWLock m_rwAgentLock;

	ROUTE_SERVID_SET_T m_routeAddressIds;
	thd::CSpinRWLock m_rwRouteIdsLock;

	volatile long m_userCount;
};

#endif /* CHANNELMANAGER_H */


