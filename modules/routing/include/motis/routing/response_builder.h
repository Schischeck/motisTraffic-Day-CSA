#pragma once

#include <vector>

#include "motis/module/message.h"

#include "motis/routing/journey.h"

namespace motis {
namespace routing {

motis::module::msg_ptr journeys_to_message(std::vector<journey> const&);

}  // namespace routing
}  // namespace motis
