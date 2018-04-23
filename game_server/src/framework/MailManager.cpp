#include "MailManager.h"
#include "Log.h"
#include "CacheOperateHelper.h"
#include "WorkerOperateHelper.h"
#include "PlayerBase.h"

bool CMailManager::SendOneselfMail(
	util::CWeakPointer<CPlayerBase> pPlayer,
	::game::SendMailPacket& sendMailPacket,
	bool bEscapeString /*= false*/)
{
	// 没有接收者，发送失败
	if(pPlayer.IsInvalid()) {
		OutputError("pPlayer.IsInvalid()");
		return false;
	}

	if(bEscapeString) {
		if(true) {
			std::string strTitle;
			McDBEscapeString(pPlayer.IsInvalid(), sendMailPacket.title(), strTitle);
			sendMailPacket.set_title(strTitle);
		}// 用于代码区间， 不用{} 是因为有些IDE 自动排版，会把括号部分剔除导致代码出错。
		if(true) {
			std::string strContent;
			McDBEscapeString(pPlayer.IsInvalid(), sendMailPacket.content(), strContent);
			sendMailPacket.set_content(strContent);
		} // 用于代码区间
	}

	::node::DataPacket dspRequest;
	dspRequest.set_cmd(N_CMD_SEND_MAIL);
	SerializeWorkerData(dspRequest, sendMailPacket);
	dspRequest.set_route(pPlayer->GetUserId());
	::node::DataPacket dspResponse;
	SendWorkerNotification(dspRequest, dspResponse, pPlayer);
	return true;
}

bool CMailManager::SendPlayersMail(
	util::CWeakPointer<CPlayerBase> pPlayer,
	uint64_t receiverId,
	::game::SendMailPacket& sendMailPacket,
	bool bEscapeString /*= false*/)
{
	if(pPlayer.IsInvalid()) {
		OutputError("pPlayer.IsInvalid()");
		return false;
	}

	uint64_t userId = pPlayer->GetUserId();

	if(bEscapeString) {
		if(true) {
			std::string strTitle;
			McDBEscapeString(userId, sendMailPacket.title(), strTitle);
			sendMailPacket.set_title(strTitle);
		}// 用于代码区间， 不用{} 是因为有些IDE 自动排版，会把括号部分剔除导致代码出错。
		if(true) {
			std::string strContent;
			McDBEscapeString(userId, sendMailPacket.content(), strContent);
			sendMailPacket.set_content(strContent);
		} // 用于代码区间
	}

	::node::DataPacket dspRequest;
	dspRequest.set_cmd(N_CMD_SEND_MAIL);
	SerializeWorkerData(dspRequest, sendMailPacket);

	dspRequest.set_route(receiverId);
	if(userId == receiverId) {
		::node::DataPacket dspResponse;
		SendWorkerNotification(dspRequest, dspResponse, pPlayer);
	} else {
		eServerError nResult = SendCachePacketToWorker(dspRequest);
		if(CACHE_ERROR_NOTFOUND == nResult) {
			::node::DataPacket dspResponse;
			SendWorkerNotification(dspRequest, dspResponse);
		}
	}
	return true;
}

