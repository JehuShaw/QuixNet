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

#include "reactor.hpp"

#include "clock.hpp"
#include "logging.hpp"
#include "rpcz/callback.hpp"

namespace rpcz {
namespace {

// Deletes each pointer in the range [begin, end)
template<typename IteratorType>
void delete_container_pointers(const IteratorType& begin,
                             const IteratorType& end) {
  for (IteratorType i = begin; i != end; ++i) {
    delete *i;
  }
}

// For each item in the range [begin, end), delete item->first and item->second.
template<typename IteratorType>
void delete_container_pair_pointers(const IteratorType& begin,
                                 const IteratorType& end) {
  for (IteratorType i = begin; i != end; ++i) {
    delete i->first;
    delete i->second;
  }
}

}  // unnamed namespace

reactor::reactor() : should_quit_(false) {
};

reactor::~reactor() {
  delete_container_pair_pointers(sockets_.begin(), sockets_.end());
  for (closure_run_map::const_iterator it = closure_run_map_.begin();
       it != closure_run_map_.end(); ++it) {
    delete_container_pointers(it->second.begin(), it->second.end());
  }
}

void reactor::add_socket(zmq::socket_t* socket, closure* closure) {
  assert(closure);
  sockets_.push_back(std::make_pair(socket, closure));
  is_dirty_ = true;
}

void reactor::del_socket(zmq::socket_t* socket, closure* callback)
{
  assert(callback);
  del_sockets_.insert(std::make_pair(socket, callback));
  is_dirty_ = true;
  // Close and delete socket in loop().
}

void reactor::rebuild_poll_items() {
  process_del_sockets();
  del_sockets_.clear();  // TODO: callback not deleted.

  pollitems_.resize(sockets_.size());
  for (size_t i = 0; i < sockets_.size(); ++i) {
    zmq::socket_t& socket = *sockets_[i].first;
    zmq::pollitem_t pollitem = {(void*)socket, 0, ZMQ_POLLIN, 0};
    pollitems_[i] = pollitem;
  }
}

void reactor::process_del_sockets()
{
  if (del_sockets_.empty()) return;
  if (sockets_.empty()) return;

  size_t nSize = sockets_.size();
  for (size_t i = nSize - 1; i > 0; i--) {
    typedef socket_to_closure::const_iterator const_iterator;
    typedef std::pair<const_iterator, const_iterator> range;
    zmq::socket_t* socket = sockets_[i].first;
    range r = del_sockets_.equal_range(socket);
    if (r.first == r.second)
      continue;

    delete socket;  // will close it
    delete sockets_[i].second;  // delele callback
    sockets_[i] = sockets_[--nSize];  // fill with the last
    sockets_.pop_back();
    assert(nSize == sockets_.size());

    for (const_iterator it = r.first; it != r.second; ++it)
      (*it).second->run();
    del_sockets_.erase(r.first, r.second);
    if (del_sockets_.empty())
      return;
  }
}

void reactor::run_closure_at(uint64 timestamp, closure* closure) {
  closure_run_map_[timestamp].push_back(closure);
}

int reactor::loop() {
  while (!should_quit_) {

    if (is_dirty_) {
      rebuild_poll_items();
      is_dirty_ = false;
    }

    long poll_timeout = process_closure_run_map();
    int rc = zmq_poll(&pollitems_[0], pollitems_.size(), poll_timeout);

    if (rc == -1) {
      int zmq_err = zmq_errno();
      CHECK_NE(zmq_err, EFAULT);
      if (zmq_err == ETERM) {
        return -1;
      }
    }

    for (size_t i = 0; i < pollitems_.size(); ++i) {
      if (pollitems_[i].revents & ZMQ_POLLIN) {
        // TODO: process all events after each zmq_poll() call.
        sockets_[i].second->run();
      }  // if
    }  // for
  }  // while (!should_quit_)
  return 0;
}

long reactor::process_closure_run_map() {

  if(closure_run_map_.empty()) {
	return -1;
  }

  uint64 now = zclock_ms();
  closure_run_map::iterator ub(closure_run_map_.upper_bound(now));
  for (closure_run_map::const_iterator it = closure_run_map_.begin();
       it != ub;
       ++it) {
    for (std::vector<closure*>::const_iterator vit = it->second.begin();
         vit != it->second.end(); ++vit) {
      (*vit)->run();
    }
  }

  long poll_timeout = -1;
  if (ub != closure_run_map_.end()) {
#if ZMQ_VERSION_MAJOR > 2
    poll_timeout = (long)(ub->first - now);
#else
	poll_timeout = (long)(1000 * (ub->first - now));
#endif
	if(poll_timeout < 0) {
		poll_timeout = 0;
	}
  }
  closure_run_map_.erase(closure_run_map_.begin(), ub);
  return poll_timeout;
}

void reactor::set_should_quit() {
  should_quit_ = true;
}

}  // namespace rpcz
