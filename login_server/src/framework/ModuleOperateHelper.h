/* 
 * File:   ModuleOperate.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_28 17:00
 */

#ifndef MODULEOPERATE_H
#define MODULEOPERATE_H

#include "ModuleManager.h"
#include "BodyBitStream.h"
#include "BodyMessage.h"
#include "WeakPointer.h"
#include "NodeDefines.h"
#include "SmallBuffer.h"



#define SendModuleProtocol(data, length, socketIdx, outBodyBitStream)\
CModuleOperate::SendProtocol(data, length, socketIdx, outBodyBitStream)

#define ParseModuleProtocol(request, outMessage)\
CModuleOperate::ParseProtocol(request, outMessage)

#define ReplyModuleMessage(message, reply)\
CModuleOperate::ReplyMessage(message, reply)

class CModuleOperate {
public:

	inline static int SendProtocol(const char* data, unsigned int length,
		int socketIdx, CBodyBitStream& outBodyBitStream) {
			
		ntwk::BitStream bs((char*)data, length, false);
		int nCmd = bs.ReadInt32();
		CBodyBitStream bodyBitStream((char*)(bs.GetData() + BITS_TO_BYTES(bs.GetReadOffset())),
			BITS_TO_BYTES(bs.GetNumberOfUnreadBits()), false);
		util::CAutoPointer<CBodyBitStream> pBodyBitStream(&bodyBitStream, false);

		util::CAutoPointer<CBodyBitStream> pOutBodyBitStream(&outBodyBitStream, false);
		mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
		return pFacade->SendProtocol(nCmd, pBodyBitStream, pOutBodyBitStream, socketIdx, false, false);
	}

	inline static bool ParseProtocol(util::CWeakPointer<CBodyBitStream> pBodyBitStream, 
		::google::protobuf::Message& outMessage) {

			if(pBodyBitStream.IsInvalid()) {
				return false;
			}
			return outMessage.ParseFromArray((char*)pBodyBitStream->GetData(),
				pBodyBitStream->GetNumberOfBytesUsed());
	}

	inline static bool ReplyMessage (const ::google::protobuf::Message& message,
		util::CWeakPointer<mdl::IResponse>& reply) {

		int nByteSize = message.ByteSize();
		ntwk::SmallBuffer smallBuffer(nByteSize);
		bool rc = message.SerializeToArray((char*)smallBuffer, nByteSize);
		if(rc) {
			util::CWeakPointer<CBodyBitStream> response(reply->GetBody());
			if(response.IsInvalid()) {
				return false;
			}
			response->WriteBytes((const char*)smallBuffer, nByteSize);
		}
		return rc;
	}

//////////////////////////////////////////////////////////////////////////

	inline static int SendNotification(int cmd, const CBodyBitStream& request, 
		CBodyBitStream& reply, int type, bool reverse)
	{
		util::CAutoPointer<CBodyBitStream> pRequestBody(&request, false);
		util::CAutoPointer<CBodyBitStream> pResponseBody(&reply, false);

		mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
		return pFacade->SendNotification(cmd, pRequestBody, pResponseBody,
			type, reverse);
	}

	inline static int SendNotification(int cmd, const CBodyBitStream& request,
		CBodyBitStream& reply)
	{
		util::CAutoPointer<CBodyBitStream> pRequestBody(&request, false);
		util::CAutoPointer<CBodyBitStream> pResponseBody(&reply, false);

		mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
		return pFacade->SendNotification(cmd, pRequestBody, pResponseBody);
	}

	inline static int SendNotification(int cmd, const CBodyBitStream& request)
	{
		util::CAutoPointer<CBodyBitStream> pRequestBody(&request, false);

		mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
		return pFacade->SendNotification(cmd, pRequestBody);
	}

	inline static int SendNotification(int cmd, int type)
	{
		mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
		return pFacade->SendNotification(cmd, type);
	}

	inline static int SendNotification(int cmd)
	{
		mdl::CFacade::PTR_T pFacade(mdl::CFacade::Pointer());
		return pFacade->SendNotification(cmd);
	}

//////////////////////////////////////////////////////////////////////////

};

#endif /* MODULEOPERATE_H */

