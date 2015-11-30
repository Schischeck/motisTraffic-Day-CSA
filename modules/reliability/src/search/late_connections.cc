#include "motis/reliability/search/late_connections.h"

#include "motis/reliability/reliability.h"
#include "motis/reliability/tools/flatbuffers/request_builder.h"
#include "motis/reliability/tools/hotels.h"

namespace motis {
namespace reliability {
namespace search {
namespace late_connections {
void search(ReliableRoutingRequest const* req, reliability& rel,
            motis::module::sid sid, motis::module::callback cb) {
  auto hotels =
      hotels::parse_hotels("modules/reliability/resources/hotels.csv");
  rel.send_message(
      flatbuffers::request_builder::to_late_connections_routing_request(
          req->request(), hotels),
      sid, cb);
}
}
}
}
}
