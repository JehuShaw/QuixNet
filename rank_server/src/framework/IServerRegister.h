/* 
 * File:   IServerRegister.h
 * Author: Jehu Shaw
 *
 * Created on 2010_9_6 PM 3:23
 */

#ifndef _ISERVERREGISTER_H_
#define	_ISERVERREGISTER_H_

class IServerRegister
{
public:
	virtual ~IServerRegister(){}

	virtual void InitStatusCallback() = 0;

	virtual void InitTemplate() = 0;

    virtual void RegisterCommand() = 0;

	virtual void RegistModule() = 0;

	virtual void UnregistModule() = 0;

};

#endif	/* _ISERVERREGISTER_H_ */

