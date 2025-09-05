/*
	Code Generate by tool 2020-08-28 16:02:55
*/

#ifndef MC_PLAYERMAPBEAN_H
#define MC_PLAYERMAPBEAN_H

#include "ICacheValue.h"
#include "SpinRWLock.h"
#include "ScopedRWLock.h"
#include "BitSignSet.h"

#define PLAYERMAPBEAN_MAPIDS_INDEX 0

class CPlayerMapBean : public ICacheValue
{
public:
	CPlayerMapBean(void):
		m_mapIds(),
		m_n64Cas(0),
		m_u8ChgType(MCCHANGE_NIL),
		m_bitSigns(1)
	{}

	~CPlayerMapBean(void) {}


	std::set<uint32_t> GetMapIds() const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_mapIds;
	}

	uint64_t GetCas() const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_n64Cas;
	}

	void SetMapIds(const std::set<uint32_t>& mapIds) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_mapIds = mapIds;
		m_bitSigns.SetBit(PLAYERMAPBEAN_MAPIDS_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetCas(uint64_t n64Cas) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_n64Cas = n64Cas;
	}


	MCResult AddToCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult LoadFromCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult StoreToCache(uint64_t u64Route, const util::CValueStream& strKeys);

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
	// µØÍ¼ID
	std::set<uint32_t> m_mapIds;
	// The cache check and set flag.
	uint64_t m_n64Cas;

protected:
	volatile uint8_t m_u8ChgType;
	util::BitSignSet m_bitSigns;
	thd::CSpinRWLock m_rwTicket;
};

#endif /* __PLAYERMAPBEAN_H__ */
