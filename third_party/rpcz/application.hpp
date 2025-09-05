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

#ifndef RPCZ_APPLICATION_H
#define RPCZ_APPLICATION_H

#include <string>

namespace zmq {
class context_t; 
}  // namespace zmq

namespace rpcz {
class rpc_channel;

// rpcz::application is a simple interface that helps setting up a common
// RPCZ client or server application.
namespace application {

  // Blocks the current thread until another thread calls terminate.
  void run();

  // Releases all the threads that are blocked inside run()
  void terminate();

  // You can change the default options BEFORE any client or server.
  // These options are:
  // * Number of connection manager threads. Those threads are used for
  //   running user code: handling server requests or running callbacks.
  //   Default 1.
  // * ZeroMQ context to use for our application. If NULL, then application will
  //   construct its own ZeroMQ context and own it. If you provide your own
  //   ZeroMQ context, application will not take ownership of it. The ZeroMQ
  //   context must outlive the rpcz application. Default NULL.
  // * Number of ZeroMQ I/O threads, to be passed to zmq_init(). This value is
  //   ignored when you provide your own ZeroMQ context. Default 1.
  void set_connection_manager_threads(int n);  // default 1
  void set_zmq_context(zmq::context_t* context);  // default NULL
  void set_zmq_io_threads(int n);  // default 1
}  // namespace application

}  // namespace rpcz
#endif
