/* 
 * File:   ArgumentMessage.h
 * Author: Jehu Shaw
 *
 * Created on 2020_07_07, 14:35
 */

#ifndef ARGUMENTMESSAGE_H
#define	ARGUMENTMESSAGE_H

#include "BitStream.h"
#include "AgentMethod.h"

class CWrapPlayer;

namespace node {
	class DataPacket;
}

class CArgumentMessage : public evt::ArgumentBase, public util::PoolBase<CArgumentMessage>
{
public:
	CArgumentMessage(const std::string& param, std::string* response)
		: m_param(param), m_response(response) {}

	CArgumentMessage(const std::string& param, std::string* response, util::CWeakPointer<CWrapPlayer>& pPlayer)
		: m_param(param), m_response(response), m_pPlayer(pPlayer) {}

	virtual void Reset() {}

	inline const std::string& GetParam() const {
		return m_param;
	}

	inline util::CWeakPointer<CWrapPlayer>& GetPlayer() {
		return m_pPlayer;
	}

	inline std::string* GetResponse() {
		return m_response;
	}

private:
	const std::string& m_param;
	std::string* m_response;
	util::CWeakPointer<CWrapPlayer> m_pPlayer;
};

#endif	/* ARGUMENTMESSAGE_H */

