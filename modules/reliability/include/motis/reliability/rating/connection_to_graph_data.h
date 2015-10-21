#pragma once

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
namespace rating {
struct connection_element;

namespace connection_to_graph_data {
/* @return for each transport, a vector of all connection-elements */
std::pair<bool, std::vector<std::vector<connection_element>>> const
get_elements(schedule const&, routing::Connection const*);

namespace detail {
connection_element const to_element(
    unsigned int const departure_stop_idx, schedule const&,
    std::string const& from_eva, std::string const& to_eva,
    motis::time const dep_time, motis::time const arr_time,
    std::string const& category, unsigned int const train_nr);
}  // namespace detail
}  // namespace connection_to_graph_data
}  // namespace rating
}  // namespace reliability
}  // namespace motis
