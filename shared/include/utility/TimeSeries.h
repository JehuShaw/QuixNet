/* 
 * File:   TimeSeries.h
 * Author: Jehu Shaw
 *
 * Created on 2016_5_13, 17:38
 */

#ifndef _TIMESERIES_H_
#define	_TIMESERIES_H_

#include "Common.h"

namespace util {
/**
 * Create 64bits value,  Unix timestamp,  since 1970.1.1 to 2149.1.1
 */
class SHARED_DLL_DECL CTimeSeries {
public:
	CTimeSeries();

	~CTimeSeries();
    /**
     * create 64 bits value 
     * @return 
     */
    uint64_t Generate();

	void Generate(uint32_t& u32Hight, uint32_t& u32Low);

private:
	volatile uint32_t m_uCount;
};

}

#endif	/* _TIMESERIES_H_ */

