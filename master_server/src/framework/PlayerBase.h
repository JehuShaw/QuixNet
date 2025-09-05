/* 
 * File:   PlayerBase.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef PLAYERBASE_H
#define	PLAYERBASE_H

#include "AutoPointer.h"

class CPlayerBase {
public:
    CPlayerBase()  {

    }

    virtual ~CPlayerBase() { 
    }

	virtual uint64_t GetUserID() const {
		throw std::runtime_error("Method GetUserID() not implemented.");
	}

protected:
	/** %Lock operation. */
	virtual void Lock() = 0;

	/** Unlock operation. */
	virtual void Unlock() = 0;

private:
	friend class CScopedPlayerMutex;
	friend class CScopedPlayersMutex;
    
};

class CScopedPlayerMutex {
public:
    explicit CScopedPlayerMutex(const util::CAutoPointer<CPlayerBase>& player) throw()
        : m_player(player) 
    {
        m_player->Lock();
    }

    ~CScopedPlayerMutex() throw()
    {
        m_player->Unlock();
    }
private:
    util::CAutoPointer<CPlayerBase> m_player;
};

typedef std::vector<util::CAutoPointer<CPlayerBase> > PLAYER_BASE_SET_T;
typedef std::vector<uint64_t> PLAYER_ID_SET_T;

class CScopedPlayersMutex {
public:
	explicit CScopedPlayersMutex(PLAYER_BASE_SET_T& players) throw()
		: m_players(players)
	{
		if(m_players.empty()) {
			return;
		}
		int nSize = (int)m_players.size();
		for(int i = 0; i < nSize; ++i) {
			m_players[i]->Lock();
		}
	}

	~CScopedPlayersMutex() throw()
	{
		if(m_players.empty()) {
			return;
		}
		int nSize = (int)m_players.size();
		for(int i = 0; i < nSize; ++i) {
			m_players[i]->Unlock();
		}
	}

private:
	PLAYER_BASE_SET_T& m_players;
};

#endif /* PLAYERBASE_H */
