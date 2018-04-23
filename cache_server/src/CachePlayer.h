/* 
 * File:   CachePlayer.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef _CACHEPLAYER_H_
#define	_CACHEPLAYER_H_

#include "PlayerBase.h"
#include "PoolBase.h"

class CCachePlayer : public CPlayerBase, public util::PoolBase<CCachePlayer>
{
public:
	CCachePlayer(uint16_t u16DBId) : m_u16DBId(u16DBId) {}

	~CCachePlayer(void) {}

	uint16_t GetDBID() const {
		return m_u16DBId;
	}

protected:
	virtual void Lock() {
	}

	/** Unlock operation. */
	virtual void Unlock() {
	}

private:
    uint16_t m_u16DBId;

};

#endif /* _CACHEPLAYER_H_ */
