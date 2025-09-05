/* 
 * File:   WorkerServiceImpHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef WORKERSERVICEIMPHELPER_H
#define WORKERSERVICEIMPHELPER_H

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
	 outInterests.add_interests(N_CMD_MASTER_TO_USER);
	 outInterests.add_interests(N_CMD_KICK_LOGGED);
	 outInterests.add_interests(N_CMD_CHECK_CREATE_CHARACTER);
	 outInterests.add_interests(N_CMD_CREATE_CHARACTER);
	 outInterests.add_interests(N_CMD_GET_CHARACTER);
	 outInterests.add_interests(N_CMD_PRESET_TARGET_MAP);
	 outInterests.add_interests(N_CMD_PRIVATE_CHAT);
	 outInterests.add_interests(N_CMD_APPLY_FRIENDS);
	 outInterests.add_interests(N_CMD_RESULT_OF_APPLYING);
	 outInterests.add_interests(N_CMD_REMOVE_FRIEND);
 }

// If want create player instance, then set the create player function. e.g
static util::CAutoPointer<CPlayerBase> CreatePlayer(uint64_t userId)
{
	return util::CAutoPointer<CPlayerBase>(new CPlayer(userId));
}

#endif /* WORKERSERVICEIMPHELPER_H */

