#include "motis/reliability/rating/public_transport.h"

#include "motis/core/schedule/schedule.h"

#include "motis/protocol/RoutingResponse_generated.h"

#include "motis/reliability/distributions_calculator.h"
#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/probability_distribution.h"

namespace motis {
namespace reliability {
using distributions_calculator::common::queue_element;

namespace rating {
namespace public_transport {

std::vector<probability_distribution> const rate(
    std::vector<queue_element> const& elements) {
  std::vector<probability_distribution> distributions;

  return distributions;
}

}  // namespace public_transport
}  // namespace rating
}  // namespace reliability
}  // namespace motis
