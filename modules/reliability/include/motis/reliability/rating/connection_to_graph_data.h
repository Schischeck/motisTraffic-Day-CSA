#include <string>
#include <vector>

#include "motis/core/schedule/time.h"

namespace motis {

struct schedule;

namespace routing {
struct Connection;
}

namespace reliability {

struct probability_distribution;
namespace distributions_calculator {
namespace common {
struct queue_element;
}
}

namespace rating {
namespace connection_to_graph_data {
std::vector<distributions_calculator::common::queue_element> const get_elements(
    schedule const&, routing::Connection const*);

namespace detail {
distributions_calculator::common::queue_element const to_element(
    schedule const&, std::string const& from_eva, std::string const& to_eva,
    motis::time const dep_time, motis::time const arr_time,
    std::string const& category, unsigned int const train_nr);
}  // namespace detail
}  // namespace connection_to_graph_data
}  // namespace rating
}  // namespace reliability
}  // namespace motis
