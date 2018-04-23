/*
 * File:   ICacheValue.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_5 AM 23:37
 */

#ifndef __ICACHEVALUE_H__
#define __ICACHEVALUE_H__

#include <string>
#include "Common.h"
#include "AutoPointer.h"
#include "MCResult.h"

enum MCChangeType {
	MCCHANGE_NIL,
	MCCHANGE_UPDATE,
};

namespace util {
	class BitSignSet;
	class CValueStream;
	class CTransferStream;
}

class ICacheValue
{
public:

	virtual ~ICacheValue() {}

    virtual MCResult AddToCache(uint64_t u64Route, const util::CValueStream& strKeys) = 0;

    virtual MCResult LoadFromCache(uint64_t u64Route, const util::CValueStream& strKeys) = 0;

    virtual MCResult StoreToCache(uint64_t u64Route, const util::CValueStream& strKeys) = 0;

    virtual MCResult GetsFromCache(uint64_t u64Route, const util::CValueStream& strKeys) = 0;

    virtual MCResult CasToCache(uint64_t u64Route, const util::CValueStream& strKeys) = 0;

    virtual MCResult DelFromCache(uint64_t u64Route, const util::CValueStream& strKeys) = 0;

    virtual uint64_t ObjectId() const = 0;

    virtual uint64_t GetCas() const = 0;

	virtual void Serialize(util::CTransferStream& outStream, util::BitSignSet& outBitSigns, uint8_t& outChgType) = 0;

	virtual void Parse(const std::string& inArray, uint64_t n64Cas) = 0;

	virtual uint8_t ChangeType() const = 0;

	virtual void SetCas(uint64_t n64Cas) = 0;

	virtual void RecoverChgType(const util::BitSignSet& inBitSigns, uint8_t oldChgType) = 0;

};

#endif /* __ICACHEVALUE_H__ */
