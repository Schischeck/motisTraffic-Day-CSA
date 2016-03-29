#pragma once

#include "motis/module/ctx_data.h"
#include "motis/module/message.h"

namespace motis {
namespace module {

using future = ctx::future<ctx_data, msg_ptr>;

}  // namespace module
}  // namespace motis
