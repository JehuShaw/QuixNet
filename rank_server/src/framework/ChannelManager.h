/*
 * File:   ChannelManager.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef CHANNELMANAGER_H_
#define CHANNELMANAGER_H_

#include "rpc_channel.hpp"
#include "rpc_controller.hpp"
#include "AutoPointer.h"
#include "SpinRWLock.h"
#include "NodeDefines.h"
#include "PlayerBase.h"
#include "Singleton.h"

class CClientChannel {
public:
	CClientChannel(): m_routeCount(0), m_timeoutCount(0) {}

	uint32_t GetRouteCount() { return m_routeCount; }
	const std::string& GetAddress() { return m_address; }

	void SetPlayer(const util::CAutoPointer<CPlayerBase>& pPlayer) {
		thd::CScopedWriteLock wrLock(m_rwPlayerLock);
		m_pPlayer = pPlayer;
	}

    void ClearPlayer() {
        thd::CScopedWriteLock wrLock(m_rwPlayerLock);
        m_pPlayer.SetRawPointer(NULL);
    }

	const util::CAutoPointer<CPlayerBase>& GetPlayer() {
		thd::CScopedReadLock rdLock(m_rwPlayerLock);
		return m_pPlayer;
	}
private:
	friend class CChannelManager;
	uint32_t m_routeCount;
	std::string m_address;
	volatile unsigned long m_timeoutCount;

	util::CAutoPointer<CPlayerBase> m_pPlayer;
	thd::CSpinRWLock m_rwPlayerLock;

};

struct UserIdAndChannel {
	uint64_t userId;
	util::CAutoPointer<rpcz::rpc_channel> pChannel;
};

class CChannelManager
	: public util::Singleton<CChannelManager>
{
public:
	typedef std::map<uint64_t, util::CAutoPointer<rpcz::rpc_channel> > RPCZ_CHANNEL_MAP_T;
	typedef std::map<uint64_t, util::CAutoPointer<CClientChannel> > USERID_TO_CLIENT_CHANNEL_T;

    void Dispose()
    {
        if(true) {
            thd::CScopedWriteLock scopedWriteLock(m_rwClientLock);
            m_clientChannels.clear();
        }
        if(true) {
            thd::CScopedWriteLock scopedWriteLock(m_rwRpczLock);
            m_rpczChannels.clear();
        }
    }

	util::CAutoPointer<rpcz::rpc_channel> FindRpczChannel(const std::string& connect) {
		uint64_t n64Key = AddressToInteger(connect.c_str());
		return FindRpczChannel(n64Key);
	}

	util::CAutoPointer<rpcz::rpc_channel> GetRpczChannel(const std::string& connect) {
		uint64_t n64Key = AddressToInteger(connect.c_str());
		if(0 == n64Key) {
			return util::CAutoPointer<rpcz::rpc_channel>();
		}
		util::CAutoPointer<rpcz::rpc_channel> pChannel(FindRpczChannel(n64Key));
		if(!pChannel.IsInvalid()) {
			return pChannel;
		}
		return CreateRpczChannel(n64Key, connect);
	}

	void SetClientChannel(uint64_t userId, const std::string& address,
		uint32_t routeCount)
	{
		thd::CScopedWriteLock scopedWriteLock(m_rwClientLock);

		util::CAutoPointer<CClientChannel> pClientChannel;
		std::pair<USERID_TO_CLIENT_CHANNEL_T::iterator, bool> pairIB(
		m_clientChannels.insert(USERID_TO_CLIENT_CHANNEL_T::value_type(
		userId, pClientChannel)));
		if(pairIB.second) {
			pairIB.first->second.SetRawPointer(new CClientChannel);
			pClientChannel = pairIB.first->second;
			pClientChannel->m_address = address;
			pClientChannel->m_routeCount = routeCount;
			atomic_xchg(&pClientChannel->m_timeoutCount, 0);
		} else {
			if(routeCount < pairIB.first->second->GetRouteCount()) {
				pairIB.first->second->m_address = address;
				pairIB.first->second->m_routeCount = routeCount;
			}
		}
	}

	util::CAutoPointer<rpcz::rpc_channel> GetRpczChannel(uint64_t userId) {

        util::CAutoPointer<CClientChannel> pClientChannel(GetClientChannel(userId));
        if(pClientChannel.IsInvalid()) {
            return util::CAutoPointer<rpcz::rpc_channel>();
        }
		return GetRpczChannel(pClientChannel->GetAddress());
	}

    void RemoveClientChannel(uint64_t userId)
    {
        thd::CScopedWriteLock scopedWriteLock(m_rwClientLock);
        USERID_TO_CLIENT_CHANNEL_T::iterator it(m_clientChannels.find(userId));
        if(m_clientChannels.end() != it) {
            it->second->ClearPlayer();
            m_clientChannels.erase(it);
        }
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
		outChannels.resize(m_clientChannels.size());
		USERID_TO_CLIENT_CHANNEL_T::iterator itUOA(m_clientChannels.begin());
		for(int i = 0; m_clientChannels.end() != itUOA; ++itUOA, ++i) {
			util::CAutoPointer<CClientChannel> pClientChannel(itUOA->second);
			if(pClientChannel.IsInvalid()) {
				continue;
			}
			struct UserIdAndChannel& idAndChan = outChannels[i];
			idAndChan.userId = itUOA->first;
			idAndChan.pChannel = GetRpczChannel(pClientChannel->GetAddress());
		}
        return true;
	}

	bool IteratorClient(std::vector<uint64_t>& outClients) {

		thd::CScopedReadLock scopedReadLock(m_rwClientLock);
		if(m_clientChannels.empty()) {
			return false;
		}
		outClients.resize(m_clientChannels.size());
		USERID_TO_CLIENT_CHANNEL_T::iterator itUOA(m_clientChannels.begin());
		for(int i = 0; m_clientChannels.end() != itUOA; ++itUOA, ++i) {
			outClients[i] = itUOA->first;
		}
		return true;
	}

	bool SetPlayer(uint64_t userId, util::CAutoPointer<CPlayerBase> pPlayer) {

        if(pPlayer.IsInvalid()) {
            return false;
        }

		util::CAutoPointer<CClientChannel> pClientChannel(GetClientChannel(userId));
		if(pClientChannel.IsInvalid()) {
			return false;
		}
		pClientChannel->SetPlayer(pPlayer);
		return true;
	}

	util::CAutoPointer<CPlayerBase> GetPlayer(uint64_t userId) {

        util::CAutoPointer<CClientChannel> pClientChannel(GetClientChannel(userId));

		if(pClientChannel.IsInvalid()) {
			return util::CAutoPointer<CPlayerBase>();
		}
		return pClientChannel->GetPlayer();
	}

    int GetClientSize() {
        thd::CScopedReadLock scopedReadLock(m_rwClientLock);
        return (int)m_clientChannels.size();
    }

private:

	inline util::CAutoPointer<rpcz::rpc_channel> CreateRpczChannel(
		uint64_t n64Key, const std::string& connect)
	{
		thd::CScopedWriteLock scopedWriteLock(m_rwRpczLock);

		util::CAutoPointer<rpcz::rpc_channel> pChannel;
		std::pair<RPCZ_CHANNEL_MAP_T::iterator, bool> pairIB(m_rpczChannels.insert(
			RPCZ_CHANNEL_MAP_T::value_type(n64Key, pChannel)));
		if(pairIB.second) {
			pairIB.first->second.SetRawPointer(::rpcz::rpc_channel::create(connect));
		}
		return pairIB.first->second;
	}

	inline util::CAutoPointer<rpcz::rpc_channel> FindRpczChannel(uint64_t n64Key)
	{
		thd::CScopedReadLock scopedReadLock(m_rwRpczLock);

		if(m_rpczChannels.empty()) {
			return util::CAutoPointer<rpcz::rpc_channel>();
		}
		RPCZ_CHANNEL_MAP_T::iterator it(m_rpczChannels.find(n64Key));
		if(m_rpczChannels.end() != it) {
			return it->second;
		}
		return util::CAutoPointer<rpcz::rpc_channel>();
	}

	inline util::CAutoPointer<CClientChannel> GetClientChannel(uint64_t userId)
	{
		thd::CScopedReadLock scopedReadLock(m_rwClientLock);

        if(m_clientChannels.empty()) {
            return util::CAutoPointer<CClientChannel>();
        }

		USERID_TO_CLIENT_CHANNEL_T::iterator itUOA(m_clientChannels.find(userId));
		if(m_clientChannels.end() == itUOA) {
			return util::CAutoPointer<CClientChannel>();
		}

        return itUOA->second;
	}

private:
	RPCZ_CHANNEL_MAP_T m_rpczChannels;
	thd::CSpinRWLock m_rwRpczLock;

	USERID_TO_CLIENT_CHANNEL_T m_clientChannels;
	thd::CSpinRWLock m_rwClientLock;
};

#endif /* CHANNELMANAGER_H_ */


