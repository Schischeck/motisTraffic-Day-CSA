#pragma once

#include <functional>

#include "boost/system/error_code.hpp"

#include "motis/module/message.h"
#include "motis/module/sid.h"

namespace motis {
namespace module {

typedef std::function<void(msg_ptr, boost::system::error_code)> callback;
typedef std::function<void(msg_ptr, sid, callback, bool)> msg_handler;
typedef std::function<void(msg_ptr, sid)> send_fun;

}  // namespace module
}  // namespace motis
