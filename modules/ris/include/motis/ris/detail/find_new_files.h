#pragma once

#include <set>
#include <string>
#include <vector>

namespace motis {
namespace ris {
namespace detail {

std::vector<std::string> find_new_files(
    std::string const& path_string, char const* ending,
    std::set<std::string> const& known_files);

}  // namespace detail
}  // namespace ris
}  // namespace motis
