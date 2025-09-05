/* 
 * File:   MasterServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef MASTERSERVERREGISTER_H
#define	MASTERSERVERREGISTER_H

#include "WeakPointer.h"
#include "AgentMethod.h"
#include "IServerRegister.h"

class CMasterServerRegister
	: public IServerRegister
{
public:
	CMasterServerRegister();

	~CMasterServerRegister();

	void InitStatusCallback();

	void InitTemplate();

	void RegisterCommand();

	void RegisterModule();

	void UnregisterModule();

private:
    static int CommandSendMail(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandAutoRestart(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandRestart(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandShutdown(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandErase(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandBegin(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandStop(const util::CWeakPointer<evt::ArgumentBase>& arg);
};

#endif	/* MASTERSERVERREGISTER_H */

