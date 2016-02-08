#pragma once

#include <map>

#include "boost/asio/io_service.hpp"

#include "motis/module/module.h"
#include "motis/module/receiver.h"
#include "motis/module/callbacks.h"

namespace motis {
namespace module {

struct dispatcher : public receiver {
  struct held_back_msg {
    msg_ptr msg;
    sid session;
    callback cb;
    bool locked;
  };

  dispatcher(boost::asio::io_service* ios);

  dispatcher(dispatcher const&) = delete;
  dispatcher(dispatcher&&) = delete;

  dispatcher& operator=(dispatcher const&) = delete;
  dispatcher& operator=(dispatcher&&) = delete;

  void set_io_service(boost::asio::io_service*);
  void set_send_fun(send_fun);
  void send(msg_ptr msg, sid session);

  void on_msg(msg_ptr msg, sid session, callback cb, bool locked) override;
  void on_open(sid session) override;
  void on_close(sid session) override;

  void remove_module(module* m);
  void add_module(module* m);

  void on_msg_finish(module* m, callback cb, msg_ptr res,
                     boost::system::error_code ec);
  void reschedule_held_back_msgs();

  boost::asio::io_service* ios_;
  send_fun send_fun_;
  std::vector<module*> modules_;
  std::map<MsgContent, std::vector<module*>> subscriptions_;
  std::vector<held_back_msg> held_back_msgs_;
};

}  // namespace module
}  // namespace motis
