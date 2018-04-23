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

#ifndef RPCZ_CONNECTION_MANAGER_H
#define RPCZ_CONNECTION_MANAGER_H

#include <string>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/weak_ptr.hpp>
#include "connection_manager_status.hpp"
#include "rpcz/common.hpp"
#include "rpcz/connection_manager_ptr.hpp"
#include "server_function.hpp"

namespace zmq {
class context_t;
class message_t;
class socket_t;
}  // namespace zmq

namespace rpcz {
class client_connection;
class closure;
class connection;
class connection_thread_context;
class message_iterator;
class sync_event;

// TODO: Update it.
// A connection_manager is a multi-threaded asynchronous system for communication
// over ZeroMQ sockets. A connection_manager can:
//   1. connect to a remote server and allow all threads the program to share
//      the connection:
//
//          connection_manager cm(10);
//          connection c = cm.connect("tcp://localhost:5557");
// 
//      Now, it is possible to send requests to this backend fron any thread:
//
//          c.send_request(...);
//
//  2. bind a socket and register a handler function. The handler function
//     gets executed in one of the connection manager threads.
//
//          c.bind("tcp://*:5555", &handler_function);
//
//  3. Queue closures to be executed on one of the connection manager threads:
//
//          c.add(closure)
//
// connection_manager and connection are thread-safe.
// connection_manager is singleton.
class connection_manager : boost::noncopyable {

 private:
  connection_manager();

 public:
  // Dynamic singleton. Auto destruct.
  static connection_manager_ptr get();
  static long use_count();

 public:
  // Blocks the current thread until all connection managers have completed.
  ~connection_manager();

  // connects all connection_manager threads to the given endpoint. On success
  // this method returns a connection object that can be used from any thread
  // to communicate with this endpoint.
  connection connect(const std::string& endpoint);

  // binds a socket to the given endpoint and registers server_function as a
  // handler for requests to this socket. The function gets executed on one of
  // the worker threads. When the function returns, the endpoint is already
  // bound.
  void bind(const std::string& endpoint, server_function function);
  // Unbind socket and unregister server_function.
  void unbind(const std::string& endpoint);

  // Executes the closure on one of the worker threads.
  void add(closure* closure);

  // Blocks this thread until terminate() is called from another thread.
  void run();

  // Releases all the threads that are blocked inside run()
  void terminate();

 private:
  typedef boost::weak_ptr<connection_manager> weak_ptr;
  typedef boost::lock_guard<boost::mutex> lock_guard;
  static weak_ptr this_weak_ptr_;
  static boost::mutex this_weak_ptr_mutex_;

 private:
  scoped_ptr<zmq::context_t> own_context_;
  zmq::context_t* context_;  // Use own_context_ or external context

  zmq::socket_t& get_frontend_socket();

  boost::thread broker_thread_;
  boost::thread_group worker_threads_;
  boost::thread_specific_ptr<zmq::socket_t> socket_;
  std::string frontend_endpoint_;
  scoped_ptr<sync_event> is_terminating_;

  friend class connection;
  friend class client_connection;
  friend class connection_managerThread;
};  // class connection_manager

}  // namespace rpcz

#endif
