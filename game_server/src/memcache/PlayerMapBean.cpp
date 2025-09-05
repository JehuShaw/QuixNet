#include "PlayerMapBean.h"
#include "CacheOperateHelper.h"
#include "Log.h"
#include "TransferStream.h"
#include "ValueStream.h"

#define CACHE_KEY_NAME "game_character_map"


MCResult CPlayerMapBean::AddToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McAddOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerMapBean::LoadFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McLoadOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerMapBean::StoreToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McStoreOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerMapBean::GetsFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McGetsOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerMapBean::CasToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McCasOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerMapBean::DelFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
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

MCResult CPlayerMapBean::InsertToDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McInsertOneRecordToDB(u64Route, tsTableKey, *this);
}

MCResult CPlayerMapBean::SelectFromDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McSelectOneRecordFromDB(u64Route, tsTableKey, *this);
}

MCResult CPlayerMapBean::UpdateToDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McUpdateOneRecordToDB(u64Route, tsTableKey, *this);
}

MCResult CPlayerMapBean::DeleteFromDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McDeleteOneRecordFromDB(u64Route, tsTableKey);
}

void CPlayerMapBean::RecoverChgType(const util::BitSignSet& inBitSigns, uint8_t oldChgType)
{
	thd::CScopedWriteLock wrLock(m_rwTicket);
	m_bitSigns = inBitSigns;
	if(MCCHANGE_UPDATE == oldChgType) {
		atomic_xchg8(&m_u8ChgType, oldChgType);
	}
}

const char* CPlayerMapBean::GetCacheKeyName() const
{
	return CACHE_KEY_NAME;
}

void CPlayerMapBean::Serialize(util::CTransferStream& outStream, util::BitSignSet& outBitSigns, uint8_t& outChgType)
{
	thd::CScopedReadLock rdLock(m_rwTicket);

	outStream.Serialize(m_mapIds, m_bitSigns.GetBit(PLAYERMAPBEAN_MAPIDS_INDEX));

	outBitSigns = m_bitSigns;
	m_bitSigns.ResetBitSet();
	outChgType = atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
}

void CPlayerMapBean::Parse(const std::string& inArray, uint64_t n64Cas)
{
	thd::CScopedWriteLock wrLock(m_rwTicket);

	util::CTransferStream stream(inArray, false);
	stream.Parse(m_mapIds);

	m_n64Cas = n64Cas;
	atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
	m_bitSigns.ResetBitSet();
}

void CPlayerMapBean::Parse(const std::string& inArray)
{
	thd::CScopedWriteLock wrLock(m_rwTicket);

	util::CTransferStream stream(inArray, false);
	stream.Parse(m_mapIds);

	atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
	m_bitSigns.ResetBitSet();
}


