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

#ifndef RPCZ_SERVER_IMPL_H
#define RPCZ_SERVER_IMPL_H

#include <map>
#include <set>
#include <boost/noncopyable.hpp>
#include "rpcz/connection_manager_ptr.hpp"

namespace rpcz {
class client_connection;
class message_iterator;
class rpc_service;
class server_channel;
class service;

// A server_impl object maps incoming RPC requests to a provided service interface.
// The service interface methods are executed inside a worker thread.
// Non-thread-safe.
class server_impl : boost::noncopyable {
 public:
  server_impl();
  ~server_impl();

  // Registers an rpc service with this server_impl. All registrations must occur
  // before bind() is called. The name parameter identifies the service for
  // external clients. Does not take ownership of the provided service.
  void register_service(service& service, const std::string& name);

  void bind(const std::string& endpoint);

  // TODO: register_service() after bind()

  // TODO: delete register_rpc_service()
  // Registers a low-level rpc_service. Owns rpc_service.
  void register_rpc_service(rpc_service* rpc_service, const std::string& name);

 private:
  void unregister_service(const std::string& name);

 private:
  void handle_request(const client_connection& connection,
                      message_iterator& iter);

  connection_manager_ptr connection_manager_ptr_;
  typedef std::map<std::string, rpcz::rpc_service*> rpc_service_map;
  rpc_service_map service_map_;  // Owns rpc_service. Delete in destructor.
  typedef std::set<std::string> bind_endpoint_set;
  bind_endpoint_set endpoints_;
};

}  // namespace rpcz

#endif  // RPCZ_SERVER_IMPL_H
