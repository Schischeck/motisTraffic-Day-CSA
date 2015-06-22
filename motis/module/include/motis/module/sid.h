#pragma once

#include <cinttypes>
#include <vector>
#include <functional>

#include "json11/json11.hpp"

namespace motis {
namespace module {

typedef uint64_t sid;
typedef std::function<json11::Json(json11::Json const&, sid)> msg_handler;
typedef std::function<void(sid)> sid_handler;

}  // namespace motis
}  // namespace module
