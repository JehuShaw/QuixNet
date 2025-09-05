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
#include "ThreadBase.h"
#include "PoolBase.h"
#include "WeakPointer.h"
#include "ReferObject.h"

namespace node {

	class CRpczMathodArgument : public thd::ThreadBase, public util::PoolBase<CRpczMathodArgument> {
	public:
		CRpczMathodArgument(
			const util::CWeakPointer<rpcz::service>& pService,
			const ::google::protobuf::MethodDescriptor* pDescriptor,
			std::unique_ptr<rpcz::server_channel>& pChannel,
			std::unique_ptr<google::protobuf::Message>& pRequest)

			: m_pService(pService)
			, m_pDescriptor(pDescriptor)
			, m_pChannel(pChannel.release())
			, m_pRequest(pRequest.release())
		{
		}

		virtual bool OnRun() {
			if(m_pService.IsInvalid()) {
				return true;
			}
			m_pService->call_method(
				m_pDescriptor,
				*m_pRequest,
				m_pChannel.release());
			return true;
		}

		virtual void OnShutdown() {}

	private:
		util::CWeakPointer<rpcz::service> m_pService;
		const ::google::protobuf::MethodDescriptor* m_pDescriptor;
		std::unique_ptr<rpcz::server_channel> m_pChannel;
		std::unique_ptr<google::protobuf::Message> m_pRequest;
	};

	class CProtoRpcServiceImpl : public rpcz::rpc_service {
	public:
		// Does not take ownership of the provided service.
		explicit CProtoRpcServiceImpl(rpcz::service& service) 
			: pService_(&service) {
		}

		~CProtoRpcServiceImpl() {
		}

		virtual void dispatch_request(
			const std::string& method,
			const void* payload, size_t payload_len,
			rpcz::server_channel* channel_);

	private:
		util::CReferObject<rpcz::service> pService_;
	};

}  // namespace node

#endif  // PROTO_RPCZ_SERVICE_IMPL_H
