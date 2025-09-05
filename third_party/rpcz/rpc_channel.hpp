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

#ifndef RPCZ_RPC_CHANNEL_H
#define RPCZ_RPC_CHANNEL_H

#include <string>
#include <set>

#include <google/protobuf/stubs/common.h>

namespace google {
namespace protobuf {
class Message;
class MethodDescriptor;
}  // namespace protobuf
}  // namespace google

namespace rpcz {
class closure;
class connection;
class rpc_controller;

class rpc_channel {
 public:
  virtual void call_method(const std::string& service_name,
                          const google::protobuf::MethodDescriptor* method,
                          const google::protobuf::Message& request,
                          google::protobuf::Message* response,
                          rpc_controller* rpc_ctrller,
                          closure* done) = 0;

  // DO NOT USE: this method exists only for language bindings and may be
  // removed.
  virtual void call_method0(const std::string& service_name,
                           const std::string& method_name,
                           const std::string& request,
                           std::string* response,
                           rpc_controller* rpc_ctrller,
                           closure* done) = 0;

  static rpc_channel* create(connection connect);

  // Creates an rpc_channel to the given endpoint. Attach it to a Stub and you
  // can start making calls through this channel from any thread. No locking
  // needed. It is your responsibility to delete this object.
  static rpc_channel* create(const std::string& endpoint);

  virtual ~rpc_channel() {}
};  // class rpc_channel

}  // namespace rpcz

#endif  // RPCZ_RPC_CHANNEL_H
