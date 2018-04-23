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
//         Jin Qing (http://blog.csdn.net/jq0123)

#ifndef RPCZ_PRC_SERVICE_H
#define RPCZ_PRC_SERVICE_H

#include <string>

namespace rpcz {

class server_channel;

// rpc_service is a low-level request handler: requests and replies are void*.
// It is exposed here for language bindings. Do not use directly.
class rpc_service {
 public:
  virtual ~rpc_service() {}

  virtual void dispatch_request(const std::string& method,
                               const void* payload, size_t payload_len,
                               server_channel* channel_) = 0;
};

}  // namespace rpcz

#endif  // RPCZ_PRC_SERVICE_H
