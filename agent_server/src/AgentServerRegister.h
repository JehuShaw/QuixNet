/* 
 * File:   AgentServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_10_7 PM 11:45
 */

#ifndef AGENTSERVERREGISTER_H
#define	AGENTSERVERREGISTER_H

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

	void RegisterModule();

	void UnregisterModule();
};

#endif	/* AGENTSERVERREGISTER_H */

