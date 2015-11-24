#pragma once

#include <string>
#include <vector>

namespace motis {
namespace reliability {
namespace hotels {

/* get the eva numbers of all stations at which hotels are located */
std::vector<std::string> parse_hotels(std::string const file_path);

}  // namespace hotels
}  // namespace reliability
}  // namespace motis
