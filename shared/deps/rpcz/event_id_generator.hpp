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

#ifndef RPCZ_EVENT_ID_GENERATOR_H
#define RPCZ_EVENT_ID_GENERATOR_H

#include <boost/noncopyable.hpp>
#include "rpcz/common.hpp"

namespace rpcz {

typedef uint64 event_id;

namespace detail {

const uint64 kLargePrime = (1ULL << 63) - 165;
const uint64 kGenerator = 2;

class event_id_generator : boost::noncopyable {
 public:
  event_id_generator();

  event_id get_next() {
    state_ = (state_ * kGenerator) % kLargePrime;
    return state_;
  }

 private:
  uint64 state_;
};  // class event_id_generator

}  // namespace detail
}  // namespace rpcz

#endif  // RPCZ_EVENT_ID_GENERATOR_H
