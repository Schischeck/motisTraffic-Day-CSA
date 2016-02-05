#pragma once

#include <vector>

namespace motis {
namespace reliability {
struct context;
namespace rating {
struct connection_element;
struct rating_element;

namespace public_transport {
void rate(std::vector<rating_element>& ratings,
          std::vector<std::vector<connection_element>> const&,
          bool const first_element_already_processed, context const&);

}  // namespace public_transport
}  // namespace rating
}  // namespace reliability
}  // namespace motis