bool CMailManager::SendPlayersMail(
	util::CWeakPointer<CPlayerBase> pPlayer,
	std::vector<uint64_t>& receivers,
	::game::SendMailPacket& sendMailPacket,
	bool bEscapeString /*= false*/)
{
	if(pPlayer.IsInvalid()) {
		OutputError("pPlayer.IsInvalid()");
		return false;
	}

	uint64_t userId = pPlayer->GetUserId();

	if(receivers.empty()) {
		OutputError("receivers.empty() userId = "I64FMTD, userId);
		return false;
	}

	if(bEscapeString) {
		if(true) {
			std::string strTitle;
			McDBEscapeString(userId, sendMailPacket.title(), strTitle);
			sendMailPacket.set_title(strTitle);
		}// 用于代码区间， 不用{} 是因为有些IDE 自动排版，会把括号部分剔除导致代码出错。
		if(true) {
			std::string strContent;
			McDBEscapeString(userId, sendMailPacket.content(), strContent);
			sendMailPacket.set_content(strContent);
		} // 用于代码区间
	}

	::node::DataPacket dspRequest;
	dspRequest.set_cmd(N_CMD_SEND_MAIL);
	SerializeWorkerData(dspRequest, sendMailPacket);

	for(int j = 0; j < (int)receivers.size(); ++j) {
		dspRequest.set_route(receivers[j]);
		if(userId == receivers[j]) {
			::node::DataPacket dspResponse;
			SendWorkerNotification(dspRequest, dspResponse, pPlayer);
		} else {
			eServerError nResult = SendCachePacketToWorker(dspRequest);
			if(CACHE_ERROR_NOTFOUND == nResult) {
				::node::DataPacket dspResponse;
				SendWorkerNotification(dspRequest, dspResponse);
			}
		}
	}
	return true;
}

bool CMailManager::SendPlayersMail(
	uint64_t userId,
	uint64_t receiverId,
	::game::SendMailPacket& sendMailPacket,
	bool bEscapeString /*= false*/)
{
	if(bEscapeString) {
		if(true) {
			std::string strTitle;
			McDBEscapeString(userId, sendMailPacket.title(), strTitle);
			sendMailPacket.set_title(strTitle);
		}// 用于代码区间， 不用{} 是因为有些IDE 自动排版，会把括号部分剔除导致代码出错。
		if(true) {
			std::string strContent;
			McDBEscapeString(userId, sendMailPacket.content(), strContent);
			sendMailPacket.set_content(strContent);
		} // 用于代码区间
	}

	::node::DataPacket dspRequest;
	dspRequest.set_cmd(N_CMD_SEND_MAIL);
	SerializeWorkerData(dspRequest, sendMailPacket);

	dspRequest.set_route(receiverId);
	if(userId == receiverId) {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		::node::DataPacket dspResponse;
		SendWorkerNotification(dspRequest, dspResponse,
			pChlMgr->GetPlayer(userId));
	} else {
		eServerError nResult = SendCachePacketToWorker(dspRequest);
		if(CACHE_ERROR_NOTFOUND == nResult) {
			::node::DataPacket dspResponse;
			SendWorkerNotification(dspRequest, dspResponse);
		}
	}
	return true;
}

bool CMailManager::SendPlayersMail(
    uint64_t userId,
    std::vector<uint64_t>& receivers,
    ::game::SendMailPacket& sendMailPacket,
    bool bEscapeString/* = false */)
{
    // 没有接收者，发送失败
    if(receivers.empty()) {
        OutputError("receivers.empty() userId = "I64FMTD, userId);
        return false;
    }

    if(bEscapeString) {
        if(true) {
            std::string strTitle;
            McDBEscapeString(userId, sendMailPacket.title(), strTitle);
            sendMailPacket.set_title(strTitle);
        }// 用于代码区间， 不用{} 是因为有些IDE 自动排版，会把括号部分剔除导致代码出错。
        if(true) {
            std::string strContent;
            McDBEscapeString(userId, sendMailPacket.content(), strContent);
            sendMailPacket.set_content(strContent);
        } // 用于代码区间
    }

	::node::DataPacket dspRequest;
	dspRequest.set_cmd(N_CMD_SEND_MAIL);
	SerializeWorkerData(dspRequest, sendMailPacket);

    for(int j = 0; j < (int)receivers.size(); ++j) {
		dspRequest.set_route(receivers[j]);
		if(userId == receivers[j]) {
			CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
			::node::DataPacket dspResponse;
			SendWorkerNotification(dspRequest, dspResponse,
				pChlMgr->GetPlayer(userId));
		} else {
			eServerError nResult = SendCachePacketToWorker(dspRequest);
			if(CACHE_ERROR_NOTFOUND == nResult) {
				::node::DataPacket dspResponse;
				SendWorkerNotification(dspRequest, dspResponse);
			}
		}
    }
    return true;
}








