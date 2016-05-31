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
    routing::RoutingRequest const& request,
    std::vector<intermodal::hotels::hotel_info> const& hotel_infos) {}
}  // namespace detail

module::msg_ptr search(ReliableRoutingRequest const& req,
                       std::string const& hotels_file) {
  return motis_call(
             detail::to_routing_late_connections_message(
                 *req.request(), intermodal::hotels::parse_hotels(hotels_file)))
      ->val();
}
}  // namespace late_connections
}  // namespace search
}  // namespace reliability
}  // namespace motis
