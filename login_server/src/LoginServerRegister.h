/* 
 * File:   LoginServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef LOGINSERVERREGISTER_H
#define	LOGINSERVERREGISTER_H

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

	void RegisterModule();

	void UnregisterModule();
};

#endif	/* LOGINSERVERREGISTER_H */

