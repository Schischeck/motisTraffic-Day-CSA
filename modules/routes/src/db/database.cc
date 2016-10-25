#include "motis/routes/db/database.h"

namespace motis {
namespace routes {

database::database(std::string file) : kv_database::kv_database() {}

database::~database() = default;

void database::put(std::string const& key, std::string const& value) {
  db_.emplace(std::make_pair(key, value));
}

std::string database::get(std::string const& key) {
  auto value = db_.find(key);
  if (value != end(db_)) {
    return value->second;
  }
  return "";
}

}  // namespace routes
}  // namespace motis
