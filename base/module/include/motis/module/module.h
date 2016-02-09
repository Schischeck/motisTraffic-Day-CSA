#pragma once

#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/asio/strand.hpp"

#include "conf/configuration.h"

#include "motis/core/schedule/synced_schedule.h"

#include "motis/module/sid.h"
#include "motis/module/message.h"
#include "motis/module/callbacks.h"

namespace motis {
namespace module {

struct context {
  motis::schedule* schedule_;
  boost::asio::io_service* ios_;
  boost::asio::io_service* thread_pool_;
  send_fun* send_;
  msg_handler* dispatch_;
};

struct module : public conf::configuration {
  virtual ~module() {}

  virtual std::string name() const = 0;

  virtual std::vector<MsgContent> subscriptions() const = 0;
  virtual void on_msg(msg_ptr msg, sid session, callback cb) = 0;
  virtual void on_open(sid /* session */) {}
  virtual void on_close(sid /* session */) {}

  virtual void init_async() {}
  virtual void init() {}

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
  template <schedule_access A>
  synced_schedule<A> synced_sched() {
    return synced_schedule<A>(*context_->schedule_);
  }

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

  void dispatch(msg_ptr msg, sid session = 0) {
    using boost::system::error_code;
    // Make sure, the dispatch function gets called in the I/O thread.
    context_->ios_->post([=]() {
      (*context_->dispatch_)(msg, session, [](msg_ptr, error_code) {});
    });
  }

  void send(msg_ptr msg, sid session) { (*context_->send_)(msg, session); }

  boost::asio::io_service& get_thread_pool() { return *context_->thread_pool_; }

private:
  context* context_;
  std::unique_ptr<boost::asio::strand> strand_;
};

}  // namespace motis
}  // namespace module
