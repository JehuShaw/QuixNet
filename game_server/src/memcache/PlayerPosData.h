/*
 * File:   QuestData.h
 * Author: Jehu Shaw
 *
 * Created on 2020_8_21, 19:36
 */

#ifndef MC_PLAYERPOSDATA_H
#define MC_PLAYERPOSDATA_H

#include "PlayerPosBean.h"

class CPlayerPosData : public CPlayerPosBean
{
public:

	void SetPos(float fX, float fY, float fZ) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_fX = fX;
		m_fY = fY;
		m_fZ = fZ;
		m_bitSigns.SetBit(PLAYERPOSBEAN_X_INDEX, true);
		m_bitSigns.SetBit(PLAYERPOSBEAN_Y_INDEX, true);
		m_bitSigns.SetBit(PLAYERPOSBEAN_Z_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetPos(const std::vector<float>* pVec) {
		if (NULL == pVec) {
			return;
		}
		assert(pVec->size() > 2);

		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_fX = pVec->at(0);
		m_fY = pVec->at(1);
		m_fZ = pVec->at(2);
		m_bitSigns.SetBit(PLAYERPOSBEAN_X_INDEX, true);
		m_bitSigns.SetBit(PLAYERPOSBEAN_Y_INDEX, true);
		m_bitSigns.SetBit(PLAYERPOSBEAN_Z_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetFace(float fFaceX, float fFaceY, float fFaceZ) {
		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_fFaceX = fFaceX;
		m_fFaceY = fFaceY;
		m_fFaceZ = fFaceZ;
		m_bitSigns.SetBit(PLAYERPOSBEAN_FACEX_INDEX, true);
		m_bitSigns.SetBit(PLAYERPOSBEAN_FACEY_INDEX, true);
		m_bitSigns.SetBit(PLAYERPOSBEAN_FACEZ_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}

	void SetFace(const std::vector<float>* pVec) {
		if (NULL == pVec) {
			return;
		}
		assert(pVec->size() > 2);

		thd::CScopedWriteLock wrLock(m_rwTicket);
		m_fFaceX = pVec->at(0);
		m_fFaceY = pVec->at(1);
		m_fFaceZ = pVec->at(2);
		m_bitSigns.SetBit(PLAYERPOSBEAN_FACEX_INDEX, true);
		m_bitSigns.SetBit(PLAYERPOSBEAN_FACEY_INDEX, true);
		m_bitSigns.SetBit(PLAYERPOSBEAN_FACEZ_INDEX, true);
		atomic_xchg8(&m_u8ChgType, MCCHANGE_UPDATE);
	}
};

#endif /* MC_PLAYERPOSDATA_H */
