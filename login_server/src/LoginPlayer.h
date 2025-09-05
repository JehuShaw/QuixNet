/* 
 * File:   LoginPlayer.h
 * Author: Jehu Shaw
 * 
 * Created on 2020_4_7, 14:46
 */

#ifndef LOGINPLAYER_H
#define	LOGINPLAYER_H

#include "PlayerBase.h"
#include "NetworkTypes.h"

class CLoginPlayer : public CPlayerBase
{
public:
	CLoginPlayer(const ntwk::SocketID& socketId)
		:m_socketId(socketId) {
	}

	~CLoginPlayer(void){}

	const ntwk::SocketID& GetSocketID() const {
		return m_socketId;
	}

protected:
	/** %Lock operation. */
	virtual void Lock() {
	}

	/** Unlock operation. */
	virtual void Unlock() {
	}

private:
	ntwk::SocketID m_socketId;
};

#endif /* _LOGINPLAYER_H_ */
