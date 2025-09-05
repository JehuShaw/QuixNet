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

#include "connection.hpp"

#include <zmq.hpp>

#include "clock.hpp"
#include "connection_manager.hpp"
#include "internal_commands.hpp"
#include "remote_response_wrapper.hpp"
#include "zmq_utils.hpp"

namespace rpcz {

connection::connection()
    : manager_(connection_manager::get()),
      connection_id_(0) {
}

connection::connection(uint64 connection_id)
    : manager_(connection_manager::get()),
      connection_id_(connection_id) {
}

void connection::send_request(
    message_vector& request,
    int64 deadline_ms,
    const client_request_callback & callback) {
  remote_response_wrapper wrapper;
  wrapper.start_time = zclock_ms();
  wrapper.deadline_ms = deadline_ms;
  wrapper.callback = callback;

  zmq::socket_t& socket = manager_->get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kRequest, ZMQ_SNDMORE);
  send_uint64(&socket, connection_id_, ZMQ_SNDMORE);
  send_object(&socket, wrapper, ZMQ_SNDMORE);
  write_vector_to_socket(&socket, request);
}

}  // namespace rpcz
