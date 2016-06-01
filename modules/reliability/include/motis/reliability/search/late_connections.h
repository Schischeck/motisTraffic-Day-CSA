#pragma once

#include <string>

#include "motis/module/message.h"

namespace motis {
namespace reliability {
namespace search {
namespace late_connections {
module::msg_ptr search(ReliableRoutingRequest const&,
                       std::string const& hotels_file);
}  // namespace late_connections
}  // namespace search
}  // namespace reliability
}  // namespace motis
