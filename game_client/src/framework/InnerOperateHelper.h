/* 
 * File:   InnerOperateHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_28 17:00
 */

#ifndef _INNEROPERATE_H
#define _INNEROPERATE_H

#include "ModuleManager.h"
#include "BodyBitStream.h"
#include "BodyMessage.h"
#include "WeakPointer.h"
#include "NodeDefines.h"
#include "SmallBuffer.h"
#include "data_packet.pb.h"



#define SendInnerProtocol CInnerOperate::HandleProtocol

#define SendInnerNotification CInnerOperate::HandleNotification

#define GetInnerRequestPacket(request)\
CInnerOperate::GetRequestPacket(request, __FILE__, __LINE__)

#define GetInnerResponsePacket(reply)\
CInnerOperate::GetResponsePacket(reply, __FILE__, __LINE__)

#define GetInnerPlayer(request)\
CInnerOperate::GetPlayer(request, __FILE__, __LINE__)

#define GetInnerPlayerSilence(request)\
CInnerOperate::GetPlayerSilence(request)

#define SerializeInnerData(outPacket, message)\
CInnerOperate::SerializeData(outPacket, message, __FILE__, __LINE__)

#define ParseInnerData(outMessage, packet)\
CInnerOperate::ParseData(outMessage, packet, __FILE__, __LINE__)


class CInnerOperate {
public:
	inline static int HandleProtocol(const ::node::DataPacket& workerRequest,
		::node::DataPacket& workerResponse, int nType = 0) {

			util::CAutoPointer<::node::DataPacket> pDspRequest(&workerRequest, false);
			CBodyMessage requestBody(pDspRequest);
			util::CAutoPointer<CBodyMessage> pRequestBody(&requestBody, false);
			mdl::CNotification notification(workerRequest.cmd(), pRequestBody, nType);
			util::CAutoPointer<mdl::CNotification> pRequest(&notification, false);

			util::CAutoPointer<::node::DataPacket> pDspResponse(&workerResponse, false);
			CBodyMessage responseBody(pDspResponse);
			util::CAutoPointer<CBodyMessage> pResponseBody(&responseBody, false);
			mdl::CResponse mdlResponse(pResponseBody);
			util::CAutoPointer<mdl::CResponse> pReply(&mdlResponse, false);

			mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
			pFacade->NotifyProtocol(pRequest, pReply, false);

			return pReply->GetResult();
	}

	inline static int HandleProtocol(const ::node::DataPacket& workerRequest,
		::node::DataPacket& workerResponse, util::CWeakPointer<CPlayerBase> pPlayer, int nType = 0) {

			util::CAutoPointer<::node::DataPacket> pDspRequest(&workerRequest, false);
			CBodyMessage requestBody(pDspRequest);
			requestBody.SetPlayer(pPlayer);
			util::CAutoPointer<CBodyMessage> pRequestBody(&requestBody, false);
			mdl::CNotification notification(workerRequest.cmd(), pRequestBody, nType);
			util::CAutoPointer<mdl::CNotification> pRequest(&notification, false);

			util::CAutoPointer<::node::DataPacket> pDspResponse(&workerResponse, false);
			CBodyMessage responseBody(pDspResponse);
			util::CAutoPointer<CBodyMessage> pResponseBody(&responseBody, false);
			mdl::CResponse mdlResponse(pResponseBody);
			util::CAutoPointer<mdl::CResponse> pReply(&mdlResponse, false);

			mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
			pFacade->NotifyProtocol(pRequest, pReply, false);

			return pReply->GetResult();	
	}
	//////////////////////////////////////////////////////////////////////////
	inline static int HandleNotification(const ::node::DataPacket& workerRequest,
		::node::DataPacket& workerResponse, int nType = 0) {

			util::CAutoPointer<::node::DataPacket> pDspRequest(&workerRequest, false);
			CBodyMessage requestBody(pDspRequest);
			util::CAutoPointer<CBodyMessage> pRequestBody(&requestBody, false);
			mdl::CNotification notification(workerRequest.cmd(), pRequestBody, nType);
			util::CAutoPointer<mdl::CNotification> pRequest(&notification, false);

			util::CAutoPointer<::node::DataPacket> pDspResponse(&workerResponse, false);
			CBodyMessage responseBody(pDspResponse);
			util::CAutoPointer<CBodyMessage> pResponseBody(&responseBody, false);
			mdl::CResponse mdlResponse(pResponseBody);
			util::CAutoPointer<mdl::CResponse> pReply(&mdlResponse, false);

			mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
			pFacade->NotifyObservers(pRequest, pReply, false);

			return pReply->GetResult();
	}

