#include "motis/reliability/search/late_connections.h"

#include "motis/core/common/constants.h"

#include "motis/module/context/motis_call.h"
#include "motis/module/message.h"

#include "motis/protocol/LookupGeoStationIdRequest_generated.h"
#include "motis/protocol/LookupGeoStationResponse_generated.h"

#include "motis/reliability/intermodal/hotels.h"
#include "motis/reliability/intermodal/individual_modes_container.h"
#include "motis/reliability/reliability.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

namespace motis {
namespace reliability {
namespace search {
namespace late_connections {
namespace detail {

constexpr auto TAXI_RADIUS = 50000;  // meters

void init_taxi(
    std::string const& destination,
    std::vector<intermodal::individual_modes_container::taxi>& taxis) {
  using namespace lookup;
  module::message_creator b;
  b.create_and_finish(MsgContent_LookupGeoStationIdRequest,
                      CreateLookupGeoStationIdRequest(
                          b, b.CreateString(destination), TAXI_RADIUS)
                          .Union(),
                      "/lookup/geo_station_id");
  auto lookup_res = motis_content(LookupGeoStationResponse,
                                  motis_call(module::make_msg(b))->val());
  for (auto const& st : *lookup_res->stations()) {
    uint16_t duration = 0, price = 0;  // TODO (Mohammad Keyhan)
    taxis.emplace_back(st->id()->str(), destination, duration, price);
  }
}
}  // namespace detail

module::msg_ptr search(ReliableRoutingRequest const& req,
                       std::string const& hotels_file) {
  using namespace motis::reliability::intermodal;
  individual_modes_container container;
  parse_hotels(hotels_file, container.hotels_);
  detail::init_taxi(req.request()->destination()->id()->str(), container.taxi_);
  flatbuffers::request_builder b(req);
  b.add_additional_edges(container);
  return motis_call(b.build_routing_request())->val();
}
}  // namespace late_connections
}  // namespace search
}  // namespace reliability
}  // namespace motis
