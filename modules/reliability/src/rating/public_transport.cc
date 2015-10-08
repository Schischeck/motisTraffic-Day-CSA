#include "motis/reliability/rating/public_transport.h"

namespace motis {

struct schedule;

namespace reliability {
namespace rating {
namespace public_transport {

distributions_calculator::common::queue_element const to_element(
    schedule const&, std::string const& from_eva, std::string const& to_eva,
    motis::time const dep_time, motis::time const arr_time,
    std::string const& category, unsigned int const train_nr);
std::vector<distributions_calculator::common::queue_element> const get_elements(
    schedule const&, routing::Connection const*);
std::vector<probability_distribution> const rate(
    std::vector<distributions_calculator::common::queue_element> const&);

}  // namespace public_transport
}  // namespace rating
}  // namespace reliability
}  // namespace motis
