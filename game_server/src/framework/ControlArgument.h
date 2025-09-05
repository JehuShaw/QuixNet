/* 
 * File:   ControlArgument.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef CONTROLARGUMENT_H
#define	CONTROLARGUMENT_H

#include "AgentMethod.h"
#include "worker.pb.h"
#include "PlayerBase.h"

class CControlArgument : public evt::ArgumentBase {
public:
	CControlArgument(const ::node::DataPacket& request,
					 const util::CWeakPointer<CWrapPlayer>& pPlayer,
					 util::CWeakPointer<::node::DataPacket>& pResponse)
		: m_request(request)
		, m_pPlayer(pPlayer)
		, m_pResponse(pResponse) {}

	const ::node::DataPacket& GetRequest() const { return m_request; }

	/**
	 * get player
	 */
	const util::CWeakPointer<CWrapPlayer>& GetPlayer() const {
		return m_pPlayer;
	}

	inline void SetResponseData(const ::google::protobuf::Message& message) {
		SerializeWorkerData(m_pResponse, message);
	}

private:
	const ::node::DataPacket& m_request;
	util::CWeakPointer<CWrapPlayer> m_pPlayer;
	util::CWeakPointer<::node::DataPacket> m_pResponse;
};

#endif	/* CONTROLARGUMENT_H */

