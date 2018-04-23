/* 
 * File:   CacheMemory.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_8_4, 16:00
 */

#ifndef _CACHERECORD_H
#define	_CACHERECORD_H

#include "Common.h"
#include "ICacheMemory.h"
#include "AutoPointer.h"
#include "AtomicLock.h"
#include "PoolBase.h"
#include "CacheOperateHelper.h"
#include "StoreStream.h"
#include "TransferStream.h"

namespace util {
	class CSeparatedStream;
}

class CCacheMemory : public ICacheMemory, public util::PoolBase<CCacheMemory>
{
public:
    CCacheMemory(uint16_t u16DBID, const std::string& strKey)
        : m_u16DBID(u16DBID)
		, m_strKey(strKey)
        , m_nIndex(-1)
        , m_n64CasIncre(0)
        , m_n64Cas(0)
        , m_n64LastActivTime(0)
		, m_u8ChgType(MCCHANGE_NIL)
    {
    }

    CCacheMemory(uint16_t u16DBID, const std::string& strKey, uint64_t n64Cas)
        : m_u16DBID(u16DBID)
		, m_strKey(strKey)
        , m_nIndex(-1)
        , m_n64CasIncre(0)
        , m_n64Cas(n64Cas)
        , m_n64LastActivTime(0)
		, m_u8ChgType(MCCHANGE_NIL)
    {
    }

    CCacheMemory(uint16_t u16DBID, const std::string& strKey, util::CTransferStream& inValue)
        : m_u16DBID(u16DBID)
		, m_strKey(strKey)
        , m_strValue(inValue)
        , m_nIndex(-1)
        , m_n64CasIncre(0)
        , m_n64Cas(0)
        , m_n64LastActivTime(0)
		, m_u8ChgType(MCCHANGE_NIL)
    {
    }

    ~CCacheMemory() 
    {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_strValue.Reset(NULL, 0, false);
    }

    virtual MCResult AddToDB(bool bResetFlag = false);

    virtual MCResult LoadFromDB(
		bool bDBCas = true,
		const int32_t* pInFlag = NULL,
		int32_t* pOutFlag = NULL);

    virtual MCResult UpdateToDB(
		bool bDBCas = true,
		bool bResetFlag = false);

    virtual MCResult DeleteFromDB();

    static MCResult SelectAllFromDB(
		uint16_t u16DBID,
        const std::string& strKey,
        uint32_t nOffset,
        uint32_t nCount,
        mc_record_set_t* pRecordSet,
		bool bCacheData,
        bool bDBCas = true);

    static MCResult StoredProcedures(
		uint16_t u16DBID,
		const std::string& strProc,
		const std::string& strParam,
        mc_record_set_t* pRecordSet);

	static MCResult AsyncStoredProcedures(
		uint16_t u16DBID,
		const std::string& strProc,
		const std::string& strParam,
		mc_record_set_t* pRecordSet);

	static MCResult ChangeFlag(
		uint16_t u16DBID,
		const std::string& strKey,
		int32_t nFlag);


    virtual const std::string& GetKey() const {
        return m_strKey;
    }

    void GetValue(util::CTransferStream& outValue) {
        thd::CScopedReadLock rdLock(m_rwTicket);
		m_strValue.Serialize(outValue);
    }

	void RecoverChgType(const std::vector<util::CTypeString>& inValues, uint8_t oldChgType);

	void GetValueAndChgType(std::vector<util::CTypeString>& outValues, uint8_t& outChgType);

    virtual void SetValue(util::CTransferStream& inValue) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_strValue.ParseSetUpdate(inValue);
		++m_n64CasIncre;
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
    }



    MCResult GetsValue(util::CTransferStream& outValue, uint64_t& n64Cas) const;

    MCResult CheckAndSetValue(util::CTransferStream& inValue, uint64_t n64Cas);

    uint64_t GetCas() const {
        thd::CScopedReadLock rdLock(m_rwTicket);
        return m_n64Cas + m_n64CasIncre;
    }

