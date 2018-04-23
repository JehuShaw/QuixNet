/* 
 * File:   FillPacketHelper.cpp
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */
#include "FillPacketHelper.h"

#include "Log.h"
#include "Common.h"
#include "WeakPointer.h"
#include "msg_rank_item.pb.h"
#include "RankData.h"

using namespace util;

// 填充角色数据到通讯数据包
void FillRankItem(
	::rank::RankItemPacket& outRankItem, 
	uint64_t userId,
	uint32_t rank,
    int32_t score)
{
    outRankItem.set_user_id(userId);
    outRankItem.set_rank(rank);
    outRankItem.set_score(score);
}





