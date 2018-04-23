/* 
 * File:   CacheServiceImpHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef _CACHESERVICEIMPHELPER_H
#define _CACHESERVICEIMPHELPER_H

#include "NodeDefines.h"
#include "CachePlayer.h"
#include "CacheDBManager.h"


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
	CCacheDBManager::PTR_T pCacheDBMgr(CCacheDBManager::Pointer());
	uint16_t u16DBId = pCacheDBMgr->GetDBIdByUserId(userId);
	if(0 == u16DBId) {
		u16DBId = pCacheDBMgr->GetMinLoadDBIdByBalUserId(userId);
	}
	return util::CAutoPointer<CCachePlayer>(new CCachePlayer(u16DBId));
}

#endif /* _CACHESERVICEIMPHELPER_H */

