/* 
 * File:   LoginServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef _LOGINSERVERREGISTER_H_
#define	_LOGINSERVERREGISTER_H_

#include "IServerRegister.h"

class CLoginServerRegister
	: public IServerRegister
{
public:
	CLoginServerRegister();

	~CLoginServerRegister();

	void InitStatusCallback();

	void InitTemplate();

	void RegisterCommand();

	void RegistModule();

	void UnregistModule();
};

#endif	/* _LOGINSERVERREGISTER_H_ */

