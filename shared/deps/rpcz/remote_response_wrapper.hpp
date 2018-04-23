#ifndef RPCZ_REMOTE_RESPONSE_WRAPPER_H
#define RPCZ_REMOTE_RESPONSE_WRAPPER_H

#include "rpcz/common.hpp"
#include "client_request_callback.hpp"

namespace rpcz {

struct remote_response_wrapper {
  int64 deadline_ms;
  uint64 start_time;
  client_request_callback callback;
};

}  // namespace rpcz

#endif  // RPCZ_REMOTE_RESPONSE_WRAPPER_H
