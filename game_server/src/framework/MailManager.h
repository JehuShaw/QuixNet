/* 
 * File:   MailManager.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_6_25, 21:20
 */

#ifndef MAILMANAGER_H
#define MAILMANAGER_H

#include "Common.h"
#include "Singleton.h"
#include "PlayerOperateHelper.h"
#include "msg_send_mail.pb.h"


class CMailManager : public util::Singleton<CMailManager>
{
public:
	CMailManager() {}

	INLINE static eServerError SendPlayerMail(
		uint64_t senderId,
		uint64_t receiverId,
		::game::SendMailPacket& sendMailPacket)
	{
		if (ID_NULL == receiverId) {
			return MAIL_RECEIVERID_NULL;
		}

		if (senderId == receiverId) {
			return MAIL_CANNT_SEND_SELF;
		}
		return SendSynPlayer(senderId, receiverId, N_CMD_SEND_MAIL, sendMailPacket, NULL);
	}

	INLINE static eServerError PostPlayerMail(
		uint64_t senderId,
		uint64_t receiverId,
		::game::SendMailPacket& sendMailPacket)
	{
		if (ID_NULL == receiverId) {
			return MAIL_RECEIVERID_NULL;
		}

		if (senderId == receiverId) {
			return MAIL_CANNT_SEND_SELF;
		}
		return PostPlayer(senderId, receiverId, N_CMD_SEND_MAIL, sendMailPacket);
	}
};

#endif  // _MAILMANAGER_H
