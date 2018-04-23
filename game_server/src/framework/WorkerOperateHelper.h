/* 
 * File:   WorkerOperate.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_28 17:00
 */

#ifndef WORKEROPERATE_H
#define WORKEROPERATE_H

#include "ModuleManager.h"
#include "BodyMessage.h"
#include "NodeDefines.h"
#include "ChannelManager.h"
#include "rpc_controller.hpp"
#include "worker.rpcz.h"
#include "Log.h"
#include "SmallBuffer.h"

// set record operator
#define SetWorkerData(workerRequest, data) workerRequest->set_data(data)

// get result
#define GetWorkerResult(workerResponse) workerResponse->result()
// get data
#define GetWorkerData(workerResponse) workerResponse->data()


#define SendWorkerProtocol CWorkerOperate::HandleProtocol

#define SendWorkerNotification CWorkerOperate::HandleNotification

#define SendWorkerCmdToClient(userId, cmd)\
CWorkerOperate::SendToClient(userId, cmd, __FILE__, __LINE__)

#define SendWorkerToClient(userId, cmd, message)\
CWorkerOperate::SendToClient(userId, cmd, message, __FILE__, __LINE__)

#define SendWorkerPacketToClient(dataPacket)\
CWorkerOperate::SendToClient(dataPacket, __FILE__, __LINE__)

#define BroadcastWorkerCmdToClient(cmd, excludeId)\
CWorkerOperate::BroadcastToClient(cmd, excludeId, __FILE__, __LINE__)

#define BroadcastWorkerToClient(cmd, message, excludeId)\
CWorkerOperate::BroadcastToClient(cmd, message, excludeId, __FILE__, __LINE__)

#define BroadcastWorkerPacketToClient(dataPacket, excludeId)\
CWorkerOperate::BroadcastToClient(dataPacket, excludeId, __FILE__, __LINE__)

#define CloseWorkerClient(userId)\
CWorkerOperate::CloseClient(userId, __FILE__, __LINE__)

#define CloseWorkerAllClient()\
CWorkerOperate::CloseAllClient(__FILE__, __LINE__)

#define SendWorkerCmdToWorker(userId, cmd)\
CWorkerOperate::SendToWorker(userId, cmd, __FILE__, __LINE__)

#define SendWorkerToWorker(userId, cmd, message)\
CWorkerOperate::SendToWorker(userId, cmd, message, __FILE__, __LINE__)

#define SendWorkerPacketToWorker(dataPacket)\
CWorkerOperate::SendToWorker(dataPacket, __FILE__, __LINE__)

#define KickWorkerLogged(userId)\
CWorkerOperate::KickLogged(userId, __FILE__, __LINE__)

#define GetWorkerRequestPacket(request)\
CWorkerOperate::GetRequestPacket(request, __FILE__, __LINE__)

#define GetWorkerResponsePacket(reply)\
CWorkerOperate::GetResponsePacket(reply, __FILE__, __LINE__)

#define GetWorkerPlayer(request)\
CWorkerOperate::GetPlayer(request, __FILE__, __LINE__)

#define GetWorkerPlayerSilence(request)\
CWorkerOperate::GetPlayerSilence(request)

#define SerializeWorkerData(outPacket, message)\
CWorkerOperate::SerializeData(outPacket, message, __FILE__, __LINE__)

#define ParseWorkerData(outMessage, packet)\
CWorkerOperate::ParseData(outMessage, packet, __FILE__, __LINE__)

enum eWSResult {
	WS_SUCCESS,
	WS_NOTFOUND,
	WS_FAIL,
};

class CWorkerOperate {
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

