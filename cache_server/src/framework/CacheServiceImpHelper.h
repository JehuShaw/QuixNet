/* 
 * File:   CacheServiceImpHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef CACHESERVICEIMPHELPER_H
#define CACHESERVICEIMPHELPER_H

#include "NodeDefines.h"
#include "CachePlayer.h"
#include "CacheServiceImp.h"


//////////////////////////////////////////////////////////////////////////
// Use by class CWorkerServiceImp

//// If you don't set anything, then interest every cmd. e.g
// static void NodeProtocolInterests(::node::InterestPacket& outInterests) 
// {
// }

// List your interesting notification
 //static void NodeNotificationInterests(::node::InterestPacket& outInterests) 
 //{
 //}

// If want create player instance, then set the create player function. e.g
static util::CAutoPointer<CPlayerBase> CreateCachePlayer(uint64_t userId) 
{
	uint16_t u16DBId = CCacheServiceImp::GetDBIDFromDBByUserID(userId);
	return util::CAutoPointer<CCachePlayer>(new CCachePlayer(u16DBId));
}

#endif /* CACHESERVICEIMPHELPER_H */

