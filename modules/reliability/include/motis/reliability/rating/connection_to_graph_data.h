#pragma once

#include <string>
#include <vector>

#include "motis/core/schedule/time.h"

namespace motis {
struct journey;
struct schedule;
namespace reliability {
namespace rating {
struct connection_element;

namespace connection_to_graph_data {
/* @return for each train in the connection, a vector of all
 * transports.  If there is more than one
 * transport element for a train (because of a change of
 * train-id or category), their connection elements
 * are inserted into the same vector. */
std::pair<bool, std::vector<std::vector<connection_element>>> get_elements(
    schedule const&, journey const&);

/* get only the last connection element of a journey */
connection_element get_last_element(schedule const& sched,
                                    journey const& journey);

namespace detail {
connection_element const to_element(
    unsigned int const departure_stop_idx, schedule const&,
    std::string const& from_eva, std::string const& to_eva,
    motis::time const dep_time, motis::time const arr_time,
    unsigned int const route_id, unsigned int const category_id,
    unsigned int const train_nr, std::string const& line_identifier);
}  // namespace detail
}  // namespace connection_to_graph_data
}  // namespace rating
}  // namespace reliability
}  // namespace motis
