/* 
 * File:   ServerRegisterHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef SERVERREGISTERHELPER_H
#define SERVERREGISTERHELPER_H

#include "NodeDefines.h"
#include "LoginServerRegister.h"


//////////////////////////////////////////////////////////////////////////
// Use by class CXXXServer

// If want create IServerRegister instance, then set the CreateServerRegister function. e.g
//static util::CAutoPointer<CreateServerRegister> CreateServerRegister() 
//{
//	return util::CAutoPointer<CGameServerRegister>(new CGameServerRegister);
//}

static util::CAutoPointer<IServerRegister> CreateServerRegister() 
{
	return util::CAutoPointer<CLoginServerRegister>(new CLoginServerRegister);
}

#endif /* SERVERREGISTERHELPER_H */