	inline static util::CWeakPointer<::node::DataPacket> GetRequestPacket(
		const util::CWeakPointer<mdl::INotification>& request,
		const char* file, long line) 
	{
		if(request.IsInvalid()) {
			PrintError("file: %s line: %u @%s request.IsInvalid()", file, line, __FUNCTION__);
			return util::CWeakPointer<::node::DataPacket>();
		}

		util::CWeakPointer<CBodyMessage> pBodyMessage(request->GetBody());
		if(pBodyMessage.IsInvalid()) {
			PrintError("file: %s line: %u @%s pBodyMessage.IsInvalid()", file, line, __FUNCTION__);
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

		int nByteSize = message.ByteSize();
        ntwk::SmallBuffer smallbuf(nByteSize);
        if(!message.SerializeToArray((char*)smallbuf, nByteSize)) {
            PrintError("file: %s line: %u @%s !message.SerializeToArray", file, line, __FUNCTION__);
            return false;
        }
        pDataPacket->set_data((char*)smallbuf, nByteSize);
        return true;
    }

	inline static bool SerializeData(::node::DataPacket& outPacket, 
		const ::google::protobuf::Message& message, const char* file, long line) 
	{
		int nByteSize = message.ByteSize();
		ntwk::SmallBuffer smallbuf(nByteSize);
		if(!message.SerializeToArray((char*)smallbuf, nByteSize)) {
			PrintError("file: %s line: %u @%s !message.SerializeToArray", file, line, __FUNCTION__);
			return false;
		}
		outPacket.set_data((char*)smallbuf, nByteSize);
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

	inline static eWSResult SendToClient(
		uint64_t userId, int32_t cmd,
		const char* file, long line) 
	{
		::node::DataPacket dataPacket;
		dataPacket.set_route(userId);
		dataPacket.set_cmd(cmd);
		dataPacket.set_result(TRUE);

		return SendToClient(dataPacket, file, line);
	}

	inline static eWSResult SendToClient(
		uint64_t userId, int32_t cmd,
		const ::google::protobuf::Message& message,
		const char* file, long line) 
	{
		::node::DataPacket dataPacket;
		dataPacket.set_route(userId);
		dataPacket.set_cmd(cmd);
		dataPacket.set_result(TRUE);

		int nByteSize = message.ByteSize();
		ntwk::SmallBuffer smallbuf(nByteSize);
		if(!message.SerializeToArray((char*)smallbuf, nByteSize)) {
			PrintError("file: %s line: %u @%s !message.SerializeToArray", file, line, __FUNCTION__);
			return WS_FAIL;
		}
		dataPacket.set_data((char*)smallbuf, nByteSize);
		return SendToClient(dataPacket, file, line);
	}

	inline static eWSResult SendToClient(
		const util::CWeakPointer<::node::DataPacket>& pDataPacket,
		const char* file, long line) 
	{
		return SendToClient(*pDataPacket, file, line);
	}

	inline static eWSResult SendToClient(
		const ::node::DataPacket& dataPacket,
		const char* file, long line) 
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> channel(pChlMgr->GetRpczChannel(dataPacket.route()));
		if(channel.IsInvalid()) {
			return WS_NOTFOUND;
		}

		::node::WorkerService_Stub workerService_stub(&*channel, false);
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		::node::DataPacket dpResponse;
		workerService_stub.SendToClient(dataPacket, &dpResponse, &controller, NULL);
		controller.wait();
		pChlMgr->CheckClientChannel(dataPacket.route(), !controller.ok());
		if(controller.ok()) {
			return WS_SUCCESS;
		}
		return WS_FAIL;
	}

	inline static void BroadcastToClient(
		int32_t cmd,
		uint64_t excludeId,
		const char* file, long line) 
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);
		dataPacket.set_result(TRUE);

