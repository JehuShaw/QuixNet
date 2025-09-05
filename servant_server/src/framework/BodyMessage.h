/* 
 * File:   BodyMessage.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 AM 11:25
 */

#ifndef BODYMESSAGE_H
#define	BODYMESSAGE_H

#include "Common.h"
#include "INotificationBody.h"
#include <google/protobuf/message.h>

class CPlayerBase;

class CBodyMessage : public mdl::IBody
{
public:
	CBodyMessage(void){}
	CBodyMessage(util::CAutoPointer<::google::protobuf::Message> message)
		:m_message(message) {
	}

    /** 
     * reset body data;
     */
    virtual void ResetBody() {
	}
	/**
	 * get message
	 */
	util::CAutoPointer<::google::protobuf::Message> GetMessage() const {
		return m_message;
	}
	/**
	 * set message                                                                   
	 */
	void SetMessage(const util::CAutoPointer<::google::protobuf::Message> message) {
		m_message = message;
	}

    /**
	 * get player
	 */
	const util::CWeakPointer<CPlayerBase>& GetPlayer() const {
		return m_player;
	}
	/**
	 * set player
	 */
	void SetPlayer(const util::CWeakPointer<CPlayerBase>& player) {
		m_player = player;
	}
private:
	util::CAutoPointer<::google::protobuf::Message> m_message;
    util::CWeakPointer<CPlayerBase> m_player;
};

#endif /* BODYMESSAGE_H */