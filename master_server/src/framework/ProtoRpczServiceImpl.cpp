#include "ProtoRpczServiceImpl.h"


#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <zmq.hpp>

#include "logging.hpp"
#include "rpc_controller.hpp"

#include "ThreadPool.h"

using namespace rpcz;
using namespace thd;

namespace node {

void CProtoRpcServiceImpl::dispatch_request(const std::string& method,
                               const void* payload, size_t payload_len,
                               server_channel* channel_) 
{
	scoped_ptr<server_channel> pChannel(channel_);
	if(bExit) {
		return;
	}
    const ::google::protobuf::MethodDescriptor* descriptor =
        service_.GetDescriptor()->FindMethodByName(
            method);
    if (descriptor == NULL) {
      // Invalid method name
      DLOG(INFO) << "Invalid method name: " << method,
      pChannel->send_error(application_error::NO_SUCH_METHOD);
      return;
    }
	scoped_ptr<google::protobuf::Message> pRequest;
    pRequest.reset(CHECK_NOTNULL(
            service_.GetRequestPrototype(descriptor).New()));
    if (!pRequest->ParseFromArray(payload, payload_len)) {
      DLOG(INFO) << "Failed to parse request.";
      // Invalid proto;
      pChannel->send_error(application_error::INVALID_MESSAGE);
      return;
    }

	SetArgument(descriptor, pChannel, pRequest);

	ThreadPool.ExecuteTask(this);
}

void CProtoRpcServiceImpl::OnShutdown() {
	atomic_xchg8(&bExit, true);
}

bool CProtoRpcServiceImpl::Run() {

	int nSize = outgoingMessages.Size();
	for(int i = 0; i < nSize && !bExit; ++i) {
		struct RpczMathodArgument argument;
		if(GetArgument(argument)) {
			
			service_.call_method(argument.pDescriptor,
				*argument.pRequest,
				argument.pChannel.release());

			continue;
		}
		break;
	}
	return false;
}

}  // namespace
