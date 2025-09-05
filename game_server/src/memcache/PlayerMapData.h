/*
	Code Generate by tool 2020-08-27 09:31:07
*/

#ifndef MC_PLAYERMAPDATA_H
#define MC_PLAYERMAPDATA_H

#include "PlayerMapBean.h"

class CPlayerMapData : public CPlayerMapBean
{
public:
	bool HadMap(uint32_t nMapId)const {
		thd::CScopedReadLock rdLock(m_rwTicket);
		return m_mapIds.end() != m_mapIds.find(nMapId);
	}

	void AddMap(uint32_t nMapId) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_mapIds.insert(nMapId);
		m_bitSigns.SetBit(PLAYERMAPBEAN_MAPIDS_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void RemoveMap(uint32_t nMapId) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_mapIds.erase(nMapId);
		m_bitSigns.SetBit(PLAYERMAPBEAN_MAPIDS_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}
};

#endif /* MC_PLAYERMAPDATA_H */
