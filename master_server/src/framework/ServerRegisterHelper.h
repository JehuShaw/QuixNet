/* 
 * File:   ServerRegisterHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef _SERVERREGISTERHELPER_H_
#define _SERVERREGISTERHELPER_H_

#include "NodeDefines.h"
#include "MasterServerRegister.h"


//////////////////////////////////////////////////////////////////////////
// Use by class CXXXServer

// If want create IServerRegister instance, then set the CreateServerRegister function. e.g
//static util::CAutoPointer<CreateServerRegister> CreateServerRegister() 
//{
//	return util::CAutoPointer<CGameServerRegister>(new CGameServerRegister);
//}

static util::CAutoPointer<IServerRegister> CreateServerRegister() 
{
	return util::CAutoPointer<CMasterServerRegister>(new CMasterServerRegister);
}

#endif /* _SERVERREGISTERHELPER_H_ */

