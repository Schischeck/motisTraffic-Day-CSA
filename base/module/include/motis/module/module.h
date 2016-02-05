#pragma once

#include <string>
#include <vector>

#include "boost/asio/io_service.hpp"
#include "boost/asio/strand.hpp"

#include "conf/configuration.h"

#include "motis/core/common/raii.h"
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

struct locked_schedule {
  locked_schedule(std::shared_ptr<synced_schedule> lock) : lock_(lock) {}
  schedule& sched() { return lock_->sched(); }
  std::shared_ptr<synced_schedule> lock_;
};

struct module : public conf::configuration {
  module() : context_(nullptr), locked_(false) {}
  virtual ~module() {}

  virtual std::string name() const = 0;

  virtual std::vector<MsgContent> subscriptions() const = 0;
  virtual void init(){};
  virtual void init_async(callback cb) {
    return cb({}, boost::system::error_code());
  };
  virtual void on_msg(msg_ptr msg, sid session, callback cb) = 0;
  virtual void on_open(sid /* session */) {}
  virtual void on_close(sid /* session */) {}

  void init_(context* context) {
    context_ = context;
    strand_ = std::unique_ptr<boost::asio::strand>(
        new boost::asio::strand(*context_->thread_pool_));
    init();
  }

  void on_msg_(msg_ptr msg, sid session, callback cb, bool locked) {
    namespace p = std::placeholders;
    strand_->post([this, locked, msg, session, cb]() {
      struct resetter {
        resetter(module& instance, bool locked) : instance_(instance) {
          previous_ = instance_.locked_.load();
          instance_.locked_.store(previous_ || locked);
        }

        ~resetter() { instance_.locked_.store(previous_); }

        module& instance_;
        bool previous_;
      } r(*this, locked);
      on_msg(msg, session, cb);
    });
  }

protected:
  template <schedule_access access>
  locked_schedule synced_sched() {
    if (locked_.load() && access == RO) {
      return std::make_shared<synced_schedule>(*context_->schedule_, RE,
                                               []() {});
    } else {
      bool prev_schedule_locked = locked_;
      return std::make_shared<synced_schedule>(
          *context_->schedule_, access,
          [this, prev_schedule_locked]() { locked_ = prev_schedule_locked; });
    }
  }

  void dispatch(msg_ptr msg, sid session, callback cb) {
    using boost::system::error_code;
    // Make sure, the dispatch function gets called in the I/O thread.
    context_->ios_->post([=]() {
      (*context_->dispatch_)(msg, session, [=](msg_ptr res, error_code e) {
        // Make sure the callback gets called in thread pool.
        strand_->post(std::bind(cb, res, e));
      }, locked_);
    });
  }

  void dispatch(msg_ptr msg, sid session = 0) {
    using boost::system::error_code;
    // Make sure, the dispatch function gets called in the I/O thread.
    context_->ios_->post([=]() {
      (*context_->dispatch_)(msg, session, [](msg_ptr, error_code) {}, locked_);
    });
  }

  void send(msg_ptr msg, sid session) { (*context_->send_)(msg, session); }

  boost::asio::io_service& get_thread_pool() { return *context_->thread_pool_; }

private:
  context* context_;
  std::atomic<bool> locked_;
  std::unique_ptr<boost::asio::strand> strand_;
};

}  // namespace motis
}  // namespace module
