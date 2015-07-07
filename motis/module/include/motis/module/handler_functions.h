#pragma once

#include <functional>

#include "json11/json11.hpp"

#include "motis/module/sid.h"

namespace motis {
namespace module {

typedef std::function<json11::Json(json11::Json const&, sid)> msg_handler;
typedef std::function<void(sid)> sid_handler;
typedef std::function<void(json11::Json const&, sid)> send_fun;

}  // namespace module
}  // namespace motis
