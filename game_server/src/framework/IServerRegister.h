/* 
 * File:   IServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef ISERVERREGISTER_H
#define	ISERVERREGISTER_H

class IServerRegister
{
public:
	virtual ~IServerRegister(){}

	virtual void InitStatusCallback() = 0;

	virtual void InitTemplate() = 0;

    virtual void RegisterCommand() = 0;

	virtual void RegisterModule() = 0;

	virtual void UnregisterModule() = 0;

};

#endif	/* ISERVERREGISTER_H */

