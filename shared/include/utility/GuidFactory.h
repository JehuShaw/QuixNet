/* 
 * File:   GuidFactory.h
 * Author: Jehu Shaw
 *
 * Created on 2011_7_26, 11:09
 */

#ifndef GUIDFACTORY_H
#define	GUIDFACTORY_H

#include <stdint.h>
#include "AtomicLock.h"
#include "TimestampManager.h"


namespace util {

	// 2014-1-1 00:00:00
#define START_FILETIME (1391184000)

	/**
	 * Create 64bits guid,  Unix timestamp,  since 1970.1.1 to 2149.1.1
	 */
	template<int typeValue>
	class CGuidFactory
	{
#define TYPE_MASK 0x7
#define TYPE_BIT 12

#define TIMESTAMP_MASK 0xFFFFFFFF
#define TIMESTAMP_BIT 32

#define INSTANCE_MASK 0x1FFFF
#define INSTANCE_BIT 17
	public:
		CGuidFactory() : m_uCount(0)
		{
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 ) || defined( _WIN64 )
			BUILD_BUG_ON(sizeof(uint32_t) < sizeof(unsigned long));
#endif
			evt::CTimestampManager::Pointer();
		}
		/**
		 * Generate 64 bits id 
		 * @return 
		 */
		uint64_t GenerateID()
		{
			evt::CTimestampManager::PTR_T pTsMgr(evt::CTimestampManager::Pointer());
			time_t curTime = pTsMgr->GetTimestamp();
			int32_t difTime = static_cast<int32_t>(curTime - START_FILETIME);
			if (difTime < 1) {
				difTime = static_cast<int32_t>(curTime);
			}

			uint32_t elapse = static_cast<uint32_t>(difTime);

			uint64_t u64GuidField = typeValue & TYPE_MASK;
			u64GuidField <<= (TYPE_BIT + TIMESTAMP_BIT);
			u64GuidField |= elapse;
			u64GuidField <<= INSTANCE_BIT;
			u64GuidField |= atomic_inc(&m_uCount) & INSTANCE_MASK;

			return u64GuidField;
		}

	private:
		CGuidFactory(const CGuidFactory &) : m_uCount(0) {}

		CGuidFactory& operator = (const CGuidFactory &) { return *this; }

	private:
		volatile uint32_t m_uCount;
	};

}

#endif	/* GUIDFACTORY_H */

