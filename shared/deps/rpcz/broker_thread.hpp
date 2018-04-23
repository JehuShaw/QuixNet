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

#ifndef RPCZ_BROKER_THREAD_H
#define RPCZ_BROKER_THREAD_H

#include "client_request_callback.hpp"
#include "event_id_generator.hpp"
#include "reactor.hpp"
#include "server_function.hpp"

namespace rpcz {

class sync_event;

class broker_thread {
 public:
  broker_thread(
      zmq::context_t & context,
      int nthreads,
      sync_event* ready_event,
      zmq::socket_t* frontend_socket);

 public:
  static void run(zmq::context_t & context,
                  int nthreads,
                  sync_event* ready_event,
                  zmq::socket_t* frontend_socket);

 private:
  void wait_for_workers_ready_reply(int nthreads);

  void handle_frontend_socket(zmq::socket_t* frontend_socket);

  inline void begin_worker_command(char command);

  inline void add_closure(closure* closure);

  void handle_connect_command(const std::string& sender,
                                   const std::string& endpoint);

  void handle_bind_command(
      const std::string& sender,
      const std::string& endpoint,
      server_function server_function);

  void handle_unbind_command(
      const std::string& sender,
      const std::string& endpoint);

  // Callback on reactor deleted socket.
  void handle_socket_deleted(const std::string sender);

  void handle_server_socket(uint64 socket_id,
      server_function server_function);

  inline void send_request(message_iterator& iter);

  void handle_client_socket(zmq::socket_t* socket);

  void handle_timeout(event_id event_id);

  inline void send_reply(message_iterator& iter);

 private:
  typedef std::map<event_id, client_request_callback>
      remote_response_map;
  typedef std::map<uint64, event_id> deadline_map;
  remote_response_map remote_response_map_;
  deadline_map deadline_map_;
  detail::event_id_generator event_id_generator_;
  reactor reactor_;
  std::vector<zmq::socket_t*> connections_;
  std::vector<zmq::socket_t*> server_sockets_;
  typedef std::map<std::string, zmq::socket_t*> endpoint_to_socket;
  endpoint_to_socket bind_map_;  // for unbind
  zmq::context_t & context_;
  zmq::socket_t* frontend_socket_;
  std::vector<std::string> workers_;
  int current_worker_;
};

}  // namespace rpcz

#endif  // RPCZ_BROKER_THREAD_H
