#pragma once

#include <string>

#include "motis/module/context/motis_call.h"
#include "motis/module/message.h"

namespace motis {
namespace reliability {
namespace intermodal {

inline module::msg_ptr ask_lookup_module(std::string const station_id,
                                         unsigned const radius) {
  using namespace lookup;
  module::message_creator b;
  b.create_and_finish(
      MsgContent_LookupGeoStationIdRequest,
      CreateLookupGeoStationIdRequest(b, b.CreateString(station_id), 0.0,
                                      static_cast<double>(radius))
          .Union(),
      "/lookup/geo_station_id");
  return motis_call(module::make_msg(b))->val();
}

inline module::msg_ptr ask_lookup_module(double const& lat, double const& lng,
                                         unsigned const radius) {
  using namespace lookup;
  module::message_creator b;
  Position pos(lat, lng);
  b.create_and_finish(
      MsgContent_LookupGeoStationRequest,
      CreateLookupGeoStationRequest(b, &pos, 0.0, static_cast<double>(radius))
          .Union(),
      "/lookup/geo_station");
  return motis_call(module::make_msg(b))->val();
}

}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
