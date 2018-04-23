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

#include "event_id_generator.hpp"

#ifdef WIN32
#include <process.h>  // for getpid()
#define getpid _getpid
#else
#include <unistd.h>  // for getpid()
#endif

namespace rpcz {
namespace detail {

event_id_generator::event_id_generator() {
    state_ = (reinterpret_cast<uint64>(this) << 32) + getpid();
}

}  // namespace detail
}  // namespace rpcz
