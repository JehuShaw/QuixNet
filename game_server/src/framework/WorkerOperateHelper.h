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


 // check data operator
#define HasDataPacketRoute(ref) ref.route() != 0
#define HasDataPacketData(ref) !ref.data().empty()

#define HasDataPacketDataPtr(ptr) !ptr->data().empty()

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

#define BroadcastWorkerCmdToClient(route, cmd, includeIds, excludeIds)\
CWorkerOperate::BroadcastToClient(route, cmd, includeIds, excludeIds, __FILE__, __LINE__)

#define BroadcastWorkerToClient(route, cmd, message, includeIds, excludeIds)\
CWorkerOperate::BroadcastToClient(route, cmd, message, includeIds, excludeIds, __FILE__, __LINE__)

#define BroadcastWorkerPacketToClient(dataPacket, includeIds, excludeIds)\
CWorkerOperate::BroadcastToClient(dataPacket, includeIds, excludeIds, __FILE__, __LINE__)

#define BroadcastPacketToWorker(broadcastPacket)\
CWorkerOperate::BroadcastToClient(broadcastPacket, __FILE__, __LINE__)

#define CloseWorkerClient(userId)\
CWorkerOperate::CloseClient(userId, __FILE__, __LINE__)

#define CloseWorkerAllNodeClients()\
CWorkerOperate::CloseAllNodeClients(__FILE__, __LINE__)

#define CloseWorkerAllClients(broadcastPacket)\
CWorkerOperate::CloseAllClients(broadcastPacket, __FILE__, __LINE__)

#define SendWorkerCmdToWorker(userId, cmd)\
CWorkerOperate::SendToWorker(userId, cmd, __FILE__, __LINE__)

#define SendWorkerToWorker(userId, cmd, message)\
CWorkerOperate::SendToWorker(userId, cmd, message, __FILE__, __LINE__)

#define SendWorkerPacketToWorker(dataPacket)\
CWorkerOperate::SendToWorker(dataPacket, __FILE__, __LINE__)

#define KickWorkerLogged(userId)\
CWorkerOperate::KickLogged(userId, __FILE__, __LINE__)

#define SendWorkerPacketToPlayer(dataRequest, dataResponse)\
CWorkerOperate::SendToPlayer(dataRequest, dataResponse, __FILE__, __LINE__)

#define PostWorkerPacketToPlayer(dataRequest)\
CWorkerOperate::PostToPlayer(dataRequest, __FILE__, __LINE__)

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

#define InvokeWorkerNotification(inConnect, cmd, userId, inMessage, outMessage)\
CWorkerOperate::InvokeNotification(inConnect, cmd, userId, inMessage, outMessage, __FILE__, __LINE__)


class CWorkerOperate {
public:
	inline static int HandleProtocol(int32_t cmd, uint64_t route,
		const ::google::protobuf::Message* const pInMessage = NULL,
		::google::protobuf::Message* const pOutMessage = NULL,
		eRouteType routeType = ROUTE_BALANCE_USERID,
		int32_t result = 0,
		int nType = 0,
		const char* file = __FILE__,
		long line = __LINE__)
	{
		::node::DataPacket workerRequest;
		workerRequest.set_cmd(cmd);
		workerRequest.set_route(route);
		workerRequest.set_route_type(routeType);
		workerRequest.set_result(result);
		if (NULL != pInMessage) {
			SerializeData(workerRequest, *pInMessage, file, line);
		}
		::node::DataPacket workerResponse;
		HandleProtocol(workerRequest, workerResponse, nType);
		if (NULL != pOutMessage) {
			ParseData(*pOutMessage, workerResponse, file, line);
		}
		return workerResponse.result();
	}

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

