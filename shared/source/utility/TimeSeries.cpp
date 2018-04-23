/* 
 * File:   TimeSeries.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2016_5_13, 17:38
 */

#include "TimeSeries.h"
#include "AtomicLock.h"
#include "TimestampManager.h"

// 2014-1-1 00:00:00
#define START_FILETIME (1391184000)

#define TIMESTAMP_MASK 0xFFFFFFFF
#define TIMESTAMP_BIT 32

using namespace util;
using namespace evt;

CTimeSeries::CTimeSeries()
	: m_uCount(0) {

	CTimestampManager::Pointer();
}

CTimeSeries::~CTimeSeries() {
}

uint64_t CTimeSeries::Generate() {
	CTimestampManager::PTR_T pTsMgr(CTimestampManager::Pointer());
	time_t curTime = pTsMgr->GetTimestamp();
	int32_t difTime = (int32_t)(curTime - START_FILETIME);
	if(difTime < 1) {
		difTime = (int32_t)curTime;
	}

    uint32_t elapse = (uint32_t)difTime;
	
	uint64_t u64GuidField = elapse;
    u64GuidField <<= TIMESTAMP_BIT;
    u64GuidField |= (uint32_t)atomic_inc(&m_uCount);

    return u64GuidField;
}

void CTimeSeries::Generate(uint32_t& u32Hight, uint32_t& u32Low) {
	CTimestampManager::PTR_T pTsMgr(CTimestampManager::Pointer());
	time_t curTime = pTsMgr->GetTimestamp();
	int32_t difTime = (int32_t)(curTime - START_FILETIME);
	if(difTime < 1) {
		difTime = (int32_t)curTime;
	}

	u32Hight = (uint32_t)difTime;
	u32Low = (uint32_t)atomic_inc(&m_uCount);
}

