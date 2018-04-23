/* 
 * File:   FillPacketHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef __FILLPACKETHELPER_H__
#define __FILLPACKETHELPER_H__

#include "Common.h"
#include "WeakPointer.h"

namespace rank 
{
    class RankItemPacket;
}

namespace google {
    namespace protobuf {
        template <typename Element>
        class RepeatedPtrField;
    }
}

class CRankData;

// 填充角色数据到通讯数据包
extern void FillRankItem(
	::rank::RankItemPacket& outRankItem, 
	uint64_t userId,
	uint32_t rank,
	int32_t score);


#endif /* __FILLPACKETHELPER_H__ */
