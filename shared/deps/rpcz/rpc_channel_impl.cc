// Copyright 2011 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: nadavs@google.com <Nadav Samet>

#include "rpc_channel_impl.hpp"

#include <google/protobuf/descriptor.h>
#include <zmq.hpp>

#include "rpcz/callback.hpp"
#include "rpcz/rpc_controller.hpp"
#include "rpcz/sync_event.hpp"
#include "logging.hpp"
#include "zmq_utils.hpp"

namespace rpcz {

rpc_channel_impl::rpc_channel_impl(connection connection)
    : connection_(connection) {
}

rpc_channel_impl::~rpc_channel_impl() {
}

struct rpc_response_context {
  rpc_controller* rpc_ctrller;
  ::google::protobuf::Message* response_msg;
  std::string* response_str;
  closure* user_closure;
};

void rpc_channel_impl::call_method_full(
    const std::string& service_name,
    const std::string& method_name,
    const ::google::protobuf::Message* request_msg,
    const std::string& request,
    ::google::protobuf::Message* response_msg,
    std::string* response_str,
    rpc_controller* rpc_ctrller,
    closure* done) {
  CHECK_EQ(rpc_ctrller->get_status(), status::INACTIVE);
  rpc_request_header generic_request;
  generic_request.set_service(service_name);
  generic_request.set_method(method_name);

  size_t msg_size = generic_request.ByteSize();
  scoped_ptr<zmq::message_t> msg_out(new zmq::message_t(msg_size));
  CHECK(generic_request.SerializeToArray(msg_out->data(), msg_size));

  scoped_ptr<zmq::message_t> payload_out;
  if (request_msg != NULL) {
    size_t bytes = request_msg->ByteSize();
    payload_out.reset(new zmq::message_t(bytes));
    if (!request_msg->SerializeToArray(payload_out->data(), bytes)) {
      throw invalid_message_error("Request serialization failed.");
    }
  } else {
    payload_out.reset(string_to_message(request));
  }

  message_vector msg_vector;
  msg_vector.push_back(msg_out.release());
  msg_vector.push_back(payload_out.release());

  rpc_response_context response_context;
  response_context.rpc_ctrller = rpc_ctrller;
  response_context.user_closure = done;
  response_context.response_str = response_str;
  response_context.response_msg = response_msg;
  rpc_ctrller->set_status(status::ACTIVE);

  connection_.send_request(
      msg_vector,
      rpc_ctrller->get_deadline_ms(),
      bind(&rpc_channel_impl::handle_client_response, this,
           response_context, _1, _2));
}

void rpc_channel_impl::call_method0(const std::string& service_name,
                                const std::string& method_name,
                                const std::string& request,
                                std::string* response,
                                rpc_controller* rpc_ctrller,
                                closure* done) {
  call_method_full(service_name,
                 method_name,
                 NULL,
                 request,
                 NULL,
                 response,
                 rpc_ctrller,
                 done);
}

void rpc_channel_impl::call_method(
    const std::string& service_name,
    const google::protobuf::MethodDescriptor* method,
    const google::protobuf::Message& request,
    google::protobuf::Message* response,
    rpc_controller* rpc_ctrller,
    closure* done) {
  call_method_full(service_name,
                 method->name(),
                 &request,
                 "",
                 response,
                 NULL,
                 rpc_ctrller,
                 done);
}

void rpc_channel_impl::handle_client_response(
    rpc_response_context response_context,
    connection_manager_status status,
    message_iterator& iter) {
  switch (status) {
    case CMSTATUS_DEADLINE_EXCEEDED:
      response_context.rpc_ctrller->set_status(
          status::DEADLINE_EXCEEDED);
      break;
    case CMSTATUS_DONE: {
        if (!iter.has_more()) {
          response_context.rpc_ctrller->set_failed(
              application_error::INVALID_MESSAGE, "");
          break;
        }
        rpc_response_header generic_response;
        zmq::message_t& msg_in = iter.next();
        if (!generic_response.ParseFromArray(msg_in.data(), msg_in.size())) {
          response_context.rpc_ctrller->set_failed(
              application_error::INVALID_MESSAGE, "");
          break;
        }
        if (generic_response.status() != status::OK) {
          response_context.rpc_ctrller->set_failed(
              generic_response.application_error(),
              generic_response.error());
        } else {
          response_context.rpc_ctrller->set_status(status::OK);
          zmq::message_t& payload = iter.next();
          if (response_context.response_msg) {
            if (!response_context.response_msg->ParseFromArray(
                    payload.data(), payload.size())) {
              response_context.rpc_ctrller->set_failed(
                  application_error::INVALID_MESSAGE, "");
              break;
            }
          } else if (response_context.response_str) {
            response_context.response_str->assign(
                static_cast<char*>(
                    payload.data()),
                payload.size());
          }
        }
      }
      break;
    case CMSTATUS_ACTIVE:
    case CMSTATUS_INACTIVE:
    default:
      CHECK(false) << "Unexpected status: "
                   << status;
  }
  // We call signal() before we execute closure since the closure may delete
  // the rpc_controller object (which contains the sync_event).
  response_context.rpc_ctrller->sync_event_->signal();
  if (response_context.user_closure) {
    response_context.user_closure->run();
  }
}
}  // namespace rpcz
