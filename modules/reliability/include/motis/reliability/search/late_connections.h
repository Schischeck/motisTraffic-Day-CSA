#pragma once

#include "motis/module/module.h"

namespace motis {
namespace reliability {
struct reliability;
namespace search {
namespace late_connections {
void search(ReliableRoutingRequest const*, reliability&, motis::module::sid,
            motis::module::callback);
}
}
}
}
