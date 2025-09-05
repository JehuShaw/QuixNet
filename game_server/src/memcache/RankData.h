/*
	Code Generate by tool 2022-11-12 16:22:39
*/

#ifndef MC_RANKDATA_H
#define MC_RANKDATA_H

#include "ICacheValue.h"
#include "SpinRWLock.h"
#include "ScopedRWLock.h"
#include "BitSignSet.h"

#define RANKDATA_SCORE_INDEX 0
#define RANKDATA_TIME_INDEX 1
#define RANKDATA_COUNT_INDEX 2

class CRankData : public ICacheValue
{
public:
	CRankData(void):
		m_nScore(0),
		m_nTime(0),
		m_nCount(0),
		m_n64Cas(0),
		m_u8ChgType(MCCHANGE_NIL),
		m_bitSigns(3)
	{}

	CRankData(const CRankData& orig):
		m_nScore(orig.m_nScore),
		m_nTime(orig.m_nTime),
		m_nCount(orig.m_nCount),
		m_n64Cas(0),
		m_u8ChgType(MCCHANGE_NIL),
		m_bitSigns(3)
	{}

	~CRankData(void) {}

	CRankData& operator = (const CRankData& right) {
		if (this == &right) {
			return *this;
		}
		thd::CScopedReadLock rdLock(right.m_rwTicket);
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_nScore = right.m_nScore;
		m_nTime = right.m_nTime;
		m_nCount = right.m_nCount;
		return *this;
	}


	int32_t GetScore() const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_nScore;
	}

	uint32_t GetTime() const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_nTime;
	}

	uint32_t GetCount() const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_nCount;
	}

	uint64_t GetCas() const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_n64Cas;
	}

	void SetScore(int32_t nScore) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_nScore = nScore;
		m_bitSigns.SetBit(RANKDATA_SCORE_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetTime(uint32_t nTime) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_nTime = nTime;
		m_bitSigns.SetBit(RANKDATA_TIME_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetCount(uint32_t nCount) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_nCount = nCount;
		m_bitSigns.SetBit(RANKDATA_COUNT_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetCas(uint64_t n64Cas) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_n64Cas = n64Cas;
	}

	MCResult AddToCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult LoadFromCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult StoreToCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult ReadOnlyFromCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult GetsFromCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult CasToCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult DelFromCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult InsertToDB(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult SelectFromDB(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult UpdateToDB(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult DeleteFromDB(uint64_t u64Route, const util::CValueStream& strKeys);

	uint8_t ChangeType() const { return m_u8ChgType; }

	void RecoverChgType(const util::BitSignSet& inBitSigns, uint8_t oldChgType);

	const char* GetCacheKeyName() const;

	uint64_t ObjectId() const { return (uint64_t)this; }

	void Serialize(util::CTransferStream& outStream, util::BitSignSet& outBitSigns, uint8_t& outChgType);

	void Parse(const std::string& inArray, uint64_t n64Cas);

	void Parse(const std::string& inArray);

protected:
	// 用于排行比较的数值
	int32_t m_nScore;
	// 用于排行比较的更新时间
	uint32_t m_nTime;
	// 用于排行比较的排列次序累计
	uint32_t m_nCount;
	// The cache check and set flag.
	uint64_t m_n64Cas;

protected:
	volatile uint8_t m_u8ChgType;
	util::BitSignSet m_bitSigns;
	thd::CSpinRWLock m_rwTicket;
};

#endif /* MC_RANKDATA_H */
