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

#include "zmq_utils.hpp"

#include <stddef.h>
#include <string.h>

#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/functional/hash.hpp>
#include <google/protobuf/stubs/common.h>
#include <zmq.hpp>

#include "logging.hpp"

namespace rpcz {
std::string message_to_string(zmq::message_t& msg) {
  return std::string((char*)msg.data(), msg.size());
}

zmq::message_t* string_to_message(const std::string& str) {
  zmq::message_t* message = new zmq::message_t(str.length());
  memcpy(message->data(), str.c_str(), str.length());
  return message;
}

bool read_message_to_vector(zmq::socket_t* socket,
                            message_vector* data) {
  while (1) {
    zmq::message_t *msg = new zmq::message_t;
    socket->recv(msg, 0);
    data->push_back(msg);
    if (!msg->more()) {
      break;
    }
  }
  return true;
}

void write_vector_to_socket(zmq::socket_t* socket,
                            message_vector& data,
                            int flags) {
  for (size_t i = 0; i < data.size(); ++i) {
    socket->send(data[i], 
                 flags |
                 ((i < data.size() - 1) ? ZMQ_SNDMORE : 0));
  }
}

bool send_empty_message(zmq::socket_t* socket,
                        int flags) {
  zmq::message_t message(0);
  return socket->send(message, flags);
}

bool send_string(zmq::socket_t* socket,
                 const std::string& str,
                 int flags) {
  zmq::message_t msg(str.size());
  str.copy((char*)msg.data(), str.size(), 0);
  return socket->send(msg, flags);
}

bool send_uint64(zmq::socket_t* socket,
                 google::protobuf::uint64 value,
                 int flags) {
  zmq::message_t msg(8);
  memcpy(msg.data(), &value, 8);
  return socket->send(msg, flags);
}

}  // namespace
