/* 
 * File:   PlayerMutex.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef _PLAYERMUTEX_H_
#define	_PLAYERMUTEX_H_

#include "PlayerBase.h"


class CPlayerMutex : public CPlayerBase
{
public:
	CPlayerMutex(){}
	virtual ~CPlayerMutex(void){}

protected:
	/** %Lock operation. */
	virtual void Lock() {
		m_csLock.Lock();
	}

	/** Unlock operation. */
	virtual void Unlock() {
		m_csLock.Unlock();
	}

private:
	thd::CCriticalSection m_csLock;
};

#endif /* _PLAYERMUTEX_H_ */
