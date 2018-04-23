/* 
 * File:   MailManager.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_6_25, 21:20
 */

#ifndef _MAILMANAGER_H
#define _MAILMANAGER_H

#include <string>
#include "Common.h"
#include "msg_send_mail.pb.h"
#include "WeakPointer.h"
#include "Singleton.h"

class CPlayerBase;

class CMailManager
{
public:
	CMailManager() {}

	bool SendOneselfMail(
		util::CWeakPointer<CPlayerBase> pPlayer, 
		::game::SendMailPacket& sendMailPacket,
		bool bEscapeString = false);

	bool SendPlayersMail(
		util::CWeakPointer<CPlayerBase> pPlayer,
		uint64_t receiverId,
		::game::SendMailPacket& sendMailPacket,
		bool bEscapeString = false);

	bool SendPlayersMail(
		util::CWeakPointer<CPlayerBase> pPlayer,
		std::vector<uint64_t>& receivers,
		::game::SendMailPacket& sendMailPacket,
		bool bEscapeString = false);

	bool SendPlayersMail(
		uint64_t userId,
		uint64_t receiverId,
		::game::SendMailPacket& sendMailPacket,
		bool bEscapeString = false);

    bool SendPlayersMail(
        uint64_t userId,
        std::vector<uint64_t>& receivers,
        ::game::SendMailPacket& sendMailPacket,
        bool bEscapeString = false);
};

#endif  // _MAILMANAGER_H
