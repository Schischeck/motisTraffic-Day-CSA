#pragma once

#include <vector>

#include "flatbuffers/flatbuffers.h"
#include "parser/file.h"

#include "motis/core/access/station_access.h"
#include "motis/module/message.h"
#include "motis/protocol/Message_generated.h"

namespace motis {
namespace routes {

flatbuffers::Offset<RoutesSectionRes> routes_railroad_sec_with_data(
    station const* departure, station const* arrival, int& clasz,
    motis::module::message_creator& b, parser::buffer& buf_);

flatbuffers::Offset<RoutesSectionRes> routes_railroad_sec_without_data(
    station const* departure, station const* arrival, int& clasz,
    motis::module::message_creator& b);

}  // namespace routes
}  // namespace motis
