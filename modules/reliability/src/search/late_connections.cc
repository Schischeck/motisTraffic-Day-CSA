#include "motis/reliability/search/late_connections.h"

#include "motis/core/common/constants.h"

#include "motis/module/context/motis_call.h"
#include "motis/module/message.h"

#include "motis/reliability/intermodal/hotels.h"
#include "motis/reliability/intermodal/individual_modes_container.h"
#include "motis/reliability/reliability.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"

namespace motis {
namespace reliability {
namespace search {
namespace late_connections {

module::msg_ptr search(ReliableRoutingRequest const& req,
                       std::string const& hotels_file) {
  intermodal::individual_modes_container container;
  intermodal::parse_hotels(hotels_file, container.hotels_);

  /* TODO(Mohammad Keyhani) ask lookup module for taxi */

  flatbuffers::request_builder b(req);
  b.add_additional_edges(container);
  return motis_call(b.build_routing_request())->val();
}
}  // namespace late_connections
}  // namespace search
}  // namespace reliability
}  // namespace motis
