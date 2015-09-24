#pragma once

#include <map>

#include "motis/module/module.h"

#include "motis/module/server.h"

namespace motis {
namespace module {

struct dispatcher {
  struct held_back_msg {
    msg_ptr msg;
    sid session;
    callback cb;
  };

  dispatcher(server& server, boost::asio::io_service& ios);

  void send(msg_ptr msg, sid session);

  void on_msg(msg_ptr msg, sid session, callback cb);
  void on_open(sid session);
  void on_close(sid session);

  void remove_module(module* m);
  void add_module(module* m);

  void on_msg_finish(module* m, callback cb, msg_ptr res,
                     boost::system::error_code ec);
  void reschedule_held_back_msgs();

  server& server_;
  boost::asio::io_service& ios_;
  std::vector<module*> modules_;
  std::map<MsgContent, std::vector<module*>> subscriptions_;
  std::vector<held_back_msg> held_back_msgs_;
};

}  // namespace module
}  // namespace motis
