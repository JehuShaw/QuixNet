/*
	Code Generate by tool 2020-10-24 13:03:39
*/

#ifndef MC_PLAYERPOSBEAN_H
#define MC_PLAYERPOSBEAN_H

#include "ICacheValue.h"
#include "SpinRWLock.h"
#include "ScopedRWLock.h"
#include "BitSignSet.h"

#define PLAYERPOSBEAN_X_INDEX 0
#define PLAYERPOSBEAN_Y_INDEX 1
#define PLAYERPOSBEAN_Z_INDEX 2
#define PLAYERPOSBEAN_FACEX_INDEX 3
#define PLAYERPOSBEAN_FACEY_INDEX 4
#define PLAYERPOSBEAN_FACEZ_INDEX 5

class CPlayerPosBean : public ICacheValue
{
public:
	CPlayerPosBean(void):
		m_fX(0),
		m_fY(0),
		m_fZ(0),
		m_fFaceX(0),
		m_fFaceY(0),
		m_fFaceZ(0),
		m_n64Cas(0),
		m_u8ChgType(MCCHANGE_NIL),
		m_bitSigns(6)
	{}

	~CPlayerPosBean(void) {}


	float GetX() const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_fX;
	}

	float GetY() const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_fY;
	}

	float GetZ() const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_fZ;
	}

	float GetFaceX() const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_fFaceX;
	}

	float GetFaceY() const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_fFaceY;
	}

	float GetFaceZ() const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_fFaceZ;
	}

	uint64_t GetCas() const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_n64Cas;
	}

	void SetX(float fX) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_fX = fX;
		m_bitSigns.SetBit(PLAYERPOSBEAN_X_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetY(float fY) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_fY = fY;
		m_bitSigns.SetBit(PLAYERPOSBEAN_Y_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetZ(float fZ) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_fZ = fZ;
		m_bitSigns.SetBit(PLAYERPOSBEAN_Z_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetFaceX(float fFaceX) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_fFaceX = fFaceX;
		m_bitSigns.SetBit(PLAYERPOSBEAN_FACEX_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetFaceY(float fFaceY) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_fFaceY = fFaceY;
		m_bitSigns.SetBit(PLAYERPOSBEAN_FACEY_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetFaceZ(float fFaceZ) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_fFaceZ = fFaceZ;
		m_bitSigns.SetBit(PLAYERPOSBEAN_FACEZ_INDEX, true);
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
	// X坐标
	float m_fX;
	// Y坐标
	float m_fY;
	// Z坐标
	float m_fZ;
	// 朝向x
	float m_fFaceX;
	// 朝向x
	float m_fFaceY;
	// 朝向z
	float m_fFaceZ;
	// The cache check and set flag.
	uint64_t m_n64Cas;

protected:
	volatile uint8_t m_u8ChgType;
	util::BitSignSet m_bitSigns;
	thd::CSpinRWLock m_rwTicket;
};

#endif /* MC_PLAYERPOSBEAN_H */
