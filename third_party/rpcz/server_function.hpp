#ifndef RPCZ_SERVER_FUNCTION_H
#define RPCZ_SERVER_FUNCTION_H

#include <boost/function.hpp>

namespace rpcz {

class client_connection;
class message_iterator;

typedef boost::function<void(const client_connection&, message_iterator&)>
    server_function;

}  // namespace rpcz

#endif  // RPCZ_CLIENT_REQUEST_CALLBACK_H
