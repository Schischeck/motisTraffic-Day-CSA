#include "motis/ris/detail/find_new_files.h"

#include <iostream>

#include "boost/algorithm/string/predicate.hpp"
#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;
using fs::directory_iterator;

namespace motis {
namespace ris {
namespace detail {

std::vector<std::string> find_new_files(
    std::string const& path_string, std::set<std::string> const* known_files) {
  fs::path path(path_string);

  std::vector<std::string> new_files;
  if (fs::exists(path) && fs::is_directory(path)) {
    for (auto it = directory_iterator(path); it != directory_iterator(); ++it) {
      if (fs::is_directory(it->path())) {
        auto rec_res = find_new_files(it->path().string(), known_files);
        std::copy(begin(rec_res), end(rec_res), std::back_inserter(new_files));
        continue;
      }

      if (!fs::is_regular_file(it->status())) {
        continue;
      }

      auto filename = it->path().string();
      if (!boost::algorithm::iends_with(filename, ".zip")) {
        continue;
      }

      if (known_files->find(filename) == end(*known_files)) {
        new_files.push_back(filename);
      }
    }
  }

  return new_files;
}

}  // namespace detail
}  // namespace ris
}  // namespace motis
