#pragma once

#include "motis/core/access/station_access.h"
#include "motis/module/message.h"
#include "motis/protocol/Message_generated.h"
#include "parser/file.h"

#include <vector>

namespace motis {
namespace routes {
flatbuffers::Offset<RoutesSectionRes> railviz_railroad_sec_with_data(
    station* const& departure, station* const& arrival, int& clasz,
    motis::module::message_creator& b, parser::buffer& buf_);

flatbuffers::Offset<RoutesSectionRes> railviz_railroad_sec_without_data(
    station* const& departure, station* const& arrival, int& clasz,
    motis::module::message_creator& b);
}  // namespace routes
}  // namespace motis
