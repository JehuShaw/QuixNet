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

#include "rpcz/application.hpp"

#include <string>
#include <zmq.hpp>

#include "application_options.hpp"
#include "connection.hpp"
#include "connection_manager.hpp"
#include "rpcz/rpc_channel.hpp"
#include "rpcz/server.hpp"

namespace rpcz {
namespace application {

void run() {
  connection_manager::get()->run();
}

void terminate() {
  connection_manager::get()->terminate();
}

void set_zmq_context(zmq::context_t* context)
{
  application_options::set_zmq_context(context);
}

void set_zmq_io_threads(int n)
{
  application_options::set_zmq_io_threads(n);
}

void set_connection_manager_threads(int n)
{
  application_options::set_connection_manager_threads(n);
}

}  // namespace application
}  // namespace rpcz
