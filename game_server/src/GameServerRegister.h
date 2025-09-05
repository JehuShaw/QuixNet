/* 
 * File:   GameServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef GAMESERVERREGISTER_H
#define	GAMESERVERREGISTER_H

#include "IServerRegister.h"
#include "AgentMethod.h"

class CGameServerRegister
	: public IServerRegister
{
public:
	CGameServerRegister();

	~CGameServerRegister();

	virtual void InitStatusCallback();

	virtual void InitTemplate();

	virtual void RegisterCommand();

	virtual void RegisterModule();

	virtual void UnregisterModule();

private:
	static int CommandShowThreadPool(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandAddAttrib(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandKickout(const util::CWeakPointer<evt::ArgumentBase>& arg);
};

#endif	/* _GAMESERVERREGISTER_H_ */

