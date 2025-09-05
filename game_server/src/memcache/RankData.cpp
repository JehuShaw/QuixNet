#include "RankData.h"
#include "CacheOperateHelper.h"
#include "Log.h"
#include "TransferStream.h"
#include "ValueStream.h"

#define CACHE_KEY_NAME "rank_mc"


MCResult CRankData::AddToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McAddOneRecord(u64Route, tsTableKey, *this);
}

MCResult CRankData::LoadFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McLoadOneRecord(u64Route, tsTableKey, *this);
}

MCResult CRankData::StoreToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McStoreOneRecord(u64Route, tsTableKey, *this);
}

MCResult CRankData::ReadOnlyFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McReadOnlyOneRecord(u64Route, tsTableKey, *this);
}

MCResult CRankData::GetsFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McGetsOneRecord(u64Route, tsTableKey, *this);
}

MCResult CRankData::CasToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McCasOneRecord(u64Route, tsTableKey, *this);
}

MCResult CRankData::DelFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
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

MCResult CRankData::InsertToDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McInsertOneRecordToDB(u64Route, tsTableKey, *this);
}

MCResult CRankData::SelectFromDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McSelectOneRecordFromDB(u64Route, tsTableKey, *this);
}

MCResult CRankData::UpdateToDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McUpdateOneRecordToDB(u64Route, tsTableKey, *this);
}

MCResult CRankData::DeleteFromDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McDeleteOneRecordFromDB(u64Route, tsTableKey);
}

void CRankData::RecoverChgType(const util::BitSignSet& inBitSigns, uint8_t oldChgType)
{
	thd::CScopedWriteLock wrLock(m_rwTicket);
	m_bitSigns = inBitSigns;
	if(MCCHANGE_UPDATE == oldChgType) {
		atomic_xchg8(&m_u8ChgType, oldChgType);
	}
}

const char* CRankData::GetCacheKeyName() const
{
	return CACHE_KEY_NAME;
}

void CRankData::Serialize(util::CTransferStream& outStream, util::BitSignSet& outBitSigns, uint8_t& outChgType)
{
	thd::CScopedReadLock rdLock(m_rwTicket);

	outStream.Serialize(m_nScore, m_bitSigns.GetBit(RANKDATA_SCORE_INDEX));
	outStream.Serialize(m_nTime, m_bitSigns.GetBit(RANKDATA_TIME_INDEX));
	outStream.Serialize(m_nCount, m_bitSigns.GetBit(RANKDATA_COUNT_INDEX));

	outBitSigns = m_bitSigns;
	m_bitSigns.ResetBitSet();
	outChgType = atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
}

void CRankData::Parse(const std::string& inArray, uint64_t n64Cas)
{
	thd::CScopedWriteLock wrLock(m_rwTicket);

	util::CTransferStream stream(inArray, false);
	stream.Parse(m_nScore);
	stream.Parse(m_nTime);
	stream.Parse(m_nCount);

	m_n64Cas = n64Cas;
	atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
	m_bitSigns.ResetBitSet();
}

void CRankData::Parse(const std::string& inArray)
{
	thd::CScopedWriteLock wrLock(m_rwTicket);

	util::CTransferStream stream(inArray, false);
	stream.Parse(m_nScore);
	stream.Parse(m_nTime);
	stream.Parse(m_nCount);

	atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
	m_bitSigns.ResetBitSet();
}


