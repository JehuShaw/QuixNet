/* 
 * File:   GuidFactory.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2011_7_26, 11:09
 */

#include "GuidFactory.h"
#include "TimestampManager.h"

// 2014-1-1 00:00:00
#define START_FILETIME (1391184000)

#define TIMESTAMP_MASK 0xFFFFFFFF
#define TIMESTAMP_BIT 32

#define CODE_MASK 0xFFFF
#define CODE_BIT 16

#define INSTANCE_MASK 0xFFFF
#define INSTANCE_BIT 16

using namespace util;
using namespace evt;

CGuidFactory::CGuidFactory()
	: m_uCount(0) {

	atomic_xchg16(&m_code.u16Code, 0);
	CTimestampManager::Pointer();
}

uint64_t CGuidFactory::CreateGuid() {
	CTimestampManager::PTR_T pTsMgr(CTimestampManager::Pointer());
	time_t curTime = pTsMgr->GetTimestamp();
	int32_t difTime = (int32_t)(curTime - START_FILETIME);
	if(difTime < 1) {
		difTime = (int32_t)curTime;
	}

    uint32_t elapse = (uint32_t)difTime;
	
	uint64_t u64GuidField = elapse;
    u64GuidField <<= CODE_BIT;
    u64GuidField |= m_code.u16Code;
    u64GuidField <<= INSTANCE_BIT;
    u64GuidField |= atomic_inc16(&m_uCount);

    return u64GuidField;
}


