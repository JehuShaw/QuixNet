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

#ifndef RPCZ_CLIENT_CONNECTION_H
#define RPCZ_CLIENT_CONNECTION_H

#include "rpcz/common.hpp"

namespace zmq {
class context_t;
}  // namespace zmq

namespace rpcz {

class connection_manager;
class message_vector;

class client_connection {
 public:
  void reply(message_vector* v);

 private:
  client_connection(connection_manager* manager, uint64 socket_id,
                   std::string& sender, std::string& event_id)
      : manager_(manager), socket_id_(socket_id), sender_(sender),
      event_id_(event_id) {}

  connection_manager* manager_;
  uint64 socket_id_;
  const std::string sender_;
  const std::string event_id_;
  friend void worker_thread(connection_manager*, zmq::context_t &, std::string);
};

}  // namespace rpcz

#endif  // RPCZ_CLIENT_CONNECTION_H