        return workerResponse.result();
	}

	inline static int HandleProtocol(
		int32_t cmd, uint64_t route,
		util::CWeakPointer<CWrapPlayer> pPlayer,
		const ::google::protobuf::Message* const pInMessage = NULL,
		::google::protobuf::Message* const pOutMessage = NULL,
		eRouteType routeType = ROUTE_BALANCE_USERID,
		int32_t result = 0,
		int nType = 0,
		const char* file = __FILE__,
		long line = __LINE__)
	{
		::node::DataPacket workerRequest;
		workerRequest.set_cmd(cmd);
		workerRequest.set_route(route);
		workerRequest.set_route_type(routeType);
		workerRequest.set_result(result);
		if (NULL != pInMessage) {
			SerializeData(workerRequest, *pInMessage, file, line);
		}
		::node::DataPacket workerResponse;
		HandleProtocol(workerRequest, workerResponse, pPlayer, nType);
		if (NULL != pOutMessage) {
			ParseData(*pOutMessage, workerResponse, file, line);
		}
		return workerResponse.result();
	}

    inline static int HandleProtocol(const ::node::DataPacket& workerRequest,
        ::node::DataPacket& workerResponse, util::CWeakPointer<CWrapPlayer> pPlayer, int nType = 0) {

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

        return workerResponse.result();
    }

	inline static int HandleNotification(int32_t cmd, uint64_t route,
		const ::google::protobuf::Message* const pInMessage = NULL,
		::google::protobuf::Message* const pOutMessage = NULL,
		eRouteType routeType = ROUTE_BALANCE_USERID,
		int32_t result = 0,
		int nType = 0,
		const char* file = __FILE__,
		long line = __LINE__)
	{
		::node::DataPacket workerRequest;
		workerRequest.set_cmd(cmd);
		workerRequest.set_route(route);
		workerRequest.set_route_type(routeType);
		workerRequest.set_result(result);
		if(NULL != pInMessage) {
			SerializeData(workerRequest, *pInMessage, file, line);
		}
		::node::DataPacket workerResponse;
		HandleNotification(workerRequest, workerResponse, nType);
		if(NULL != pOutMessage) {
			ParseData(*pOutMessage, workerResponse, file, line);
		}
		return workerResponse.result();
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

        return workerResponse.result();
	}

	inline static int HandleNotification(
		int32_t cmd, uint64_t route,
		util::CWeakPointer<CWrapPlayer> pPlayer,
		const ::google::protobuf::Message* const pInMessage = NULL,
		::google::protobuf::Message* const pOutMessage = NULL,
		eRouteType routeType = ROUTE_BALANCE_USERID,
		int32_t result = 0,
		int nType = 0,
		const char* file = __FILE__,
		long line = __LINE__)
	{
		::node::DataPacket workerRequest;
		workerRequest.set_cmd(cmd);
		workerRequest.set_route(route);
		workerRequest.set_route_type(routeType);
		workerRequest.set_result(result);
		if (NULL != pInMessage) {
			SerializeData(workerRequest, *pInMessage, file, line);
		}
		::node::DataPacket workerResponse;
		HandleNotification(workerRequest, workerResponse, pPlayer, nType);
		if (NULL != pOutMessage) {
			ParseData(*pOutMessage, workerResponse, file, line);
		}
		return workerResponse.result();
	}

    inline static int HandleNotification(const ::node::DataPacket& workerRequest,
        ::node::DataPacket& workerResponse, util::CWeakPointer<CWrapPlayer> pPlayer, int nType = 0) {

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

        return workerResponse.result();
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

    inline static util::CWeakPointer<CWrapPlayer> GetPlayer(
        const util::CWeakPointer<mdl::INotification>& request,
        const char* file, long line) 
    {
        if(request.IsInvalid()) {
            PrintError("file: %s line: %u @%s request.IsInvalid()", file, line, __FUNCTION__);
            return util::CWeakPointer<CWrapPlayer>();
        }

        util::CWeakPointer<CBodyMessage> pBodyMessage(request->GetBody());
        if(pBodyMessage.IsInvalid()) {
            PrintError("file: %s line: %u @%s pBodyMessage.IsInvalid()", file, line, __FUNCTION__);
            return util::CWeakPointer<CWrapPlayer>();
        }

        if(pBodyMessage->GetPlayer().IsInvalid()) {
            PrintError("file: %s line: %u @%s pBodyMessage->getMessage().IsInvalid()", file, line, __FUNCTION__);
        }

        return pBodyMessage->GetPlayer();
    }

    inline static util::CWeakPointer<CWrapPlayer> GetPlayerSilence(
        const util::CWeakPointer<mdl::INotification>& request) 
    {
        if(request.IsInvalid()) {
            return util::CWeakPointer<CWrapPlayer>();
        }

        util::CWeakPointer<CBodyMessage> pBodyMessage(request->GetBody());
        if(pBodyMessage.IsInvalid()) {
            return util::CWeakPointer<CWrapPlayer>();
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

		if (!message.SerializeToString(pDataPacket->mutable_data())) {
			PrintError("file: %s line: %u @%s !message.SerializeToString", file, line, __FUNCTION__);
			return false;
		}
        return true;
    }

	inline static bool SerializeData(::node::DataPacket& outPacket, 
		const ::google::protobuf::Message& message, const char* file, long line) 
	{
		if (!message.SerializeToString(outPacket.mutable_data())) {
			PrintError("file: %s line: %u @%s !message.SerializeToString", file, line, __FUNCTION__);
			return false;
		}
		return true;
	}

	inline static bool SerializeData(std::string& outData,
		const ::google::protobuf::Message& message, const char* file, long line)
	{
		if (!message.SerializeToString(&outData)) {
			PrintError("file: %s line: %u @%s !message.SerializeToString", file, line, __FUNCTION__);
			return false;
		}
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

		if(!outMessage.ParseFromArray(bytes.data(), bytes.length())) {
			PrintError("file: %s line: %u @%s !outMessage.ParseFromArray", file, line, __FUNCTION__);
			return false;
		}
		return true;
	}

	inline static bool ParseData(::google::protobuf::Message& outMessage,
		const std::string& bytes, const char* file, long line)
	{
		if (!outMessage.ParseFromArray(bytes.data(), bytes.length())) {
			PrintError("file: %s line: %u @%s !outMessage.ParseFromArray", file, line, __FUNCTION__);
			return false;
		}
		return true;
	}

	inline static eServerError SendToClient(
		uint64_t userId, int32_t cmd,
		const char* file, long line) 
	{
		::node::DataPacket dataPacket;
		dataPacket.set_route(userId);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_cmd(cmd);
		dataPacket.set_result(SERVER_SUCCESS);

		return SendToClient(dataPacket, file, line);
	}

	inline static eServerError SendToClient(
		uint64_t userId, int32_t cmd,
		const ::google::protobuf::Message& message,
		const char* file, long line) 
	{
		::node::DataPacket dataPacket;
		dataPacket.set_route(userId);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_cmd(cmd);
		dataPacket.set_result(SERVER_SUCCESS);

		if (!message.SerializeToString(dataPacket.mutable_data())) {
			PrintError("file: %s line: %u @%s !message.SerializeToString", file, line, __FUNCTION__);
			return SERVER_ERROR_SERIALIZE;
		}
		return SendToClient(dataPacket, file, line);
	}

	inline static eServerError SendToClient(
		const util::CWeakPointer<::node::DataPacket>& pDataPacket,
		const char* file, long line) 
	{
		return SendToClient(*pDataPacket, file, line);
	}

	inline static eServerError SendToClient(
		const ::node::DataPacket& dataPacket,
		const char* file, long line) 
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		util::CAutoPointer<rpcz::rpc_channel> channel(pChlMgr->GetRpczChannel(dataPacket.route()));
		if(channel.IsInvalid()) {
			PrintError("file: %s line: %u @%s channel.IsInvalid()", file, line, __FUNCTION__);
			return SERVER_ERROR_NOTFOUND_CHANNEL;
		}

		::node::WorkerService_Stub workerService_stub(&*channel, false);
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		::node::DataPacket dpResponse;
		workerService_stub.SendToClient(dataPacket, &dpResponse, &controller, NULL);
		controller.wait();

		pChlMgr->CheckClientChannel(dataPacket.route(), !controller.ok());

		const rpcz::status_code curStatus = controller.get_status();
		if (curStatus == rpcz::status::DEADLINE_EXCEEDED) {
			return SERVER_CALL_DEADLINE;
		} else if (curStatus == rpcz::status::OK) {
			return SERVER_SUCCESS;
		}
		return SERVER_FAILURE;
	}

	inline static void BroadcastToClient(
		uint64_t route,
		int32_t cmd,
		const std::set<uint64_t>* includeIds,
		const std::set<uint64_t>* excludeIds,
		const char* file, long line) 
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_route(route);
		dataPacket.set_result(SERVER_SUCCESS);

		BroadcastToClient(dataPacket, includeIds, excludeIds, file, line);
	}

	inline static void BroadcastToClient(
		uint64_t route,
		int32_t cmd,
		const ::google::protobuf::Message& message,
		const std::set<uint64_t>* includeIds,
		const std::set<uint64_t>* excludeIds,
		const char* file, long line) 
	{
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(cmd);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_route(route);
		dataPacket.set_result(SERVER_SUCCESS);

		if (!message.SerializeToString(dataPacket.mutable_data())) {
			PrintError("file: %s line: %u @%s !message.SerializeToString"
				, file, line, __FUNCTION__);
			return;
		}
		BroadcastToClient(dataPacket, includeIds, excludeIds, file, line);
	}

	inline static void BroadcastToClient(
		const util::CWeakPointer<::node::DataPacket>& pDataPacket,
		const std::set<uint64_t>* includeIds,
		const std::set<uint64_t>* excludeIds,
		const char* file, long line) 
	{
		BroadcastToClient(*pDataPacket, includeIds, excludeIds, file, line);
	}

	inline static void BroadcastToClient(
		const ::node::DataPacket& dataPacket,
		const std::set<uint64_t>* includeIds,
		const std::set<uint64_t>* excludeIds,
		const char* file, long line)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		CChannelManager::CHLKEY_AGTID_MAP_T channelKeys;
		pChlMgr->IteratorAgentChannelKey(channelKeys, NULL);

		if (!channelKeys.empty()) {
			::node::BroadcastRequest broadcastPacket;

			if (NULL == includeIds || includeIds->empty()) {
				if (NULL != excludeIds) {
					std::set<uint64_t>::const_iterator itEx(excludeIds->begin());
					for (; excludeIds->end() != itEx; ++itEx) {
						broadcastPacket.add_excludeids(*itEx);
					}
				}
			} else {
				std::set<uint64_t>::const_iterator itIn(includeIds->begin());
				for (; includeIds->end() != itIn; ++itIn) {
					broadcastPacket.add_includeids(*itIn);
				}
			}

			if (!dataPacket.SerializeToString(broadcastPacket.mutable_data())) {
				PrintError("file: %s line: %u @%s !dataPacket.SerializeToString(broadcastPacket)"
					, file, line, __FUNCTION__);
				return;
			}

			std::vector< rpcz::rpc_controller> ctrls(channelKeys.size());
			int nSize = 0;
			CChannelManager::CHLKEY_AGTID_MAP_T::iterator it(channelKeys.begin());
			for (; channelKeys.end() != it; ++it) {
				util::CAutoPointer<CRpczChannel> pRpczChannel(pChlMgr->FindRpczChannel(it->first));
				if (pRpczChannel.IsInvalid()){
					PrintError("file: %s line: %u @%s pRpczChannel.IsInvalid()", file, line, __FUNCTION__);
					continue;
				}
				util::CAutoPointer<rpcz::rpc_channel> pChannel(pRpczChannel->GetChannel());
				if (pChannel.IsInvalid()) {
					PrintError("file: %s line: %u @%s pChannel.IsInvalid()", file, line, __FUNCTION__);
					continue;
				}
				CChannelManager::AGENT_ID_SET_T::iterator itAg(it->second.begin());
				for (; it->second.end() != itAg; ++itAg) {
					broadcastPacket.add_agentids(*itAg);
				}

				::node::WorkerService_Stub workerService_stub(&*pChannel, false);
				rpcz::rpc_controller& controller = ctrls[nSize++];
				controller.set_deadline_ms(CALL_DEADLINE_MS);
				workerService_stub.BroadcastToClient(broadcastPacket, NULL, &controller, NULL);
			}
			// wait
			for (int i = 0; i < nSize; ++i) {
				ctrls[i].wait();
			}
		}
	}

	inline static void BroadcastToClient(
        ::node::BroadcastRequest& broadcastPacket,
		const char* file, long line) 
	{
		CChannelManager::AGENT_ID_SET_T agentIds;
		int nSizeAg = broadcastPacket.agentids_size();
		for (int i = 0; i < nSizeAg; ++i) {
			agentIds.insert(broadcastPacket.agentids(i));
		}

		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		CChannelManager::CHLKEY_AGTID_MAP_T channelKeys;
		pChlMgr->IteratorAgentChannelKey(channelKeys, &agentIds);

		if (!channelKeys.empty()) {
			std::vector< rpcz::rpc_controller> ctrls(channelKeys.size());
			int nSize = 0;
			CChannelManager::CHLKEY_AGTID_MAP_T::iterator it(channelKeys.begin());
			for (; channelKeys.end() != it; ++it) {
				util::CAutoPointer<CRpczChannel> pRpczChannel(pChlMgr->FindRpczChannel(it->first));
				if (pRpczChannel.IsInvalid()) {
					PrintError("file: %s line: %u @%s pRpczChannel.IsInvalid()", file, line, __FUNCTION__);
					continue;
				}
				util::CAutoPointer<rpcz::rpc_channel> pChannel(pRpczChannel->GetChannel());
				if (pChannel.IsInvalid()) {
					PrintError("file: %s line: %u @%s pChannel.IsInvalid()", file, line, __FUNCTION__);
					continue;
				}

				broadcastPacket.clear_agentids();
				CChannelManager::AGENT_ID_SET_T::iterator itAg (it->second.begin());
				for (; it->second.end() != itAg; ++itAg) {
					broadcastPacket.add_agentids(*itAg);
				}

				::node::WorkerService_Stub workerService_stub(&*pChannel, false);
				rpcz::rpc_controller& controller = ctrls[nSize++];
				controller.set_deadline_ms(CALL_DEADLINE_MS);
				workerService_stub.BroadcastToClient(broadcastPacket, NULL, &controller, NULL);
			}
			// wait
			for (int i = 0; i < nSize; ++i) {
				ctrls[i].wait();
			}
		}
	}

    inline static eServerError CloseClient(uint64_t userId, const char* file, long line)
    {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<rpcz::rpc_channel> channel(pChlMgr->GetRpczChannel(userId));
        if(channel.IsInvalid()) {
			PrintError("file: %s line: %u @%s channel.IsInvalid()", file, line, __FUNCTION__);
            return SERVER_ERROR_NOTFOUND_CHANNEL;
        }

        ::node::WorkerService_Stub workerService_stub(&*channel, false);
        ::node::DataPacket dataPacket;
        dataPacket.set_cmd(0);
        dataPacket.set_route(userId);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
        rpcz::rpc_controller controller;
        controller.set_deadline_ms(CALL_DEADLINE_MS);
        ::node::DataPacket dpResponse;
        workerService_stub.CloseClient(dataPacket, &dpResponse, &controller, NULL);
        controller.wait();
        pChlMgr->CheckClientChannel(userId, !controller.ok());

		const rpcz::status_code curStatus = controller.get_status();
		if (curStatus == rpcz::status::DEADLINE_EXCEEDED) {
			return SERVER_CALL_DEADLINE;
		} else if (curStatus == rpcz::status::OK) {
			return SERVER_SUCCESS;
		}
		return SERVER_FAILURE;
    }

	inline static void CloseAllNodeClients(const char* file, long line)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		CChannelManager::CHLKEY_AGTID_MAP_T channelKeys;
		pChlMgr->IteratorAgentChannelKey(channelKeys, NULL);

		if (!channelKeys.empty()) {
			::node::BroadcastRequest broadcastPacket;

			pChlMgr->IteratorClient(broadcastPacket.mutable_includeids());

			std::vector< rpcz::rpc_controller> ctrls(channelKeys.size());
			int nSize = 0;
			CChannelManager::CHLKEY_AGTID_MAP_T::iterator it(channelKeys.begin());
			for (; channelKeys.end() != it; ++it) {
				util::CAutoPointer<CRpczChannel> pRpczChannel(pChlMgr->FindRpczChannel(it->first));
				if (pRpczChannel.IsInvalid()) {
					PrintError("file: %s line: %u @%s pRpczChannel.IsInvalid()", file, line, __FUNCTION__);
					continue;
				}
				util::CAutoPointer<rpcz::rpc_channel> pChannel(pRpczChannel->GetChannel());
				if (pChannel.IsInvalid()) {
					PrintError("file: %s line: %u @%s pChannel.IsInvalid()", file, line, __FUNCTION__);
					continue;
				}

				CChannelManager::AGENT_ID_SET_T::iterator itAg(it->second.begin());
				for (; it->second.end() != itAg; ++itAg) {
					broadcastPacket.add_agentids(*itAg);
				}

				::node::WorkerService_Stub workerService_stub(&*pChannel, false);
				rpcz::rpc_controller& controller = ctrls[nSize++];
				controller.set_deadline_ms(CALL_DEADLINE_MS);
				workerService_stub.CloseAllClients(broadcastPacket, NULL, &controller, NULL);
			}
			// wait
			for (int i = 0; i < nSize; ++i) {
				ctrls[i].wait();
			}
		}
	}

	inline static void CloseAllClients(
		::node::BroadcastRequest& broadcastPacket,
		const char* file, long line)
	{
		CChannelManager::AGENT_ID_SET_T agentIds;
		int nSizeAg = broadcastPacket.agentids_size();
		for (int i = 0; i < nSizeAg; ++i) {
			agentIds.insert(broadcastPacket.agentids(i));
		}

		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		CChannelManager::CHLKEY_AGTID_MAP_T channelKeys;
		pChlMgr->IteratorAgentChannelKey(channelKeys, &agentIds);

		if (!channelKeys.empty()) {
			std::vector< rpcz::rpc_controller> ctrls(channelKeys.size());
			int nSize = 0;
			CChannelManager::CHLKEY_AGTID_MAP_T::iterator it(channelKeys.begin());
			for (; channelKeys.end() != it; ++it) {
				util::CAutoPointer<CRpczChannel> pRpczChannel(pChlMgr->FindRpczChannel(it->first));
				if (pRpczChannel.IsInvalid()) {
					PrintError("file: %s line: %u @%s pRpczChannel.IsInvalid()", file, line, __FUNCTION__);
					continue;
				}
				util::CAutoPointer<rpcz::rpc_channel> pChannel(pRpczChannel->GetChannel());
				if (pChannel.IsInvalid()) {
					PrintError("file: %s line: %u @%s pChannel.IsInvalid()", file, line, __FUNCTION__);
					continue;
				}

				broadcastPacket.clear_agentids();
				CChannelManager::AGENT_ID_SET_T::iterator itAg(it->second.begin());
				for (; it->second.end() != itAg; ++itAg) {
					broadcastPacket.add_agentids(*itAg);
				}

				::node::WorkerService_Stub workerService_stub(&*pChannel, false);
				rpcz::rpc_controller& controller = ctrls[nSize++];
				controller.set_deadline_ms(CALL_DEADLINE_MS);
				workerService_stub.CloseAllClients(broadcastPacket, NULL, &controller, NULL);
			}
			// wait
			for (int i = 0; i < nSize; ++i) {
				ctrls[i].wait();
			}
		}
	}

	inline static eServerError SendToWorker(
		uint64_t userId, int32_t cmd,
		const char* file, long line) 
	{
		::node::DataPacket dataPacket;
		dataPacket.set_route(userId);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_cmd(cmd);
		dataPacket.set_result(TRUE);

		return SendToWorker(dataPacket, file, line);
	}

	inline static eServerError SendToWorker(
		uint64_t userId, int32_t cmd,
		const ::google::protobuf::Message& message,
		const char* file, long line) 
	{
		::node::DataPacket dataPacket;
		dataPacket.set_route(userId);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		dataPacket.set_cmd(cmd);
		dataPacket.set_result(TRUE);

		if (!message.SerializeToString(dataPacket.mutable_data())) {
			PrintError("file: %s line: %u @%s !message.SerializeToString", file, line, __FUNCTION__);
			return SERVER_ERROR_SERIALIZE;
		}
		return SendToWorker(dataPacket, file, line);
	}

    inline static eServerError SendToWorker(
        const ::node::DataPacket& dataPacket,
        const char* file, long line) 
    {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<rpcz::rpc_channel> channel(pChlMgr->GetRpczChannel(dataPacket.route()));
        if(channel.IsInvalid()) {
			PrintError("file: %s line: %u @%s channel.IsInvalid()", file, line, __FUNCTION__);
            return SERVER_ERROR_NOTFOUND_CHANNEL;
        }

        ::node::WorkerService_Stub workerService_stub(&*channel, false);
        rpcz::rpc_controller controller;
        controller.set_deadline_ms(CALL_DEADLINE_MS);
        ::node::DataPacket dpResponse;
        workerService_stub.SendToWorker(dataPacket, &dpResponse, &controller, NULL);
        controller.wait();
        pChlMgr->CheckClientChannel(dataPacket.route(), !controller.ok());

		const rpcz::status_code curStatus = controller.get_status();
		if (curStatus == rpcz::status::DEADLINE_EXCEEDED) {
			return SERVER_CALL_DEADLINE;
		} else if (curStatus == rpcz::status::OK) {
			return SERVER_SUCCESS;
		}
		return SERVER_FAILURE;
    }

	inline static eServerError SendToWorker(
		const util::CWeakPointer<::node::DataPacket>& pDataPacket,
		const char* file, long line) 
	{
		return SendToWorker(*pDataPacket, file, line);
	}

	inline static eServerError KickLogged(uint64_t userId, const char* file, long line)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> channel(pChlMgr->GetRpczChannel(userId));
		if(channel.IsInvalid()) {
			PrintError("file: %s line: %u @%s channel.IsInvalid()", file, line, __FUNCTION__);
			return SERVER_ERROR_NOTFOUND_CHANNEL;
		}

		::node::WorkerService_Stub workerService_stub(&*channel, false);
		::node::DataPacket dataPacket;
		dataPacket.set_cmd(0);
		dataPacket.set_route(userId);
		dataPacket.set_route_type(ROUTE_BALANCE_USERID);
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		::node::DataPacket dpResponse;
		workerService_stub.KickLogged(dataPacket, &dpResponse, &controller, NULL);
		controller.wait();
		pChlMgr->CheckClientChannel(userId, !controller.ok());

		const rpcz::status_code curStatus = controller.get_status();
		if (curStatus == rpcz::status::DEADLINE_EXCEEDED) {
			return SERVER_CALL_DEADLINE;
		} else if (curStatus == rpcz::status::OK) {
			return (eServerError)dpResponse.result();
		}
		return SERVER_FAILURE;
	}

	inline static eServerError InvokeNotification(
		const std::string& inConnect,
		int cmd, uint64_t userId,
		const ::google::protobuf::Message* pInMessage,
		::google::protobuf::Message* pOutMessage,
		const char* file, long line)
	{
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<rpcz::rpc_channel> pChannel(pChlMgr->GetRpczChannel(inConnect));
		if (pChannel.IsInvalid()) {
			OutputError("pChannel.IsInvalid() %s ", inConnect.c_str());
			return SERVER_ERROR_NOTFOUND_CHANNEL;
		}

		::node::WorkerService_Stub workerService_stub(&*pChannel, false);
		::node::DataPacket request;
		request.set_cmd(cmd);
		request.set_route(userId);
		request.set_route_type(ROUTE_BALANCE_USERID);

		if(NULL != pInMessage) {
			SerializeData(request, *pInMessage, file, line);
		}

		::node::DataPacket response;
		rpcz::rpc_controller controller;
		controller.set_deadline_ms(CALL_DEADLINE_MS);
		workerService_stub.HandleNotification(request, &response, &controller, NULL);
		controller.wait();

		if (controller.get_status() == rpcz::status::DEADLINE_EXCEEDED) {
			return SERVER_CALL_DEADLINE;
		}

		if(NULL != pOutMessage) {
			ParseData(*pOutMessage, response, file, line);
		}
		return (eServerError)response.result();
	}

	inline static void SendToPlayer(
		::node::ForwardRequest& dataRequest,
		::node::DataPacket& dataResponse,
		const char* file, long line)
	{
		CChannelManager::AGENT_ID_SET_T agentIds;
		int nSizeAg = dataRequest.agentids_size();
		for (int i = 0; i < nSizeAg; ++i) {
			agentIds.insert(dataRequest.agentids(i));
		}

		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		CChannelManager::CHLKEY_AGTID_MAP_T channelKeys;
		pChlMgr->IteratorAgentChannelKey(channelKeys, &agentIds);

		CChannelManager::CHLKEY_AGTID_MAP_T::iterator it(channelKeys.begin());
		if(channelKeys.end() != it) {
			util::CAutoPointer<CRpczChannel> pRpczChannel(pChlMgr->FindRpczChannel(it->first));
			if (pRpczChannel.IsInvalid()) {
				PrintError("file: %s line: %u @%s pRpczChannel.IsInvalid()", file, line, __FUNCTION__);
				dataResponse.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
				return;
			}
			util::CAutoPointer<rpcz::rpc_channel> pChannel(pRpczChannel->GetChannel());
			if (pChannel.IsInvalid()) {
				PrintError("file: %s line: %u @%s pChannel.IsInvalid()", file, line, __FUNCTION__);
				dataResponse.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
				return;
			}

			dataRequest.clear_agentids();
			CChannelManager::AGENT_ID_SET_T::iterator itAg(it->second.begin());
			for (; it->second.end() != itAg; ++itAg) {
				dataRequest.add_agentids(*itAg);
			}

			::node::WorkerService_Stub workerService_stub(&*pChannel, false);
			rpcz::rpc_controller controller;
			controller.set_deadline_ms(CALL_DEADLINE_MS);
			workerService_stub.SendToPlayer(dataRequest, &dataResponse, &controller, NULL);
			controller.wait();

			const rpcz::status_code curStatus = controller.get_status();
			if (curStatus == rpcz::status::DEADLINE_EXCEEDED) {
				dataResponse.set_result(SERVER_CALL_DEADLINE);
			}
		} else {
			PrintError("file: %s line: %u @%s channelKeys.empty()", file, line, __FUNCTION__);
			dataResponse.set_result(SERVER_ERROR_NOTFOUND_CHANNEL);
		}
	}

	inline static void PostToPlayer(
		::node::ForwardRequest& dataRequest,
		const char* file, long line)
	{
		CChannelManager::AGENT_ID_SET_T agentIds;
		int nSizeAg = dataRequest.agentids_size();
		for (int i = 0; i < nSizeAg; ++i) {
			agentIds.insert(dataRequest.agentids(i));
		}

		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());

		CChannelManager::CHLKEY_AGTID_MAP_T channelKeys;
		pChlMgr->IteratorAgentChannelKey(channelKeys, &agentIds);

		CChannelManager::CHLKEY_AGTID_MAP_T::iterator it(channelKeys.begin());
		if (channelKeys.end() != it) {
			util::CAutoPointer<CRpczChannel> pRpczChannel(pChlMgr->FindRpczChannel(it->first));
			if (pRpczChannel.IsInvalid()) {
				PrintError("file: %s line: %u @%s pRpczChannel.IsInvalid()", file, line, __FUNCTION__);
				return;
			}
			util::CAutoPointer<rpcz::rpc_channel> pChannel(pRpczChannel->GetChannel());
			if (pChannel.IsInvalid()) {
				PrintError("file: %s line: %u @%s pChannel.IsInvalid()", file, line, __FUNCTION__);
				return;
			}

			dataRequest.clear_agentids();
			CChannelManager::AGENT_ID_SET_T::iterator itAg(it->second.begin());
			for (; it->second.end() != itAg; ++itAg) {
				dataRequest.add_agentids(*itAg);
			}

			::node::WorkerService_Stub workerService_stub(&*pChannel, false);
			rpcz::rpc_controller controller;
			controller.set_deadline_ms(CALL_DEADLINE_MS);
			::node::VoidPacket voidPacket;
			workerService_stub.PostToPlayer(dataRequest, &voidPacket, &controller, NULL);
			controller.wait();
			const rpcz::status_code curStatus = controller.get_status();
			if (curStatus == rpcz::status::DEADLINE_EXCEEDED) {
				PrintError("file: %s line: %u @%s curStatus == rpcz::status::DEADLINE_EXCEEDED", file, line, __FUNCTION__);
			}
		} else {
			PrintError("file: %s line: %u @%s channelKeys.empty()", file, line, __FUNCTION__);
		}
	}
};

#endif /* WORKEROPERATE_H */

