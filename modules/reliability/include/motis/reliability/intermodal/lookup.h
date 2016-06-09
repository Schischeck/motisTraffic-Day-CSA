#pragma once

#include <stdexcept>
#include <string>

#include "motis/module/context/motis_call.h"
#include "motis/module/message.h"

namespace motis {
namespace reliability {
namespace intermodal {

module::msg_ptr ask_lookup_module(std::string const station_id,
                                  unsigned const radius) {
  using namespace lookup;
  module::message_creator b;
  b.create_and_finish(
      MsgContent_LookupGeoStationIdRequest,
      CreateLookupGeoStationIdRequest(b, b.CreateString(station_id),
                                      static_cast<double>(radius))
          .Union(),
      "/lookup/geo_station_id");
  return motis_call(module::make_msg(b))->val();
}

module::msg_ptr ask_lookup_module(double const& lat, double const& lng,
                                  unsigned const radius) {
  using namespace lookup;
  module::message_creator b;
  Position pos(lat, lng);
  b.create_and_finish(
      MsgContent_LookupGeoStationRequest,
      CreateLookupGeoStationRequest(b, &pos, static_cast<double>(radius))
          .Union(),
      "/lookup/geo_station");
  return motis_call(module::make_msg(b))->val();
}

}  // namespace intermodal
}  // namespace reliability
}  // namespace motis
