/* 
 * File:   ProtoRpczServiceImpl.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_6_25, 21:20
 */

#ifndef PROTO_RPCZ_SERVICE_IMPL_H
#define PROTO_RPCZ_SERVICE_IMPL_H

#include <string>
#include <google/protobuf/stubs/common.h>
#include "rpc_service.hpp"
#include "service.hpp"
#include "Common.h"
#include "CThreads.h"
#include "CircleQueue.h"

namespace node {

	using google::protobuf::scoped_ptr; 

	struct RpczMathodArgument {
		::google::protobuf::MethodDescriptor* pDescriptor;
		scoped_ptr<rpcz::server_channel> pChannel;
		scoped_ptr<google::protobuf::Message> pRequest;
	};

	class CProtoRpcServiceImpl : public rpcz::rpc_service, public thd::CThread {
	public:
		// Does not take ownership of the provided service.
		explicit CProtoRpcServiceImpl(rpcz::service& service) 
			: service_(service), outgoingMessages(32), bExit(false) {
		}

		~CProtoRpcServiceImpl() {
			ClearArguments();
		}

		virtual void dispatch_request(const std::string& method,
			const void* payload, size_t payload_len,
			rpcz::server_channel* channel_);

		virtual bool Run();

		virtual void OnShutdown();

	private:

		inline void SetArgument(const ::google::protobuf::MethodDescriptor* descriptor,
			scoped_ptr<rpcz::server_channel>& pChannel,
			scoped_ptr<google::protobuf::Message>& pRequest) 
		{
			struct RpczMathodArgument* pArgument = outgoingMessages.WriteLock();
			pArgument->pDescriptor = const_cast<::google::protobuf::MethodDescriptor*>(descriptor);
			pArgument->pChannel.reset(pChannel.release());
			pArgument->pRequest.reset(pRequest.release());
			outgoingMessages.WriteUnlock();
		}

		inline bool GetArgument(struct RpczMathodArgument& outArgument) 
		{
			struct RpczMathodArgument* pArgument = outgoingMessages.ReadLock();
			if(NULL != pArgument) {
				outArgument.pDescriptor = pArgument->pDescriptor;
				outArgument.pChannel.reset(pArgument->pChannel.release());
				outArgument.pRequest.reset(pArgument->pRequest.release());
				outgoingMessages.ReadUnlock();
				return true;
			}
			return false;
		}

		inline void ClearArguments() {
			outgoingMessages.Clear();
		}

	private:
		rpcz::service& service_;
		thd::CCircleQueue<struct RpczMathodArgument> outgoingMessages;

		volatile bool bExit;
	};

}  // namespace node

#endif  // PROTO_RPCZ_SERVICE_IMPL_H
