#ifndef RPCZ_CLIENT_REQUEST_CALLBACK_H
#define RPCZ_CLIENT_REQUEST_CALLBACK_H

#include <boost/function.hpp>
#include "connection_manager_status.hpp"

namespace rpcz {

class message_iterator;

typedef boost::function<void(connection_manager_status, message_iterator&)>
    client_request_callback;

}  // namespace rpcz

#endif  // RPCZ_CLIENT_REQUEST_CALLBACK_H
