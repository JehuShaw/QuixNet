#include "rpcz/rpc_channel.hpp"

#include "connection_manager.hpp"
#include "rpc_channel_impl.hpp"

namespace rpcz {

rpc_channel* rpc_channel::create(connection connect) {
  return new rpc_channel_impl(connect);
}

rpc_channel* rpc_channel::create(const std::string& endpoint) {
  return create(connection_manager::get()->connect(endpoint));
}

}  // namespace rpcz
