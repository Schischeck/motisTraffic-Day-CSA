#pragma once

#include <vector>

#include "motis/module/message.h"

#include "motis/core/journey/journey.h"

namespace motis {

motis::module::msg_ptr journeys_to_message(std::vector<journey> const&);

flatbuffers::Offset<routing::Connection> to_connection(
    flatbuffers::FlatBufferBuilder&, journey const&);

}  // namespace motis
