#ifndef RPCZ_APPLICATION_OPTIONS_H
#define RPCZ_APPLICATION_OPTIONS_H

#include <boost/thread.hpp>

namespace zmq {
class context_t; 
}  // namespace zmq

namespace rpcz {

// Thread-safe.
class application_options
{
public:
  application_options(void);
  ~application_options(void);

public:
  static void set_zmq_context(zmq::context_t* context);
  static zmq::context_t* get_zmq_context();
  static void set_zmq_io_threads(int n);
  static int get_zmq_io_threads();
  static void set_connection_manager_threads(int n);
  static int get_connection_manager_threads();

 private:
  static boost::mutex mutex_;
  typedef boost::lock_guard<boost::mutex> lock_guard;

 private:
  // Number of connection manager threads. Those threads are used for
  // running user code: handling server requests or running callbacks.
  static int connection_manager_threads_;  // default 1

  // ZeroMQ context to use for our application. If NULL, then application will
  // construct its own ZeroMQ context and own it. If you provide your own
  // ZeroMQ context, application will not take ownership of it. The ZeroMQ
  // context must outlive the rpcz application.
  static zmq::context_t* zmq_context_;  // default NULL

  // Number of ZeroMQ I/O threads, to be passed to zmq_init(). This value is
  // ignored when you provide your own ZeroMQ context.
  static int zmq_io_threads_;  // default 1

};  // class application_options

}  // namespace rpcz

#endif  // RPCZ_APPLICATION_OPTIONS_H
