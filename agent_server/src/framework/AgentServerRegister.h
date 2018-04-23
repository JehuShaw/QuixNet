/* 
 * File:   AgentServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_10_7 PM 11:45
 */

#ifndef _AGENTSERVERREGISTER_H_
#define	_AGENTSERVERREGISTER_H_

#include "IServerRegister.h"

class CAgentServerRegister
	: public IServerRegister
{
public:
	CAgentServerRegister();

	~CAgentServerRegister();

	void InitStatusCallback();

	void InitTemplate();

	void RegisterCommand();

	void RegistModule();

	void UnregistModule();
};

#endif	/* _AGENTSERVERREGISTER_H_ */

