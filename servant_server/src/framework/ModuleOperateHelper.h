/* 
 * File:   ModuleOperateHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2020_4_27 17:00
 */

#ifndef MODULEOPERATEHELPER_H
#define MODULEOPERATEHELPER_H

#include "ModuleManager.h"
#include "BodyBitStream.h"
#include "NodeDefines.h"
#include "Log.h"

#define SendModuleNotification CModuleOperate::HandleNotification

#define GetModuleRequestPacket(request)\
CModuleOperate::GetRequestPacket(request, __FILE__, __LINE__)

#define GetModuleResponsePacket(reply)\
CModuleOperate::GetResponsePacket(reply, __FILE__, __LINE__)


class CModuleOperate {
public:
	inline static int HandleNotification(int name,
		const CBodyBitStream& moduleRequest,
		CBodyBitStream& moduleResponse, int nType = 0) 
	{
        util::CAutoPointer<CBodyBitStream> pRequestBody(&moduleRequest, false);
        mdl::CNotification notification(name, pRequestBody, nType);
        util::CAutoPointer<mdl::CNotification> pRequest(&notification, false);

        util::CAutoPointer<CBodyBitStream> pResponseBody(&moduleResponse, false);
        mdl::CResponse mdlResponse(pResponseBody);
        util::CAutoPointer<mdl::CResponse> pReply(&mdlResponse, false);

        mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
        pFacade->NotifyObservers(pRequest, pReply, false);

        return pReply->GetResult();
	}

	inline static util::CWeakPointer<CBodyBitStream> GetRequestPacket(
		const util::CWeakPointer<mdl::INotification>& request,
		const char* file, long line) 
	{
		if(request.IsInvalid()) {
			PrintError("file: %s line: %u @%s request.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<CBodyBitStream>();
		}

		util::CWeakPointer<CBodyBitStream> pBodyMessage(request->GetBody());
		if(pBodyMessage.IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyMessage.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<CBodyBitStream>();
		}

		return pBodyMessage;
	}

	inline static util::CWeakPointer<CBodyBitStream> GetResponsePacket(
		const util::CWeakPointer<mdl::IResponse>& reply,
		const char* file, long line) 
	{
		if(reply.IsInvalid()) {
			PrintError("file: %s line: %u @%s reply.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<CBodyBitStream>();
		}

		util::CWeakPointer<CBodyBitStream> pBodyReply(reply->GetBody());
		if(pBodyReply.IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyReply.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<CBodyBitStream>();
		}

		return pBodyReply;
	}

};

#endif /* MODULEOPERATEHELPER_H */

