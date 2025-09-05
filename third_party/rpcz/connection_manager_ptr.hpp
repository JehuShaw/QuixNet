#ifndef RPCZ_CONNECTION_MANAGER_PTR_H
#define RPCZ_CONNECTION_MANAGER_PTR_H

#include <boost/shared_ptr.hpp>

namespace rpcz {
class connection_manager;
typedef boost::shared_ptr<connection_manager> connection_manager_ptr;
}  // namespace rpcz

#endif  // RPCZ_CONNECTION_MANAGER_PTR_H
