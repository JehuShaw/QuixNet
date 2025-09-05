/* 
 * File:   PlayerBase.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef PLAYERBASE_H
#define	PLAYERBASE_H

#include "WrapObject.h"

class CPlayerBase {
public:
    CPlayerBase()  {

    }

    virtual ~CPlayerBase() { 
    }

	virtual uint64_t GetUserID() const {
		throw std::runtime_error("Method GetUserID() not implemented.");
	} 
};

class CWrapPlayer : public thd::CWrapObject<CPlayerBase> {
public:
	CWrapPlayer() : thd::CWrapObject<CPlayerBase>() {}

	CWrapPlayer(util::CAutoPointer<CPlayerBase>& pPlayer) : thd::CWrapObject<CPlayerBase>(pPlayer) {}

	uint64_t GetUserID() const {
		if (m_object.IsInvalid()) {
			return ID_NULL;
		}

		return m_object->GetUserID();
	}

	inline bool IsObjectInvalid() const {
		return m_object.IsInvalid();
	}
};

#endif /* PLAYERBASE_H */
