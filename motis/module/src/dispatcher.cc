#include "motis/module/dispatcher.h"

#include "motis/module/error.h"

namespace motis {
namespace module {

dispatcher::dispatcher(server& server, boost::asio::io_service& ios)
    : server_(server), ios_(ios) {
  namespace p = std::placeholders;
  server_.on_msg(std::bind(&dispatcher::on_msg, this, p::_1, p::_2, p::_3));
  server_.on_open(std::bind(&dispatcher::on_open, this, p::_1));
  server_.on_close(std::bind(&dispatcher::on_close, this, p::_1));
}

void dispatcher::send(msg_ptr msg, sid session) { server_.send(msg, session); }

void dispatcher::on_msg(msg_ptr msg, sid session, callback cb) {
  auto module_it = subscriptions_.find(msg->msg_->content_type());
  if (module_it == end(subscriptions_)) {
    return cb({}, error::no_module_capable_of_handling);
  }
  if (module_it->second.empty()) {
    held_back_msgs_.push_back({msg, session, cb});
    return;
  } else {
    auto processor = module_it->second.back();
    remove_module(processor);

    namespace p = std::placeholders;
    auto wrapped_cb = std::bind(&dispatcher::on_msg_finish, this, processor, cb,
                                p::_1, p::_2);
    return processor->on_msg_(msg, session, ios_.wrap(wrapped_cb));
  }
}

void dispatcher::on_open(sid session) {
  for (auto const& module : modules_) {
    module->on_open(session);
  }
}

void dispatcher::on_close(sid session) {
  for (auto const& module : modules_) {
    module->on_close(session);
  }
}

void dispatcher::remove_module(module* m) {
  for (auto& sub : subscriptions_) {
    for (auto it = begin(sub.second); it != end(sub.second);) {
      if (*it == m) {
        it = sub.second.erase(it);
      } else {
        ++it;
      }
    }
  }
}

void dispatcher::add_module(module* m) {
  for (auto const& sub : m->subscriptions()) {
    subscriptions_[sub].push_back(m);
  }
}

void dispatcher::on_msg_finish(module* m, callback cb, msg_ptr res,
                               boost::system::error_code ec) {
  add_module(m);
  reschedule_held_back_msgs();
  cb(res, ec);
}

void dispatcher::reschedule_held_back_msgs() {
  auto held_back_msgs_copy = held_back_msgs_;
  held_back_msgs_.clear();
  for (auto const& m : held_back_msgs_copy) {
    on_msg(m.msg, m.session, m.cb);
  }
}

}  // namespace module
}  // namespace motis
