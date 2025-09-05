#include "client_connection.hpp"

#include <zmq.hpp>

#include "connection_manager.hpp"
#include "internal_commands.hpp"
#include "zmq_utils.hpp"

namespace rpcz {

void client_connection::reply(message_vector* v) {
  zmq::socket_t& socket = manager_->get_frontend_socket();
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_char(&socket, kReply, ZMQ_SNDMORE);
  send_uint64(&socket, socket_id_, ZMQ_SNDMORE);
  send_string(&socket, sender_, ZMQ_SNDMORE);
  send_empty_message(&socket, ZMQ_SNDMORE);
  send_string(&socket, event_id_, ZMQ_SNDMORE);
  write_vector_to_socket(&socket, *v);
}

}  // namespace rpcz
