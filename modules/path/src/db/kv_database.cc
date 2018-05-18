#include "motis/path/db/kv_database.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "motis/path/db/lmdb.h"
#include "motis/path/db/mapdb.h"

namespace motis {
namespace path {

std::unique_ptr<kv_database> load_db(std::string const& path,
                                     size_t const max_db_size,
                                     bool const required) {
  if (path == ":memory:" || (!required && !boost::filesystem::is_directory(
                                              boost::filesystem::path{path}))) {
    return std::make_unique<map_database>();
  }
  return std::make_unique<lmdb_database>(path, max_db_size);
}

}  // namespace path
}  // namespace motis
