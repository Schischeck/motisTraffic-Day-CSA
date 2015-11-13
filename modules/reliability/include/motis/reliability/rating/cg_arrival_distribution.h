#pragma once

#include <ctime>
#include <utility>

namespace motis {
namespace reliability {
struct probability_distribution;
namespace search {
struct connection_graph;
}
namespace rating {
namespace cg {

/* Delivers a pair consisting of a timestamp (begin of the distribution)
 * and the arrival distribution of the connection graph at its target */
std::pair<time_t, probability_distribution> calc_arrival_distribution(
    search::connection_graph const&);

}  // namespace cg
}  // namespace rating
}  // namespace reliability
}  // namespace motis