	inline static int HandleNotification(const ::node::DataPacket& workerRequest,
		::node::DataPacket& workerResponse, util::CWeakPointer<CPlayerBase> pPlayer, int nType = 0) {

			util::CAutoPointer<::node::DataPacket> pDspRequest(&workerRequest, false);
			CBodyMessage requestBody(pDspRequest);
			requestBody.SetPlayer(pPlayer);
			util::CAutoPointer<CBodyMessage> pRequestBody(&requestBody, false);
			mdl::CNotification notification(workerRequest.cmd(), pRequestBody, nType);
			util::CAutoPointer<mdl::CNotification> pRequest(&notification, false);

			util::CAutoPointer<::node::DataPacket> pDspResponse(&workerResponse, false);
			CBodyMessage responseBody(pDspResponse);
			util::CAutoPointer<CBodyMessage> pResponseBody(&responseBody, false);
			mdl::CResponse mdlResponse(pResponseBody);
			util::CAutoPointer<mdl::CResponse> pReply(&mdlResponse, false);

			mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
			pFacade->NotifyObservers(pRequest, pReply, false);

			return pReply->GetResult();
	}
	//////////////////////////////////////////////////////////////////////////
	inline static util::CWeakPointer<::node::DataPacket> GetRequestPacket(
		const util::CWeakPointer<mdl::INotification>& request,
		const char* file, long line) 
	{
		if(request.IsInvalid()) {
			OutputError("request.IsInvalid()");
			return util::CWeakPointer<::node::DataPacket>();
		}

		util::CWeakPointer<CBodyMessage> pBodyMessage(request->GetBody());
		if(pBodyMessage.IsInvalid()) {
			PrintError("pBodyMessage.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<::node::DataPacket>();
		}

		if(pBodyMessage->GetMessage().IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyMessage->getMessage().IsInvalid()", file, line, __FUNCTION__);
		}

		return pBodyMessage->GetMessage();
	}

	inline static util::CWeakPointer<::node::DataPacket> GetResponsePacket(
		const util::CWeakPointer<mdl::IResponse>& reply,
		const char* file, long line) 
	{
		if(reply.IsInvalid()) {
			PrintError("file: %s line: %u @%s reply.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<::node::DataPacket>();
		}

		util::CWeakPointer<CBodyMessage> pBodyReply(reply->GetBody());
		if(pBodyReply.IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyReply.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<::node::DataPacket>();
		}

		if(pBodyReply->GetMessage().IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyReply->getMessage().IsInvalid()", file, line, __FUNCTION__);
		}
		return pBodyReply->GetMessage();
	}

	inline static util::CWeakPointer<CPlayerBase> GetPlayer(
		const util::CWeakPointer<mdl::INotification>& request,
		const char* file, long line) 
	{
		if(request.IsInvalid()) {
			PrintError("file: %s line: %u @%s request.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<CPlayerBase>();
		}

		util::CWeakPointer<CBodyMessage> pBodyMessage(request->GetBody());
		if(pBodyMessage.IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyMessage.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<CPlayerBase>();
		}

		if(pBodyMessage->GetPlayer().IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyMessage->getMessage().IsInvalid()", file, line, __FUNCTION__);
		}

		return pBodyMessage->GetPlayer();
	}

	inline static util::CWeakPointer<CPlayerBase> GetPlayerSilence(
		const util::CWeakPointer<mdl::INotification>& request) 
	{
		if(request.IsInvalid()) {
			return util::CWeakPointer<CPlayerBase>();
		}

		util::CWeakPointer<CBodyMessage> pBodyMessage(request->GetBody());
		if(pBodyMessage.IsInvalid()) {
			return util::CWeakPointer<CPlayerBase>();
		}

		return pBodyMessage->GetPlayer();
	}

	inline static bool SerializeData(util::CWeakPointer<::node::DataPacket>& pDataPacket,
		const ::google::protobuf::Message& message, const char* file, long line) 
	{
		if(pDataPacket.IsInvalid()) {
			PrintError("file: %s line: %u @%s pDataPacket.IsInvalid()", file, line, __FUNCTION__);
			return false;
		}
		ntwk::SmallBuffer smallbuf(message.ByteSize());
		if(!message.SerializeToArray((char*)smallbuf, message.ByteSize())) {
			PrintError("file: %s line: %u @%s !message.SerializeToArray", file, line, __FUNCTION__);
			return false;
		}
		pDataPacket->set_data((char*)smallbuf, message.ByteSize());
		return true;
	}

	inline static bool SerializeData(::node::DataPacket& outPacket,
		const ::google::protobuf::Message& message, const char* file, long line) 
	{
		ntwk::SmallBuffer smallbuf(message.ByteSize());
		if(!message.SerializeToArray((char*)smallbuf, message.ByteSize())) {
			PrintError("file: %s line: %u @%s !message.SerializeToArray", file, line, __FUNCTION__);
			return false;
		}
		outPacket.set_data((char*)smallbuf, message.ByteSize());
		return true;
	}

	inline static bool ParseData(::google::protobuf::Message& outMessage,
		const util::CWeakPointer<::node::DataPacket>& pDataPacket, const char* file, long line) 
	{
		if(pDataPacket.IsInvalid()) {
			PrintError("file: %s line: %u @%s pDataPacket.IsInvalid()", file, line, __FUNCTION__);
			return false;
		}
		const std::string& bytes = pDataPacket->data();

		if(bytes.empty()) {
			PrintError("file: %s line: %u @%s bytes.empty()", file, line, __FUNCTION__);
			return false;
		}

		if(!outMessage.ParseFromArray(bytes.data(), bytes.length())) {
			PrintError("file: %s line: %u @%s !outMessage.ParseFromArray", file, line, __FUNCTION__);
			return false;
		}
		return true;
	}

	inline static bool ParseData(::google::protobuf::Message& outMessage,
		const ::node::DataPacket& packet, const char* file, long line) 
	{
		const std::string& bytes = packet.data();

		if(bytes.empty()) {
			PrintError("file: %s line: %u @%s bytes.empty()", file, line, __FUNCTION__);
			return false;
		}

		if(!outMessage.ParseFromArray(bytes.data(), bytes.length())) {
			PrintError("file: %s line: %u @%s !outMessage.ParseFromArray", file, line, __FUNCTION__);
			return false;
		}
		return true;
	}

//////////////////////////////////////////////////////////////////////////

};

#endif /* _INNEROPERATE_H */

