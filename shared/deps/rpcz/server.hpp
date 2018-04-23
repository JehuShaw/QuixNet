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

#ifndef RPCZ_SERVER_H
#define RPCZ_SERVER_H

#include <string>
#include <boost/noncopyable.hpp>
#include "rpcz/common.hpp"  // for scoped_ptr

namespace rpcz {
class rpc_service;
class service;
class server_impl;

// A server object maps incoming RPC requests to a provided service interface.
// The service interface methods are executed inside a worker thread.
// Non-thread-safe.
class server : boost::noncopyable {
 public:
  server();
  ~server();

  // Registers an rpc service with this server. All registrations must occur
  // before bind() is called. The name parameter identifies the service for
  // external clients. If you use the first form, the service name from the
  // protocol buffer definition will be used. Does not take ownership of the
  // provided service.
  void register_service(service& service);
  void register_service(service& service, const std::string& name);

  void bind(const std::string& endpoint);

  // TODO: unregister_service()
  // TODO: register_service() after bind()
  // TODO: register_service_factory(), which creates service for each connection.

  // TODO: delete register_rpc_service()
  // Registers a low-level rpc_service. It takes ownership of the rpc_service
  void register_rpc_service(rpc_service* rpc_service, const std::string& name);

 private:
  scoped_ptr<server_impl> impl_;
};  // class server

}  // namespace rpcz

#endif  // RPCZ_SERVER_H
