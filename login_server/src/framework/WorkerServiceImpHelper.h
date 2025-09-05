/* 
 * File:   WorkerServiceImpHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef WORKERSERVICEIMPHELPER_H
#define WORKERSERVICEIMPHELPER_H

#include "NodeDefines.h"

//////////////////////////////////////////////////////////////////////////
// Use by class CWorkerServiceImp
// If want create player instance, then set the create player function. e.g
// static CAutoPointer<CPlayerBase> CreatePlayer(uint64_t userId) 
// {
// 		return CAutoPointer<CPlayer>(new CPlayer(userId));
// }
// If this node has interest cmd list them. e.g 
// static void NodeListProtocolInterests(::node::InterestsResponse& interestsResponse) 
// {
//		interestsResponse->add_interests(P_CMD_C_LOGIN);
// }
// If you don't set anything, then interest every cmd. e.g
// static void NodeListProtocolInterests(::node::InterestsResponse& interestsResponse) 
// {
// }


#endif /* WORKERSERVICEIMPHELPER_H */

