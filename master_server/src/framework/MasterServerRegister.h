/* 
 * File:   MasterServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef _MASTERSERVERREGISTER_H_
#define	_MASTERSERVERREGISTER_H_

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

	void RegistModule();

	void UnregistModule();

private:
    static int CommandSendMail(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandAutoRestart(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandRestart(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandShutdown(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandErase(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandPlay(const util::CWeakPointer<evt::ArgumentBase>& arg);
	static int CommandStop(const util::CWeakPointer<evt::ArgumentBase>& arg);
};

#endif	/* _MASTERSERVERREGISTER_H_ */

