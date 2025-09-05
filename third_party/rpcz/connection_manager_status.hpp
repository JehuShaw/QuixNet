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

#ifndef RPCZ_CONNECTION_MANAGER_STATUS_H
#define RPCZ_CONNECTION_MANAGER_STATUS_H

enum connection_manager_status {
    CMSTATUS_INACTIVE = 0,
    CMSTATUS_ACTIVE = 1,
    CMSTATUS_DONE = 2,
    CMSTATUS_DEADLINE_EXCEEDED = 3,
};

#endif  // RPCZ_CONNECTION_MANAGER_STATUS_H
