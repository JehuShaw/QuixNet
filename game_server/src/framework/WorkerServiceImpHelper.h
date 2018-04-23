/* 
 * File:   WorkerServiceImpHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef _WORKERSERVICEIMPHELPER_H
#define _WORKERSERVICEIMPHELPER_H

#include "NodeDefines.h"
#include "interest_packet.pb.h"
#include "Player.h"


//////////////////////////////////////////////////////////////////////////
// Use by class CWorkerServiceImp

//// If you don't set anything, then interest every cmd. e.g
// static void NodeProtocolInterests(::node::InterestPacket& outInterests) 
// {
// }

// List your interesting notification
 static void NodeNotificationInterests(::node::InterestPacket& outInterests) 
 {
	 outInterests.add_interests(N_CMD_KICK_LOGGED);
 }

// If want create player instance, then set the create player function. e.g
static util::CAutoPointer<CPlayerBase> CreatePlayer(uint64_t userId) 
{
	return util::CAutoPointer<CPlayer>(new CPlayer(userId));
}

#endif /* _WORKERSERVICEIMPHELPER_H */

