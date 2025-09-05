

#include "application_options.hpp"

namespace rpcz {

boost::mutex application_options::mutex_;
int application_options::connection_manager_threads_ = 1;
zmq::context_t* application_options::zmq_context_ = NULL;
int application_options::zmq_io_threads_ = 1;


application_options::application_options(void)
{
}


application_options::~application_options(void)
{
}

void application_options::set_zmq_context(zmq::context_t* context)
{
  lock_guard lock(mutex_);
  zmq_context_ = context;
}

zmq::context_t* application_options::get_zmq_context()
{
  lock_guard lock(mutex_);
  return zmq_context_;
}

void application_options::set_zmq_io_threads(int n)
{
  lock_guard lock(mutex_);
  if (n <= 0) return;
  zmq_io_threads_ = n;
}

int application_options::get_zmq_io_threads()
{
  lock_guard lock(mutex_);
  assert(zmq_io_threads_ > 0);
  return zmq_io_threads_;
}

void application_options::set_connection_manager_threads(int n)
{
  lock_guard lock(mutex_);
  if (n <= 0) return;
  connection_manager_threads_ = n;
}

int application_options::get_connection_manager_threads()
{
  lock_guard lock(mutex_);
  assert(connection_manager_threads_ > 0);
  return connection_manager_threads_;
}

}  // namespace rpcz
