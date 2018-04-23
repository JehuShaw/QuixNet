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

#ifndef RPCZ_INTERNAL_COMMANDS_H
#define RPCZ_INTERNAL_COMMANDS_H

namespace rpcz {
namespace {

// Command codes for internal process communication.
//
// Message sent from outside to the broker thread:
const char kRequest = 0x01;      // send request to a connected socket.
const char kConnect = 0x02;      // connect to a given endpoint.
const char kBind    = 0x03;      // bind to an endpoint.
const char kUnbind  = 0x04;      // unbind an endpoint.
const char kReply   = 0x05;      // reply to a request
const char kQuit    = 0x0f;      // Starts the quit second.

// Messages sent from the broker to a worker thread:
const char krunclosure        = 0x11;   // run a closure
const char krunserver_function = 0x12;   // Handle a request (a reply path
                                        // is given)
const char kInvokeclient_request_callback = 0x13;  // run a user supplied
                                                 // function that processes
                                                 // a reply from a remote
                                                 // server.
const char kWorkerQuit = 0x1f;          // Asks the worker to quit.

// Messages sent from a worker thread to the broker:
const char kReady = 0x21;        // Always the first message sent.
const char kWorkerDone = 0x22;   // Sent just before the worker quits.

}  // unnamed namespace
}  // namespace rpcz

#endif  // RPCZ_INTERNAL_COMMANDS_H
