/*
 * File:   ICacheRecord.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef __ICACHERECORD_H__
#define __ICACHERECORD_H__

#include "AutoPointer.h"
#include "Common.h"
#include "MCResult.h"

class ICacheValue;

class ICacheRecord
{
public:

	ICacheRecord() : m_bLock(false) {}
	virtual ~ICacheRecord() {}

    virtual MCResult AddToCache() = 0;

    virtual MCResult LoadFromCache() = 0;

    virtual MCResult StoreToCache() = 0;

    virtual MCResult GetsFromCache() = 0;

    virtual MCResult CasToCache() = 0;

    virtual MCResult DelFromCache() = 0;

    virtual uint8_t ChangeType() const = 0;

    virtual int Index() const = 0;

    virtual void Index(int nIndex) = 0;

    virtual uint64_t Route() const = 0;

    virtual uint64_t ObjectId() const = 0;

private:
	friend class CCacheRecordManager;
	volatile bool m_bLock;

};

#endif /* __ICACHERECORD_H__ */
