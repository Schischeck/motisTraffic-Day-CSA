#pragma once

#include <vector>

#include "motis/module/message.h"

#include "motis/routing/journey.h"

namespace motis {
namespace routing {
struct Stop;

motis::module::msg_ptr journeys_to_message(std::vector<journey> const&);

namespace detail {
std::vector<flatbuffers::Offset<Stop>> convert_stops(
    flatbuffers::FlatBufferBuilder& b, std::vector<journey::stop> const& stops);
std::vector<flatbuffers::Offset<MoveWrapper>> convert_moves(
    flatbuffers::FlatBufferBuilder& b,
    std::vector<journey::transport> const& transports);
std::vector<flatbuffers::Offset<Attribute>> convert_attributes(
    flatbuffers::FlatBufferBuilder& b,
    std::vector<journey::attribute> const& attributes);
}

}  // namespace routing
}  // namespace motis
