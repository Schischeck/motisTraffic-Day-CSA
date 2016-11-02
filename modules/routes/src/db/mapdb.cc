#include "motis/routes/db/mapdb.h"

#include "motis/routes/db/db_builder.h"
#include "motis/routes/error.h"

namespace motis {
namespace routes {

map_database::map_database(std::string path) : path_(path) {
  db_builder b(*this);
  b.finish();
}

map_database::~map_database() = default;

void map_database::put(std::string const& key, std::string const& value) {
  db_.emplace(std::make_pair(key, value));
}

std::string map_database::get(std::string const& key) {
  auto value = try_get(key);
  if (!value) {
    return {};
  }
  return *value;
}

boost::optional<std::string> map_database::try_get(std::string const& key) {
  auto const& it = db_.find(key);
  if (it != end(db_)) {
    return it->second;
  }
  throw std::system_error(error::not_found);
}

}  // namespace routes
}  // namespace motis
