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

#include "rpcz/server.hpp"

#include <google/protobuf/descriptor.h>

#include "rpcz/service.hpp"
#include "server_impl.hpp"

namespace rpcz {

server::server()
  : impl_(new server_impl) /* scoped_ptr */ {
}

server::~server() {
}

void server::register_service(rpcz::service& service) {
  impl_->register_service(service, service.GetDescriptor()->name());
}

void server::register_service(rpcz::service& service, const std::string& name) {
  impl_->register_service(service, name);
}

void server::register_rpc_service(rpcz::rpc_service* rpc_service,
                                  const std::string& name) {
  impl_->register_rpc_service(rpc_service, name);
}

void server::bind(const std::string& endpoint) {
  impl_->bind(endpoint);
}

}  // namespace rpcz
