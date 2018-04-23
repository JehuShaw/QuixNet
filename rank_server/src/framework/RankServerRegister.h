/* 
 * File:   RankServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef _RANKSERVERREGISTER_H_
#define	_RANKSERVERREGISTER_H_

#include "AgentMethod.h"
#include "IServerRegister.h"

class CRankServerRegister
	: public IServerRegister
{
public:
	CRankServerRegister();

	~CRankServerRegister();

	void InitStatusCallback();

	void InitTemplate();

	void RegisterCommand();

	void RegistModule();

	void UnregistModule();

private:
    static int CommandKickout(const util::CWeakPointer<evt::ArgumentBase>& arg);
};

#endif	/* _RANKSERVERREGISTER_H_ */

