/*
	Code Generate by tool 2016-10-24 02:24:04
*/

#ifndef __PLAYERDATA_H__
#define __PLAYERDATA_H__

#include "ICacheValue.h"
#include "SpinRWLock.h"
#include "PoolBase.h"
#include "BitSignSet.h"

class CResponseRows;

class CPlayerData : public ICacheValue, public util::PoolBase<CPlayerData>
{
public:
	CPlayerData(void):
		m_nAccount(0),
		m_strName(),
		m_nLevel(0),
		m_nExp(0),
		m_nGem(0),
		m_nCoin(0),
		m_strCreateTime(),
		m_strOfflineTime(),
		m_nStatus(0),
		m_n64Cas(0),
		m_u8ChgType(MCCHANGE_NIL),
		m_bitSigns(9)
	{}

	~CPlayerData(void) {}


	uint64_t GetAccount()const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_nAccount;
	}

	std::string GetName()const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_strName;
	}

	int32_t GetLevel()const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_nLevel;
	}

	int32_t GetExp()const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_nExp;
	}

	int32_t GetGem()const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_nGem;
	}

	int32_t GetCoin()const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_nCoin;
	}

	std::string GetCreateTime()const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_strCreateTime;
	}

	std::string GetOfflineTime()const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_strOfflineTime;
	}

	int32_t GetStatus()const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_nStatus;
	}

	uint64_t GetCas()const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_n64Cas;
	}

	void SetAccount(uint64_t nAccount) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_nAccount = nAccount;
		m_bitSigns.SetBit(0, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetName(const std::string& strName) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_strName = strName;
		m_bitSigns.SetBit(1, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetLevel(int32_t nLevel) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_nLevel = nLevel;
		m_bitSigns.SetBit(2, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetExp(int32_t nExp) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_nExp = nExp;
		m_bitSigns.SetBit(3, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetGem(int32_t nGem) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_nGem = nGem;
		m_bitSigns.SetBit(4, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetCoin(int32_t nCoin) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_nCoin = nCoin;
		m_bitSigns.SetBit(5, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetCreateTime(const std::string& strCreateTime) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_strCreateTime = strCreateTime;
		m_bitSigns.SetBit(6, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetOfflineTime(const std::string& strOfflineTime) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_strOfflineTime = strOfflineTime;
		m_bitSigns.SetBit(7, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetStatus(int32_t nStatus) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_nStatus = nStatus;
		m_bitSigns.SetBit(8, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetCas(uint64_t n64Cas) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_n64Cas = n64Cas;
	}


	static MCResult LoadAllRecord(CResponseRows& outRecords, uint64_t u64Route,
		const util::CValueStream& strKeys, uint32_t nCount = 0, uint32_t nOffset = 0);

	MCResult AddToCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult LoadFromCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult StoreToCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult GetsFromCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult CasToCache(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult DelFromCache(uint64_t u64Route, const util::CValueStream& strKeys);


	static MCResult SelectAllRecord(CResponseRows& outRecords, uint64_t u64Route,
		const util::CValueStream& strKeys, uint32_t nCount = 0, uint32_t nOffset = 0);

	MCResult InsertToDB(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult SelectFromDB(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult UpdateToDB(uint64_t u64Route, const util::CValueStream& strKeys);

	MCResult DeleteFromDB(uint64_t u64Route, const util::CValueStream& strKeys);

	uint8_t ChangeType() const { return m_u8ChgType; }

	void RecoverChgType(const util::BitSignSet& inBitSigns, uint8_t oldChgType);

	uint64_t ObjectId() const { return (uint64_t)this; }

	void Serialize(util::CTransferStream& outStream, util::BitSignSet& outBitSigns, uint8_t& outChgType);

	void Parse(const std::string& inArray, uint64_t n64Cas);

protected:
	// 角色账号ID
	uint64_t m_nAccount;
	// 角色名称
	std::string m_strName;
	// 角色等级
	int32_t m_nLevel;
	// 角色经验
	int32_t m_nExp;
	// 角色充值货币
	int32_t m_nGem;
	// 角色游戏币
	int32_t m_nCoin;
	// 角色创建时间
	std::string m_strCreateTime;
	// 角色最后一次离线时间
	std::string m_strOfflineTime;
	// 玩家状态
	int32_t m_nStatus;
	// The cache check and set flag.
	uint64_t m_n64Cas;

protected:
	volatile uint8_t m_u8ChgType;
	util::BitSignSet m_bitSigns;
	thd::CSpinRWLock m_rwTicket;
};

#endif /* __PLAYERDATA_H__ */
