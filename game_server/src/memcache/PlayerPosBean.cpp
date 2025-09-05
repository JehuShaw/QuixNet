#include "PlayerPosBean.h"
#include "CacheOperateHelper.h"
#include "Log.h"
#include "TransferStream.h"
#include "ValueStream.h"

#define CACHE_KEY_NAME "game_character_pos"


MCResult CPlayerPosBean::AddToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McAddOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerPosBean::LoadFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McLoadOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerPosBean::StoreToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McStoreOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerPosBean::GetsFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McGetsOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerPosBean::CasToCache(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McCasOneRecord(u64Route, tsTableKey, *this);
}

MCResult CPlayerPosBean::DelFromCache(uint64_t u64Route, const util::CValueStream& strKeys)
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

MCResult CPlayerPosBean::InsertToDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McInsertOneRecordToDB(u64Route, tsTableKey, *this);
}

MCResult CPlayerPosBean::SelectFromDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McSelectOneRecordFromDB(u64Route, tsTableKey, *this);
}

MCResult CPlayerPosBean::UpdateToDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McUpdateOneRecordToDB(u64Route, tsTableKey, *this);
}

MCResult CPlayerPosBean::DeleteFromDB(uint64_t u64Route, const util::CValueStream& strKeys)
{
	util::CTransferStream tsTableKey;
	tsTableKey.Serialize(CACHE_KEY_NAME, true);
	if(strKeys.GetLength() > 0) {
		tsTableKey.WriteBytes(strKeys.GetData(), strKeys.GetLength());
	}
	return McDeleteOneRecordFromDB(u64Route, tsTableKey);
}

void CPlayerPosBean::RecoverChgType(const util::BitSignSet& inBitSigns, uint8_t oldChgType)
{
	thd::CScopedWriteLock wrLock(m_rwTicket);
	m_bitSigns = inBitSigns;
	if(MCCHANGE_UPDATE == oldChgType) {
		atomic_xchg8(&m_u8ChgType, oldChgType);
	}
}

const char* CPlayerPosBean::GetCacheKeyName() const
{
	return CACHE_KEY_NAME;
}

void CPlayerPosBean::Serialize(util::CTransferStream& outStream, util::BitSignSet& outBitSigns, uint8_t& outChgType)
{
	thd::CScopedReadLock rdLock(m_rwTicket);

	outStream.Serialize(m_fX, m_bitSigns.GetBit(PLAYERPOSBEAN_X_INDEX));
	outStream.Serialize(m_fY, m_bitSigns.GetBit(PLAYERPOSBEAN_Y_INDEX));
	outStream.Serialize(m_fZ, m_bitSigns.GetBit(PLAYERPOSBEAN_Z_INDEX));
	outStream.Serialize(m_fFaceX, m_bitSigns.GetBit(PLAYERPOSBEAN_FACEX_INDEX));
	outStream.Serialize(m_fFaceY, m_bitSigns.GetBit(PLAYERPOSBEAN_FACEY_INDEX));
	outStream.Serialize(m_fFaceZ, m_bitSigns.GetBit(PLAYERPOSBEAN_FACEZ_INDEX));

	outBitSigns = m_bitSigns;
	m_bitSigns.ResetBitSet();
	outChgType = atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
}

void CPlayerPosBean::Parse(const std::string& inArray, uint64_t n64Cas)
{
	thd::CScopedWriteLock wrLock(m_rwTicket);

	util::CTransferStream stream(inArray, false);
	stream.Parse(m_fX);
	stream.Parse(m_fY);
	stream.Parse(m_fZ);
	stream.Parse(m_fFaceX);
	stream.Parse(m_fFaceY);
	stream.Parse(m_fFaceZ);

	m_n64Cas = n64Cas;
	atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
	m_bitSigns.ResetBitSet();
}

void CPlayerPosBean::Parse(const std::string& inArray)
{
	thd::CScopedWriteLock wrLock(m_rwTicket);

	util::CTransferStream stream(inArray, false);
	stream.Parse(m_fX);
	stream.Parse(m_fY);
	stream.Parse(m_fZ);
	stream.Parse(m_fFaceX);
	stream.Parse(m_fFaceY);
	stream.Parse(m_fFaceZ);

	atomic_xchg8(&m_u8ChgType, MCCHANGE_NIL);
	m_bitSigns.ResetBitSet();
}


