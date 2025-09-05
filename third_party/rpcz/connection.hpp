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

#ifndef RPCZ_CONNECTION_H
#define RPCZ_CONNECTION_H

#include "rpcz/common.hpp"
#include "rpcz/connection_manager_ptr.hpp"
#include "client_request_callback.hpp"

namespace rpcz {
class message_vector;

// Represents a connection to a server. Thread-safe.
class connection {
 public:
  connection();

  // Asynchronously sends a request over the connection.
  // request: a vector of messages to be sent. Does not take ownership of the
  //          request. The vector has to live valid at least until the request
  //          completes. It can be safely de-allocated inside the provided
  //          closure or after remote_response->wait() returns.
  // deadline_ms - milliseconds before giving up on this request. -1 means
  //               forever.
  // callback - a closure that will be ran on one of the worker threads when a
  //           response arrives or it timeouts.
  void send_request(
      message_vector& request,
      int64 deadline_ms,
      const client_request_callback & callback);

 private:
  explicit connection(uint64 connection_id);

 private:
  connection_manager_ptr manager_;
  uint64 connection_id_;
  friend class connection_manager;
};

}  // namespace rpcz

#endif  // RPCZ_CONNECTION_H
