/* 
 * File:   ServantServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef _SERVANTSERVERREGISTER_H_
#define	_SERVANTSERVERREGISTER_H_

#include <set>
#include <map>
#include <zmq.hpp>
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

	void RegistModule();

	void UnregistModule();
};

#endif	/* _SERVANTSERVERREGISTER_H_ */

