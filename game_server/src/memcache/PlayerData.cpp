#include "PlayerData.h"
#include "CacheOperateHelper.h"
#include "Log.h"
#include "TransferStream.h"
#include "ValueStream.h"

#define CACHE_KEY_NAME "game_character"


MCResult CPlayerData::AddToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McAddOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::LoadFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McLoadOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::StoreToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McStoreOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::ReadOnlyFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McReadOnlyOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::GetsFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McGetsOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::CasToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McCasOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::DelFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	MCResult nResult = McDelOneRecord(u64Route, tsTableKey);
	if(MCERR_OK == nResult) {
		atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
	}
	return nResult;
}

MCResult CPlayerData::InsertToDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McInsertOneRecordToDB(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::SelectFromDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McSelectOneRecordFromDB(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::UpdateToDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McUpdateOneRecordToDB(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::DeleteFromDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McDeleteOneRecordFromDB(u64Route, tsTableKey);
}

void CPlayerData::RecoverChgType(const util::BitSignSet& inBitSigns, uint8_t oldChgType)
{
	thd::CScopedWriteLock wrLock(m_rwTicket);
	m_bitSigns = inBitSigns;
	if(MCCHANGE_UPDATE == oldChgType) {
		atomic_xchg8(&m_u8ChgType, oldChgType);
	}
}

const char* CPlayerData::GetCacheKeyName() const
{
	return CACHE_KEY_NAME;
}

void CPlayerData::Serialize(util::CTransferStream& outStream, util::BitSignSet& outBitSigns, uint8_t& outChgType)
{
	thd::CScopedReadLock rdLock(m_rwTicket);

	outStream.Serialize(m_nAccount, m_bitSigns.GetBit(PLAYERDATA_ACCOUNT_INDEX));
	outStream.Serialize(m_nCfgId, m_bitSigns.GetBit(PLAYERDATA_CFGID_INDEX));
	outStream.Serialize(m_strName, m_bitSigns.GetBit(PLAYERDATA_NAME_INDEX));
	outStream.Serialize(m_nLevel, m_bitSigns.GetBit(PLAYERDATA_LEVEL_INDEX));
	outStream.Serialize(m_nExp, m_bitSigns.GetBit(PLAYERDATA_EXP_INDEX));
	outStream.Serialize(m_nGem, m_bitSigns.GetBit(PLAYERDATA_GEM_INDEX));
	outStream.Serialize(m_nCoin, m_bitSigns.GetBit(PLAYERDATA_COIN_INDEX));
	outStream.Serialize(m_strCreateTime, m_bitSigns.GetBit(PLAYERDATA_CREATETIME_INDEX));
	outStream.Serialize(m_strOfflineTime, m_bitSigns.GetBit(PLAYERDATA_OFFLINETIME_INDEX));
	outStream.Serialize(m_nStatus, m_bitSigns.GetBit(PLAYERDATA_STATUS_INDEX));
	outStream.Serialize(m_nPrologueProcess, m_bitSigns.GetBit(PLAYERDATA_PROLOGUEPROCESS_INDEX));

	outBitSigns = m_bitSigns;
	m_bitSigns.ResetBitSet();
	outChgType = atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
}

void CPlayerData::Parse(const std::string& inArray, uint64_t n64Cas)
{
	thd::CScopedWriteLock wrLock(m_rwTicket);

	util::CTransferStream stream(inArray, false);
	stream.Parse(m_nAccount);
	stream.Parse(m_nCfgId);
	stream.Parse(m_strName);
	stream.Parse(m_nLevel);
	stream.Parse(m_nExp);
	stream.Parse(m_nGem);
	stream.Parse(m_nCoin);
	stream.Parse(m_strCreateTime);
	stream.Parse(m_strOfflineTime);
	stream.Parse(m_nStatus);
	stream.Parse(m_nPrologueProcess);

	m_n64Cas = n64Cas;
	atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
	m_bitSigns.ResetBitSet();
}

void CPlayerData::Parse(const std::string& inArray)
{
	thd::CScopedWriteLock wrLock(m_rwTicket);

	util::CTransferStream stream(inArray, false);
	stream.Parse(m_nAccount);
	stream.Parse(m_nCfgId);
	stream.Parse(m_strName);
	stream.Parse(m_nLevel);
	stream.Parse(m_nExp);
	stream.Parse(m_nGem);
	stream.Parse(m_nCoin);
	stream.Parse(m_strCreateTime);
	stream.Parse(m_strOfflineTime);
	stream.Parse(m_nStatus);
	stream.Parse(m_nPrologueProcess);

	atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
	m_bitSigns.ResetBitSet();
}


