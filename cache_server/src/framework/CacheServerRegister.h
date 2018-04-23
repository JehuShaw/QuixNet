/* 
 * File:   CacheServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef _CACHESERVERREGISTER_H_
#define	_CACHESERVERREGISTER_H_

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

	virtual void RegistModule();

	virtual void UnregistModule();
};

#endif	/* _CACHESERVERREGISTER_H_ */

