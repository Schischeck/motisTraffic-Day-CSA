#include "motis/reliability/search/late_connections.h"

#include "motis/core/common/constants.h"

#include "motis/module/context/motis_call.h"
#include "motis/module/message.h"

#include "motis/reliability/intermodal/hotels.h"
#include "motis/reliability/reliability.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

namespace motis {
namespace reliability {
namespace search {
namespace late_connections {
namespace detail {
/* convert routing::RoutingRequest to Offset<RoutingRequest> */
module::msg_ptr to_routing_late_connections_message(
    routing::RoutingRequest const* request,
    std::vector<intermodal::hotels::hotel_info> const& hotel_infos) {
  using namespace routing;
  flatbuffers::request_builder builder(request);
  builder.type_ = Type::Type_LateConnection;

  auto& b = builder.b_;
  /* Create taxi-edges */
  for (auto const& e : *request->additional_edges()) {
    if (e->additional_edge_type() != AdditionalEdge_TimeDependentMumoEdge) {
      continue;
    }
    auto const* mumo =
        static_cast<TimeDependentMumoEdge const*>(e->additional_edge());
    /* TODO(Mohammad Keyhani) ask intermodal modul for taxi-edges */
    builder.add_additional_edge(CreateAdditionalEdgeWrapper(
        b, AdditionalEdge_PeriodicMumoEdge,
        CreatePeriodicMumoEdge(
            b, CreateMumoEdge(
                   b, b.CreateString(mumo->edge()->from_station_eva()->c_str()),
                   b.CreateString(mumo->edge()->to_station_eva()->c_str()),
                   mumo->edge()->duration(), mumo->edge()->price()),
            LATE_TAXI_BEGIN_TIME, LATE_TAXI_END_TIME)
            .Union()));
  }
  for (auto const& hotel : hotel_infos) {
    builder.add_additional_edge(CreateAdditionalEdgeWrapper(
        b, AdditionalEdge_HotelEdge,
        CreateHotelEdge(b, b.CreateString(hotel.station_),
                        hotel.earliest_checkout_, hotel.min_stay_duration_,
                        hotel.price_)
            .Union()));
  }

  return builder.build_routing_request();
}
}

module::msg_ptr search(ReliableRoutingRequest const* req,
                       std::string const& hotels_file) {
  return motis_call(
             detail::to_routing_late_connections_message(
                 req->request(), intermodal::hotels::parse_hotels(hotels_file)))
      ->val();
}
}
}
}
}
