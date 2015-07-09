#pragma once

#include <functional>

#include "motis/module/message.h"
#include "motis/module/sid.h"

namespace motis {
namespace module {

typedef std::function<msg_ptr(msg_ptr const&, sid)> msg_handler;
typedef std::function<void(sid)> sid_handler;
typedef std::function<void(msg_ptr const&, sid)> send_fun;
typedef std::function<msg_ptr(msg_ptr const&, sid)> dispatch_fun;

}  // namespace module
}  // namespace motis
