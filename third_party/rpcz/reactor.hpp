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

#ifndef RPCZ_REACTOR_H
#define RPCZ_REACTOR_H

#include <map>
#include <set>
#include <vector>
#include <boost/noncopyable.hpp>
#include <zmq.hpp>
#include "rpcz/common.hpp"

namespace rpcz {

class closure;

// Non-thread-safe. Only used in broker_thread.
class reactor : boost::noncopyable {
 public:
  reactor();
  ~reactor();

  // TODO: use boost::function. Callback ownship sucks.

  // Will own socket and callback.
  void add_socket(zmq::socket_t* socket, closure* callback);
  // Delete socket and callback on deleted. Will not own callback.
  void del_socket(zmq::socket_t* socket, closure* callback);

  // Will not own callback.
  void run_closure_at(uint64 timestamp, closure *callback);

  int loop();

  void set_should_quit();

 private:
  long process_closure_run_map();
  void rebuild_poll_items();
  void process_del_sockets();

 private:
  bool should_quit_;
  bool is_dirty_;
  std::vector<std::pair<zmq::socket_t*, closure*> > sockets_;
  typedef std::multimap<zmq::socket_t*, closure*> socket_to_closure;
  socket_to_closure del_sockets_;
  std::vector<zmq::pollitem_t> pollitems_;
  typedef std::map<uint64, std::vector<closure*> > closure_run_map;
  closure_run_map closure_run_map_;
};
}  // namespace
#endif
