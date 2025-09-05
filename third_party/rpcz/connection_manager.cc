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

#include "connection_manager.hpp"

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include <map>
#include <ostream>
#include <sstream>
#include <stddef.h>
#include <string>
#include <utility>
#include <vector>

#include <zmq.hpp>
#include <google/protobuf/stubs/common.h>

#include "application_options.hpp"
#include "client_connection.hpp"
#include "connection.hpp"
#include "broker_thread.hpp"
#include "internal_commands.hpp"
#include "logging.hpp"
#include "rpcz/callback.hpp"
#include "rpcz/sync_event.hpp"
#include "zmq_utils.hpp"

namespace rpcz {

connection_manager::weak_ptr connection_manager::this_weak_ptr_;
boost::mutex connection_manager::this_weak_ptr_mutex_;

void worker_thread(connection_manager* connection_manager,
                  zmq::context_t & context, std::string endpoint) {
  zmq::socket_t socket(context, ZMQ_DEALER);
  socket.connect(endpoint.c_str());
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kReady);
  bool should_quit = false;
  while (!should_quit) {
    message_iterator iter(socket);
    CHECK_EQ(0, iter.next().size());
    char command(interpret_message<char>(iter.next()));
    switch (command) {
      case kWorkerQuit:
        should_quit = true;
        break;
      case krunclosure:
        interpret_message<closure*>(iter.next())->run();
        break;
      case krunserver_function: {
        server_function sf =
            interpret_message<server_function>(iter.next());
        uint64 socket_id = interpret_message<uint64>(iter.next());
        std::string sender(message_to_string(iter.next()));
        if (iter.next().size() != 0) {
          break;
        }
        std::string event_id(message_to_string(iter.next()));
        sf(client_connection(connection_manager, socket_id, sender, event_id),
           iter);
        }
        break;
      case kInvokeclient_request_callback: {
        client_request_callback cb =
            interpret_message<client_request_callback>(
                iter.next());
        connection_manager_status status = connection_manager_status(
            interpret_message<uint64>(iter.next()));
        cb(status, iter);
      }
    }
  }
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kWorkerDone);
}

connection_manager::connection_manager()
  : context_(NULL),
    frontend_endpoint_("inproc://" + 
#pragma warning(push)
#pragma warning(disable : 4355)
	boost::lexical_cast<std::string>(this)
#pragma warning(pop)
        + ".rpcz.connection_manager.frontend"),
    is_terminating_(new sync_event)  // scoped_ptr
{
 /* DLOG(INFO) << "connection_manager() ";*/
  application_options options;
  context_ = options.get_zmq_context();
  if (NULL == context_)
  {
      int zmq_io_threads = options.get_zmq_io_threads();
      assert(zmq_io_threads > 0);
      own_context_.reset(new zmq::context_t(zmq_io_threads));
      context_ = own_context_.get();
  }
  assert(context_);
  zmq::socket_t* frontend_socket = new zmq::socket_t(*context_, ZMQ_ROUTER);
  int linger_ms = 0;
  frontend_socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
  frontend_socket->bind(frontend_endpoint_.c_str());
  int nthreads = options.get_connection_manager_threads();
  assert(nthreads > 0);
  for (int i = 0; i < nthreads; ++i) {
    worker_threads_.add_thread(
        new boost::thread(&worker_thread, this, boost::ref(*context_), frontend_endpoint_));
  }
  sync_event event;
  broker_thread_ = boost::thread(&broker_thread::run,
                                 boost::ref(*context_), nthreads, &event,
                                 frontend_socket);
  event.wait();
}

connection_manager_ptr connection_manager::get()
{
  connection_manager_ptr p = this_weak_ptr_.lock();
  if (p) return p;
  lock_guard lock(this_weak_ptr_mutex_);
  p = this_weak_ptr_.lock();
  if (p) return p;
  p.reset(new connection_manager);
  this_weak_ptr_ = p;
  return p;
}

long connection_manager::use_count()
{
    return this_weak_ptr_.use_count();
}

zmq::socket_t& connection_manager::get_frontend_socket() {
  zmq::socket_t* socket = socket_.get();
  if (socket == NULL) {
    socket = new zmq::socket_t(*context_, ZMQ_DEALER);
    int linger_ms = 0;
    socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
    socket->connect(frontend_endpoint_.c_str());
    socket_.reset(socket);
  }
  return *socket;
}

connection connection_manager::connect(const std::string& endpoint) {
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kConnect, ZMQ_SNDMORE);
  send_string(&socket, endpoint, 0);
  zmq::message_t msg;
  socket.recv(&msg);
  socket.recv(&msg);
  uint64 connection_id = interpret_message<uint64>(msg);
  return connection(connection_id);
}

void connection_manager::bind(const std::string& endpoint,
                             server_function function) {
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kBind, ZMQ_SNDMORE);
  send_string(&socket, endpoint, ZMQ_SNDMORE);
  send_object(&socket, function, 0);
  zmq::message_t msg;
  socket.recv(&msg);
  socket.recv(&msg);
}

// Unbind socket and unregister server_function.
void connection_manager::unbind(const std::string& endpoint)
{
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kUnbind, ZMQ_SNDMORE);
  send_string(&socket, endpoint, 0);
  zmq::message_t msg;
  socket.recv(&msg);
  socket.recv(&msg);
}

void connection_manager::add(closure* closure) {
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, krunclosure, ZMQ_SNDMORE);
  send_pointer(&socket, closure, 0);
  return;
}
 
void connection_manager::run() {
  is_terminating_->wait();
}

void connection_manager::terminate() {
  is_terminating_->signal();
}

connection_manager::~connection_manager() {
  DLOG(INFO) << "~connection_manager()";
  zmq::socket_t& socket = get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kQuit, 0);
  broker_thread_.join();
  worker_threads_.join_all();
  DLOG(INFO) << "All threads joined.";
}

}  // namespace rpcz
