#include "motis/path/db/kv_database.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "motis/path/db/mapdb.h"
#include "motis/path/db/rocksdb.h"

namespace motis {
namespace path {

std::unique_ptr<kv_database> load_db(std::string const& path,
                                     bool const required) {
  if (path == ":memory:" ||
      (!required &&
       !boost::filesystem::is_directory(boost::filesystem::path{path}))) {
    return std::make_unique<map_database>(path);
  }
  return std::make_unique<rocksdb_database>(path);
}

}  // namespace path
}  // namespace motis
