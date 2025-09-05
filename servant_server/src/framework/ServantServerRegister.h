/* 
 * File:   ServantServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef SERVANTSERVERREGISTER_H
#define	SERVANTSERVERREGISTER_H

#include <set>
#include <map>
#include "NodeDefines.h"
#include "CThreads.h"
#include "rpcz.hpp"
#include "AutoPointer.h"
#include "WorkerServiceImp.h"
#include "ServantServiceImp.h"
#include "IServerRegister.h"

class CServantServerRegister
	: public IServerRegister
{
public:
	CServantServerRegister();

	~CServantServerRegister();

	void InitStatusCallback();

	void InitTemplate();

	void RegisterCommand();

	void RegisterModule();

	void UnregisterModule();
};

#endif	/* SERVANTSERVERREGISTER_H */

