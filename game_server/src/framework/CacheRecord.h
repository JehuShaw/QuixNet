/* 
 * File:   CacheRecord.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_8_4, 16:00
 */

#ifndef CACHERECORD_H
#define	CACHERECORD_H

#include "Common.h"
#include "ICacheRecord.h"
#include "ICacheValue.h"
#include "AutoPointer.h"
#include "PoolBase.h"
#include "CacheOperateHelper.h"
#include "ValueStream.h"


class CCacheRecord : public ICacheRecord, public util::PoolBase<CCacheRecord>
{
public:
    CCacheRecord(uint64_t u64Route, const util::CValueStream& strKeys, const util::CAutoPointer<ICacheValue>& pValue)
        : m_u64Route(u64Route)
        , m_strKey(strKeys.GetData(), strKeys.GetLength(), true)
        , m_pValue(pValue)
        , m_nIndex(-1)
    {
    }

    virtual MCResult AddToCache() {
        if(m_pValue.IsInvalid()) {
            return MCERR_NOREPLY;
        }
        return m_pValue->AddToCache(m_u64Route, m_strKey);
    }

    virtual MCResult LoadFromCache() {
        if(m_pValue.IsInvalid()) {
            return MCERR_NOREPLY;
        }
        return m_pValue->LoadFromCache(m_u64Route, m_strKey);
    }

    virtual MCResult StoreToCache() {
        if(m_pValue.IsInvalid()) {
            return MCERR_NOREPLY;
        }
        return m_pValue->StoreToCache(m_u64Route, m_strKey);
    }

    virtual MCResult GetsFromCache() {
        if(m_pValue.IsInvalid()) {
            return MCERR_NOREPLY;
        }
        return m_pValue->GetsFromCache(m_u64Route, m_strKey);
    }

    virtual MCResult CasToCache() {
        if(m_pValue.IsInvalid()) {
            return MCERR_NOREPLY;
        }
        return m_pValue->CasToCache(m_u64Route, m_strKey);
    }

    virtual MCResult DelFromCache() {
        if(m_pValue.IsInvalid()) {
            return MCERR_NOREPLY;
        }
        return m_pValue->DelFromCache(m_u64Route, m_strKey);
    }

    virtual uint8_t ChangeType() const {
        if(m_pValue.IsInvalid()) {
            return MCCHANGE_NIL;
        }
        return m_pValue->ChangeType();
    }

    virtual int Index() const {
        return m_nIndex;
    }

    virtual void Index(int nIndex) {
        m_nIndex = nIndex;
    }

    virtual uint64_t Route() const {
        return m_u64Route;
    }

    virtual uint64_t ObjectId() const {
        if(m_pValue.IsInvalid()) {
            return ID_NULL;
        }
        return m_pValue->ObjectId();
    }

	virtual uint64_t Cas() const {
		if (m_pValue.IsInvalid()) {
			return 0;
		}
		return m_pValue->GetCas();
	}

	virtual const char* CacheKeyName() const {
		if (m_pValue.IsInvalid()) {
			return "";
		}
		const char* szCacheKeyName = m_pValue->GetCacheKeyName();
		if (NULL == szCacheKeyName) {
			return "";
		}
		return szCacheKeyName;
	}

private:
    uint64_t m_u64Route;
    util::CValueStream m_strKey;
    util::CAutoPointer<ICacheValue> m_pValue;
    int m_nIndex;
};

#endif /* CACHERECORD_H */



