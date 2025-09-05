#include "ProtoRpczServiceImpl.h"


#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "logging.hpp"
#include "rpc_controller.hpp"
#include "data_packet.pb.h"

#include "ThreadPool.h"

using namespace rpcz;
using namespace thd;

namespace node {

void CProtoRpcServiceImpl::dispatch_request(const std::string& method,
                               const void* payload, size_t payload_len,
                               server_channel* channel_) 
{
	std::unique_ptr<server_channel> pChannel(channel_);

    const ::google::protobuf::MethodDescriptor* descriptor =
		pService_->GetDescriptor()->FindMethodByName(
            method);
    if (descriptor == NULL) {
      // Invalid method name
      DLOG(INFO) << "Invalid method name: " << method,
      pChannel->send_error(application_error::NO_SUCH_METHOD);
      return;
    }
	std::unique_ptr<google::protobuf::Message> pRequest;
    pRequest.reset(CHECK_NOTNULL(
		pService_->GetRequestPrototype(descriptor).New()));
    if (!pRequest->ParseFromArray(payload, payload_len)) {
      DLOG(INFO) << "Failed to parse request.";
      // Invalid proto;
      pChannel->send_error(application_error::INVALID_MESSAGE);
      return;
    }

	// The CRpczMathodArgument delete in ThreadPool;
	CRpczMathodArgument* pArg = new CRpczMathodArgument(
		pService_(), descriptor, pChannel, pRequest);
	ThreadPool.ExecuteTask(pArg);
}

}  // namespace
