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

#include "rpcz/rpc_controller.hpp"

#include <boost/lexical_cast.hpp>

#include "logging.hpp"
#include "rpcz/sync_event.hpp"
#include "rpcz/rpcz.pb.h"

namespace rpcz {

rpc_controller::rpc_controller()
    : status_(status::INACTIVE),
      application_error_code_(0),
      deadline_ms_(-1),
      sync_event_(new sync_event()) /* unique_ptr */ {
};

rpc_controller::~rpc_controller() {}

void rpc_controller::set_failed(int application_error, const std::string& error_message) {
  set_status(status::APPLICATION_ERROR);
  error_message_ = error_message;
  application_error_code_ = application_error;
}

void rpc_controller::wait() {
	sync_event_->wait();
}

std::string rpc_controller::to_string() const {
  std::string result =
      "status: " + rpc_response_header_status_code_Name(get_status());
  if (get_status() == status::APPLICATION_ERROR) {
    result += "(" + boost::lexical_cast<std::string>(
            get_application_error_code())
           + ")";
  }
  std::string error_message = get_error_message();
  if (!error_message.empty()) {
    result += ": " + error_message;
  }
  return result;
}
}  // namespace rpcz
