#include <vector>

namespace motis {
namespace reliability {

struct probability_distribution;
namespace distributions_calculator {
namespace common {
struct queue_element;
}
}

namespace rating {
namespace public_transport {

std::vector<probability_distribution> const rate(
    std::vector<distributions_calculator::common::queue_element> const&);

}  // namespace public_transport
}  // namespace rating
}  // namespace reliability
}  // namespace motis
