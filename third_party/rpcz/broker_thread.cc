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

#include "broker_thread.hpp"

#include "internal_commands.hpp"
#include "logging.hpp"
#include "remote_response_wrapper.hpp"
#include "rpcz/callback.hpp"
#include "rpcz/sync_event.hpp"  // TODO: hide it
#include "zmq_utils.hpp"

namespace rpcz {

broker_thread::broker_thread(
    zmq::context_t & context, int nthreads, sync_event* ready_event,
    zmq::socket_t* frontend_socket)
    : context_(context),
      frontend_socket_(frontend_socket),
      current_worker_(0) {
    wait_for_workers_ready_reply(nthreads);
    ready_event->signal();
    reactor_.add_socket(frontend_socket, new_permanent_callback(
        this, &broker_thread::handle_frontend_socket,
        frontend_socket));
}

void broker_thread::wait_for_workers_ready_reply(int nthreads) {
  for (int i = 0; i < nthreads; ++i) {
      message_iterator iter(*frontend_socket_);
      std::string sender = message_to_string(iter.next());
      assert(!sender.empty());  // zmq id
      CHECK_EQ(0, iter.next().size());
      char command(interpret_message<char>(iter.next()));
      CHECK_EQ(kReady, command) << "Got unexpected command " << (int)command;
      workers_.push_back(sender);
  }
}

void broker_thread::run(zmq::context_t & context,
        int nthreads, sync_event* ready_event,
        zmq::socket_t* frontend_socket) {
    broker_thread cmt(
        context, nthreads, ready_event, frontend_socket);
    cmt.reactor_.loop();
}

void broker_thread::handle_frontend_socket(zmq::socket_t* frontend_socket) {
    message_iterator iter(*frontend_socket);
    std::string sender = message_to_string(iter.next());
    CHECK_EQ(0, iter.next().size());
    char command(interpret_message<char>(iter.next()));
    switch (command) {
      case kQuit:
        // Ask the workers to quit. They'll in turn send kWorkerDone.
        for (size_t i = 0; i < workers_.size(); ++i) {
          send_string(frontend_socket_, workers_[i], ZMQ_SNDMORE);
          send_empty_message(frontend_socket_, ZMQ_SNDMORE);
          send_char(frontend_socket_, kWorkerQuit, 0);
        }
        break;
      case kConnect:
        handle_connect_command(sender, message_to_string(iter.next()));
        break;
      case kBind: {
        std::string endpoint(message_to_string(iter.next()));
        server_function sf(interpret_message<server_function>(iter.next()));
        handle_bind_command(sender, endpoint, sf);
        break;
      }
      case kUnbind: {
        std::string endpoint(message_to_string(iter.next()));
        handle_unbind_command(sender, endpoint);
        break;
      }
      case kRequest:
        send_request(iter);
        break;
      case kReply:
        send_reply(iter);
        break;
      case kReady:
        CHECK(false);
        break;
      case kWorkerDone:
        workers_.erase(std::remove(workers_.begin(), workers_.end(), sender));
        current_worker_ = 0;
        if (workers_.size() == 0) {
          // All workers are gone, time to quit.
          reactor_.set_should_quit();
        }
        break;
      case krunclosure:
        add_closure(interpret_message<closure*>(iter.next()));
        break;
    }
}

void broker_thread::begin_worker_command(char command) {
    send_string(frontend_socket_, workers_[current_worker_], ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, ZMQ_SNDMORE);
    send_char(frontend_socket_, command, ZMQ_SNDMORE);
    ++current_worker_;
    if (current_worker_ == workers_.size()) {
      current_worker_ = 0;
    }
}

void broker_thread::add_closure(closure* closure) {
    begin_worker_command(krunclosure);
    send_pointer(frontend_socket_, closure, 0);
}

void broker_thread::handle_connect_command(
        const std::string& sender, const std::string& endpoint) {
    zmq::socket_t* socket = new zmq::socket_t(context_, ZMQ_DEALER);
    connections_.push_back(socket);
    int linger_ms = 0;
    socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
    socket->connect(endpoint.c_str());
    reactor_.add_socket(socket, new_permanent_callback(
            this, &broker_thread::handle_client_socket,
            socket));

    send_string(frontend_socket_, sender, ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, ZMQ_SNDMORE);
    send_uint64(frontend_socket_, connections_.size() - 1, 0);
}

void broker_thread::handle_bind_command(
      const std::string& sender,
      const std::string& endpoint,
      server_function server_function) {
    zmq::socket_t* socket = new zmq::socket_t(context_, ZMQ_ROUTER);  // delete in reactor
    int linger_ms = 0;
    socket->setsockopt(ZMQ_LINGER, &linger_ms, sizeof(linger_ms));
    socket->bind(endpoint.c_str());  // TODO: catch exception
    uint64 socket_id = server_sockets_.size();
    server_sockets_.push_back(socket);
    bind_map_[endpoint] = socket;  // for unbind
    // reactor will own socket and callback.
    reactor_.add_socket(socket, new_permanent_callback(
        this, &broker_thread::handle_server_socket,
        socket_id, server_function));

    send_string(frontend_socket_, sender, ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, 0);
}

void broker_thread::handle_unbind_command(
      const std::string& sender,
      const std::string& endpoint) {
    endpoint_to_socket::const_iterator it = bind_map_.find(endpoint);
    if (it == bind_map_.end()) return;
    assert((*it).second);
    reactor_.del_socket((*it).second,
        new_callback(this, &broker_thread::handle_socket_deleted, std::string(sender)));
    bind_map_.erase(it);
    // Socket is not delelted yet.
    // It will callback on deleted before next zmq_poll().
}

void broker_thread::handle_socket_deleted(const std::string sender)
{
    send_string(frontend_socket_, sender, ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, ZMQ_SNDMORE);
    send_empty_message(frontend_socket_, 0);
}

void broker_thread::handle_server_socket(uint64 socket_id,
        server_function server_function) {
    message_iterator iter(*server_sockets_[(unsigned int)socket_id]);
    begin_worker_command(krunserver_function);
    send_object(frontend_socket_, server_function, ZMQ_SNDMORE);
    send_uint64(frontend_socket_, socket_id, ZMQ_SNDMORE);
    forward_messages(iter, *frontend_socket_);
}

void broker_thread::send_request(message_iterator& iter) {
    uint64 connection_id = interpret_message<uint64>(iter.next());
    remote_response_wrapper remote_response_wrapper =
        interpret_message<rpcz::remote_response_wrapper>(iter.next());
    event_id event_id = event_id_generator_.get_next();
    remote_response_map_[event_id] = remote_response_wrapper.callback;
    if (remote_response_wrapper.deadline_ms != -1) {
      reactor_.run_closure_at(
          remote_response_wrapper.start_time +
              remote_response_wrapper.deadline_ms,
          new_callback(this, &broker_thread::handle_timeout, event_id));
    }
    zmq::socket_t*& socket = connections_[(unsigned int)connection_id];
    send_string(socket, "", ZMQ_SNDMORE);
    send_uint64(socket, event_id, ZMQ_SNDMORE);
    forward_messages(iter, *socket);
}

void broker_thread::handle_client_socket(zmq::socket_t* socket) {
    message_iterator iter(*socket);
    if (iter.next().size() != 0) {
      return;
    }
    if (!iter.has_more()) {
      return;
    }
    event_id event_id(interpret_message<event_id>(iter.next()));
    remote_response_map::iterator response_iter = remote_response_map_.find(event_id);
    if (response_iter == remote_response_map_.end()) {
      return;
    }
    client_request_callback& callback = response_iter->second;
    begin_worker_command(kInvokeclient_request_callback);
    send_object(frontend_socket_, callback, ZMQ_SNDMORE);
    send_uint64(frontend_socket_, CMSTATUS_DONE, ZMQ_SNDMORE);
    forward_messages(iter, *frontend_socket_);
    remote_response_map_.erase(response_iter);
}

void broker_thread::handle_timeout(event_id event_id) {
    remote_response_map::iterator response_iter = remote_response_map_.find(event_id);
    if (response_iter == remote_response_map_.end()) {
        return;
    }
    client_request_callback& callback = response_iter->second;
	begin_worker_command(kInvokeclient_request_callback);
	send_object(frontend_socket_, callback, ZMQ_SNDMORE);
	send_uint64(frontend_socket_, CMSTATUS_DEADLINE_EXCEEDED, 0);
    remote_response_map_.erase(response_iter);
}

void broker_thread::send_reply(message_iterator& iter) {
    uint64 socket_id = interpret_message<uint64>(iter.next());
    zmq::socket_t* socket = server_sockets_[(unsigned int)socket_id];
    forward_messages(iter, *socket);
}

}  // namespace rpcz
