#pragma once

#include <string>
#include <vector>

#include "motis/core/schedule/time.h"

#include "motis/module/module.h"

namespace motis {
namespace reliability {
namespace flatbuffers_tools {

inline module::msg_ptr to_flatbuffers_message(std::string const& from_name,
                                              std::string const& from_eva,
                                              std::string const& to_name,
                                              std::string const& to_eva,
                                              motis::time interval_begin,
                                              motis::time interval_end) {
  /* convert routing::RoutingRequest to flatbuffers::Offset<RoutingRequest> */
  flatbuffers::FlatBufferBuilder b;
  std::vector<flatbuffers::Offset<routing::StationPathElement> >
      station_elements;
  station_elements.push_back(routing::CreateStationPathElement(
      b, b.CreateString(from_name), b.CreateString(from_eva)));
  station_elements.push_back(routing::CreateStationPathElement(
      b, b.CreateString(to_name), b.CreateString(to_eva)));
  routing::Interval interval(
      motis_to_unixtime(to_unix_time(2015, 9, 28), interval_begin),
      motis_to_unixtime(to_unix_time(2015, 9, 28), interval_end));
  b.Finish(CreateMessage(
      b, MsgContent_RoutingRequest,
      routing::CreateRoutingRequest(b, &interval, routing::Type::Type_PreTrip,
                                    routing::Direction::Direction_Forward,
                                    b.CreateVector(station_elements))
          .Union()));
  return module::make_msg(b);
}

}  // namespace flatbuffers_tools
}  // namespace reliability
}  // namespace motis
