#pragma once

#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/asio/strand.hpp"

#include "conf/configuration.h"

#include "motis/core/schedule/synced_schedule.h"

#include "motis/module/sid.h"
#include "motis/module/message.h"
#include "motis/module/handler_functions.h"

namespace motis {
namespace module {

struct context {
  /// The schedule the module operates on.
  motis::schedule* schedule_;

  /// The I/O io_service:
  /// This service is running on a single thread. Every call that touches
  /// central components like the dispatcher need to be conducted through this
  /// service.
  /// Important: This services should not be blocked by long running I/O or
  /// compute intensive operations. Otherwise, the program will become
  /// unresponsive.
  boost::asio::io_service* ios_;

  /// The module thread pool:
  /// This thread pool runs compute intensive background tasks. Every
  /// module::execute() call should be carried out in this thread pool in
  /// order to not block the I/O thread.
  boost::asio::io_service* thread_pool_;

  /// Sends a message to the client with the specified session id.
  send_fun* send_;

  /// Dispatches a message to other modules.
  msg_handler* dispatch_;
};

struct module : public conf::configuration {
  virtual ~module() {}

  /// \return  a unique module name
  virtual std::string name() const = 0;

  /// \return  a vector containing all message types this module can handle
  virtual std::vector<MsgContent> subscriptions() const = 0;

  /// This function will be called in order to initialize the module. It will be
  /// called exactly once in the process of loading modules.
  ///
  /// Limitation: You cannot assume other modules to be available through the
  /// dispatch function because they may not already be loaded. This could be
  /// solved by adding a dependency graph for the initialization process.
  virtual void init(){};

  /// This function will be called when messages arrive.
  ///
  /// Warranties:
  /// - It will only be called in the thread pool. It is allowed
  ///   to do compute intensive work here.
  /// - This function will not be called in parallel. Member
  ///   variable access does not require extra synchronization.
  /// - The message type of msg will be one of the types
  ///   this module subscribed to. Other message types or empty
  ///   messages should not be handled.
  ///
  /// Implementation obligation:
  /// It is required to call the callback function (cb). In case of an error,
  /// supply the corresponding error type.
  ///
  /// \param msg      the incoming message to handle
  /// \param session  a unique identifier of the client that
  ///                 produced this message
  /// \param cb       the callback to call (calling is obligatory!)
  virtual void on_msg(msg_ptr msg, sid session, callback cb) = 0;

  /// This function will be called if a new client arrives / connects.
  ///
  /// \param session  the session id of the newly connected client
  virtual void on_open(sid /* session */) {}

  /// This function will be called if a new client leaves / disconnects.
  ///
  /// \param session  the session id of the leaving client
  virtual void on_close(sid /* session */) {}

  void init_(context* context) {
    context_ = context;
    strand_ = std::unique_ptr<boost::asio::strand>(
        new boost::asio::strand(*context_->thread_pool_));
    init();
  }

  void on_msg_(msg_ptr msg, sid session, callback cb) {
    namespace p = std::placeholders;
    strand_->post(std::bind(&module::on_msg, this, msg, session, cb));
  }

protected:
  /// \tparam A  the access type: RO or RW
  /// \return  the schedule instance this module is supposed to work on
  template <schedule_access A>
  synced_schedule<A> synced_sched() {
    return synced_schedule<A>(*context_->schedule_);
  }

  /// Dispatch a message to another module. This makes use of the central
  /// dispatcher where every module is registered.
  ///
  /// \param msg      message to dispatch
  /// \param session  session of the user who triggered this request
  /// \param cb       callback function that will be called when the response
  ///                 is available
  void dispatch(msg_ptr msg, sid session, callback cb) {
    using boost::system::error_code;
    // Make sure, the dispatch function gets called in the I/O thread.
    context_->ios_->post([=]() {
      (*context_->dispatch_)(msg, session, [=](msg_ptr res, error_code e) {
        // Make sure the callback gets called in thread pool.
        strand_->post(std::bind(cb, res, e));
      });
    });
  }

  /// Sends a message to the client with the specified session id.
  ///
  /// \param msg      the message to send
  /// \param session  the session id of the user to send to
  void send(msg_ptr msg, sid session) { (*context_->send_)(msg, session); }

private:
  context* context_;

  /// Strand to prevent parallel execution in the thread pool:
  /// The thread pool parallelizes over all modules. Using the thread pool
  /// only through the strand prevents data races in the module sub-classes.
  std::unique_ptr<boost::asio::strand> strand_;
};

}  // namespace motis
}  // namespace module
