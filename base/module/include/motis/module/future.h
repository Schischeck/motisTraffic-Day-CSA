#pragma once

#include <memory>

#include "motis/module/ctx_data.h"
#include "motis/module/message.h"

namespace motis {
namespace module {

using future = std::shared_ptr<ctx::future<ctx_data, msg_ptr>>;

}  // namespace module
}  // namespace motis
