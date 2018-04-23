#include "PlayerData.h"
#include "CacheOperateHelper.h"
#include "Log.h"
#include "TransferStream.h"
#include "ValueStream.h"

#define CACHE_TABLE_NAME "game_character"


MCResult CPlayerData::LoadAllRecord(CResponseRows& outRecords, uint64_t u64Route,
	const util::CValueStream& strKeys, uint32_t nCount/* = 0*/, uint32_t nOffset/* = 0*/)
{
	CRequestMultiRows cacheRequest;
	util::CValueStream tsTableKey;
	tsTableKey.Serialize(CACHE_TABLE_NAME);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	cacheRequest.SetKey(tsTableKey);
	if(nCount > 0) {
		cacheRequest.SetCount(nCount);
		cacheRequest.SetOffset(nOffset);
	}
	McLoadAll(u64Route, cacheRequest, outRecords);

	return outRecords.GetFirstRecordResult();
}

MCResult CPlayerData::AddToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_TABLE_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McAddOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::LoadFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_TABLE_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McLoadOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::StoreToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_TABLE_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McStoreOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::GetsFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_TABLE_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McGetsOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::CasToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_TABLE_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McCasOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::DelFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_TABLE_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	MCResult nResult = McDelOneRecord(u64Route, tsTableKey);
	if(MCERR_OK == nResult) {
		atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
	}
	return nResult;
}

MCResult CPlayerData::SelectAllRecord(CResponseRows& outRecords, uint64_t u64Route,
	const util::CValueStream& strKeys, uint32_t nCount/* = 0*/, uint32_t nOffset/* = 0*/)
{
	CRequestMultiRows cacheRequest;
	util::CValueStream tsTableKey;
	tsTableKey.Serialize(CACHE_TABLE_NAME);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	cacheRequest.SetKey(tsTableKey);
	if(nCount > 0) {
		cacheRequest.SetCount(nCount);
		cacheRequest.SetOffset(nOffset);
	}
	McDBSelectAll(u64Route, cacheRequest, outRecords);

	return outRecords.GetFirstRecordResult();
}

MCResult CPlayerData::InsertToDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_TABLE_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McInsertOneRecordToDB(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::SelectFromDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_TABLE_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McSelectOneRecordFromDB(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::UpdateToDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_TABLE_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McUpdateOneRecordToDB(u64Route, tsTableKey, *this);
}

MCResult CPlayerData::DeleteFromDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_TABLE_NAME, true);
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

void CPlayerData::Serialize(util::CTransferStream& outStream, util::BitSignSet& outBitSigns, uint8_t& outChgType)
{
	thd::CScopedReadLock rdLock(m_rwTicket);

	outStream.Serialize(m_nAccount, m_bitSigns.GetBit(0));
	outStream.Serialize(m_strName, m_bitSigns.GetBit(1));
	outStream.Serialize(m_nLevel, m_bitSigns.GetBit(2));
	outStream.Serialize(m_nExp, m_bitSigns.GetBit(3));
	outStream.Serialize(m_nGem, m_bitSigns.GetBit(4));
	outStream.Serialize(m_nCoin, m_bitSigns.GetBit(5));
	outStream.Serialize(m_strCreateTime, m_bitSigns.GetBit(6));
	outStream.Serialize(m_strOfflineTime, m_bitSigns.GetBit(7));
	outStream.Serialize(m_nStatus, m_bitSigns.GetBit(8));

	outBitSigns = m_bitSigns;
	m_bitSigns.ResetBitSet();
	outChgType = atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
}

void CPlayerData::Parse(const std::string& inArray, uint64_t n64Cas)
{
	thd::CScopedWriteLock wrLock(m_rwTicket);

	util::CTransferStream stream(inArray, false);
	stream.Parse(m_nAccount);
	stream.Parse(m_strName);
	stream.Parse(m_nLevel);
	stream.Parse(m_nExp);
	stream.Parse(m_nGem);
	stream.Parse(m_nCoin);
	stream.Parse(m_strCreateTime);
	stream.Parse(m_strOfflineTime);
	stream.Parse(m_nStatus);

	m_n64Cas = n64Cas;
	atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
	m_bitSigns.ResetBitSet();
}


