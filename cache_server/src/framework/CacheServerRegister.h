/* 
 * File:   CacheServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef CACHESERVERREGISTER_H
#define	CACHESERVERREGISTER_H

#include "IServerRegister.h"

class CCacheServerRegister
	: public IServerRegister
{
public:
	CCacheServerRegister();

	~CCacheServerRegister();

	virtual void InitStatusCallback();

	virtual void InitTemplate();
	
    virtual void RegisterCommand();

	virtual void RegisterModule();

	virtual void UnregisterModule();
};

#endif	/* CACHESERVERREGISTER_H */