		BroadcastToClient(dataPacket, excludeId, file, line);
	}

	inline static void BroadcastToClient(
		int32_t cmd,
		const ::google::protobuf::Message& message,
		uint64_t excludeId,
		const char* file, long line) 
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);
		dataPacket.set_result(TRUE);

		int nByteSize = message.ByteSize();
		ntwk::SmallBuffer smallbuf(nByteSize);
		if(!message.SerializeToArray((char*)smallbuf, nByteSize)) {
			PrintError("file: %s line: %u @%s !message.SerializeToArray"
				, file, line, __FUNCTION__);
			return;
		}
		dataPacket.set_data((char*)smallbuf, nByteSize);
		BroadcastToClient(dataPacket, excludeId, file, line);
	}

	inline static void BroadcastToClient(
		const util::CWeakPointer<::node::DataPacket>& pDataPacket,
		uint64_t excludeId,
		const char* file, long line) 
	{
		BroadcastToClient(*pDataPacket, excludeId, file, line);
	}

	inline static void BroadcastToClient(
        const ::node::DataPacket& dataPacket,
		uint64_t excludeId,
		const char* file, long line) 
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		std::vector<struct UserIdAndChannel> channels;
		pChlMgr->IteratorClientChannel(channels);

		std::vector<struct UserIdAndChannel>::iterator it(channels.begin());
		for(; channels.end() != it; ++it) {

			if(it->pChannel.IsInvalid()) {
				PrintError("file: %s line: %u @%s channel.IsInvalid()", file, line, __FUNCTION__);
				continue;
			}

			if(ID_NULL != excludeId && it->userId == excludeId) {
				continue;
			}

			::node::WorkerService_Stub workerService_stub(&*it->pChannel, false);
			rpcz::rpc_controller controller;
			controller.set_deadline_ms(CALL_DEADLINE_MS);
			::node::DataPacket dpResponse;
            const_cast<::node::DataPacket&>(dataPacket).set_route(it->userId);
			workerService_stub.SendToClient(dataPacket, &dpResponse, &controller, NULL);
			controller.wait();
			pChlMgr->CheckClientChannel(it->userId, !controller.ok());
		}
	}

    inline static eWSResult CloseClient(uint64_t userId, const char* file, long line)
    {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<rpcz::rpc_channel> channel(pChlMgr->GetRpczChannel(userId));
        if(channel.IsInvalid()) {
            return WS_NOTFOUND;
        }

        ::node::WorkerService_Stub workerService_stub(&*channel, false);
        ::node::DataPacket dataPacket;
        dataPacket.set_cmd(0);
        dataPacket.set_route(userId);
        rpcz::rpc_controller controller;
        controller.set_deadline_ms(CALL_DEADLINE_MS);
        ::node::DataPacket dpResponse;
        workerService_stub.CloseClient(dataPacket, &dpResponse, &controller, NULL);
        controller.wait();
        pChlMgr->CheckClientChannel(userId, !controller.ok());
        if(controller.ok()) {
			return WS_SUCCESS;
		}
		return WS_FAIL;
    }

    inline static void CloseAllClient(const char* file, long line) 
    {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

        std::vector<struct UserIdAndChannel> channels;
        pChlMgr->IteratorClientChannel(channels);

        std::vector<struct UserIdAndChannel>::iterator it(channels.begin());
        for(; channels.end() != it; ++it) {

            if(it->pChannel.IsInvalid()) {
                PrintError("file: %s line: %u @%s channel.IsInvalid()", file, line, __FUNCTION__);
                continue;
            }

            ::node::WorkerService_Stub workerService_stub(&*it->pChannel, false);
            ::node::DataPacket dataPacket;
            dataPacket.set_cmd(0);
            dataPacket.set_route(it->userId);
            rpcz::rpc_controller controller;
            controller.set_deadline_ms(CALL_DEADLINE_MS);
            ::node::DataPacket dpResponse;
            workerService_stub.CloseClient(dataPacket, &dpResponse, &controller, NULL);
            controller.wait();
            pChlMgr->CheckClientChannel(it->userId, !controller.ok());
        }
    }

	inline static eWSResult SendToWorker(
		uint64_t userId, int32_t cmd,
		const char* file, long line) 
	{
		::node::DataPacket dataPacket;
		dataPacket.set_route(userId);
		dataPacket.set_cmd(cmd);
		dataPacket.set_result(TRUE);

		return SendToWorker(dataPacket, file, line);
	}

	inline static eWSResult SendToWorker(
		uint64_t userId, int32_t cmd,
		const ::google::protobuf::Message& message,
		const char* file, long line) 
	{
		::node::DataPacket dataPacket;
		dataPacket.set_route(userId);
		dataPacket.set_cmd(cmd);
		dataPacket.set_result(TRUE);

		int nByteSize = message.ByteSize();
		ntwk::SmallBuffer smallbuf(nByteSize);
		if(!message.SerializeToArray((char*)smallbuf, nByteSize)) {
			PrintError("file: %s line: %u @%s !message.SerializeToArray", file, line, __FUNCTION__);
			return WS_FAIL;
		}
		dataPacket.set_data((char*)smallbuf, nByteSize);
		return SendToWorker(dataPacket, file, line);
	}

    inline static eWSResult SendToWorker(
        const ::node::DataPacket& dataPacket,
        const char* file, long line) 
    {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<rpcz::rpc_channel> channel(pChlMgr->GetRpczChannel(dataPacket.route()));
        if(channel.IsInvalid()) {
            return WS_NOTFOUND;
        }

        ::node::WorkerService_Stub workerService_stub(&*channel, false);
        rpcz::rpc_controller controller;
        controller.set_deadline_ms(CALL_DEADLINE_MS);
        ::node::DataPacket dpResponse;
        workerService_stub.SendToWorker(dataPacket, &dpResponse, &controller, NULL);
        controller.wait();
        pChlMgr->CheckClientChannel(dataPacket.route(), !controller.ok());
        if(controller.ok()) {
			return WS_SUCCESS;
		}
		return WS_FAIL;
    }

	inline static eWSResult SendToWorker(
		const util::CWeakPointer<::node::DataPacket>& pDataPacket,
		const char* file, long line) 
	{
		return SendToWorker(*pDataPacket, file, line);
	}

	inline static eWSResult KickLogged(uint64_t userId, const char* file, long line)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> channel(pChlMgr->GetRpczChannel(userId));
		if(channel.IsInvalid()) {
			return WS_NOTFOUND;
		}

		::node::WorkerService_Stub workerService_stub(&*channel, false);
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(0);
		dataPacket.set_route(userId);
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		::node::DataPacket dpResponse;
		workerService_stub.KickLogged(dataPacket, &dpResponse, &controller, NULL);
		controller.wait();
		pChlMgr->CheckClientChannel(userId, !controller.ok());
		if(controller.ok()) {
			if(dpResponse.result() == TRUE) {
				return WS_SUCCESS;
			} else {
				return WS_NOTFOUND;
			}
		}
		return WS_FAIL;
	}
};

#endif /* WORKEROPERATE_H */