protected:
	virtual int Index() const {
		return m_nIndex;
	}

	virtual void Index(int nIndex) {
		m_nIndex = nIndex;
	}

	virtual uint64_t LastActivityTime() const {
		return m_n64LastActivTime;
	}

	virtual void LastActivityTime(uint64_t n64ActivTime) {
		atomic_xchg64(&m_n64LastActivTime, n64ActivTime);
	}
	
	virtual uint8_t ChangeType() const {
		return m_u8ChgType;
	}
	
	virtual void ChangeValue(util::CTransferStream& inValue) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_strValue.ParseResetUpdate(inValue);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
	}

	inline void ResetChgType() {
		atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
	}

private:
	static bool InsertCacheRecord(
		uint16_t u16DBID,
		const std::string& strKey, 
		util::CTransferStream& inValue,
		bool bSetCas, uint64_t n64Cas);

    inline void ChangeValueAndCas(util::CTransferStream& inValue, uint64_t n64Cas) {
        thd::CScopedWriteLock wrLock(m_rwTicket);
        m_strValue.ParseResetUpdate(inValue);
        m_n64Cas = n64Cas;
		atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
    }

    inline uint64_t GetDBCas() {
        thd::CScopedReadLock rdLock(m_rwTicket);
        return m_n64Cas;
    }

    inline uint64_t FlushDBCas() {
        thd::CScopedWriteLock wrLock(m_rwTicket);
        m_n64Cas += m_n64CasIncre;
        m_n64CasIncre = 0;
        return m_n64Cas;
    }

    static std::string CastToString(...) {
        throw std::runtime_error("Unknown the type of key !");
        return std::string();
    }

    static std::string CastToString(int8_t n8Key) {
        char szBuf[64] = { '\0' };
        //snprintf(szBuf, sizeof(szBuf), "%d", (int)n8Key);
		ltostr(szBuf, n8Key, 10, 0);
        return std::string(szBuf);
    }

    static std::string CastToString(int16_t n16Key) {
        char szBuf[64] = { '\0' };
        //snprintf(szBuf, sizeof(szBuf), "%hd", n16Key);
		ltostr(szBuf, n16Key, 10, 0);
        return std::string(szBuf);
    }

    static std::string CastToString(int32_t n32Key) {
        char szBuf[64] = { '\0' };
        //snprintf(szBuf, sizeof(szBuf), "%d", n32Key);
		ltostr(szBuf, n32Key, 10, 0);
        return std::string(szBuf);
    }

    static std::string CastToString(int64_t n64Key) {
        char szBuf[64] = { '\0' };
        //snprintf(szBuf, sizeof(szBuf), SI64FMTD, n64Key);
		lltostr(szBuf, n64Key, 10, 0);
        return std::string(szBuf);
    }

    static std::string CastToString(uint8_t u8Key) {
        char szBuf[64] = { '\0' };
        //snprintf(szBuf, sizeof(szBuf), "%u", (unsigned int)u8Key);
		ultostr(szBuf, u8Key, 10, 0);
        return std::string(szBuf);
    }

    static std::string CastToString(uint16_t u16Key) {
        char szBuf[64] = { '\0' };
        //snprintf(szBuf, sizeof(szBuf), "%hu", u16Key);
		ultostr(szBuf, u16Key, 10, 0);
        return std::string(szBuf);
    }

    static std::string CastToString(uint32_t u32Key) {
        char szBuf[64] = { '\0' };
        //snprintf(szBuf, sizeof(szBuf), "%u", u32Key);
		ultostr(szBuf, u32Key, 10, 0);
        return std::string(szBuf);
    }

    static std::string CastToString(uint64_t u64Key) {
        char szBuf[64] = { '\0' };
        //snprintf(szBuf, sizeof(szBuf), I64FMTD, u64Key);
		ulltostr(szBuf, u64Key, 10, 0);
        return std::string(szBuf);
    }

private:
	uint16_t m_u16DBID;
    std::string m_strKey;
    util::CStoreStream m_strValue;
    thd::CSpinRWLock m_rwTicket;
    uint64_t m_n64CasIncre;
    uint64_t m_n64Cas;
    volatile uint64_t m_n64LastActivTime;
	int m_nIndex;
	volatile uint8_t m_u8ChgType;
};

#endif /* _CACHERECORD_H */



