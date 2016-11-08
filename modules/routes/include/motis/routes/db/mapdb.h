#pragma once

#include <map>

#include "motis/routes/db/kv_database.h"

namespace motis {
namespace routes {

struct map_database : public kv_database {
  map_database(std::string path);
  ~map_database();

  void put(std::string const&, std::string const&);
  std::string get(std::string const&);
  boost::optional<std::string> try_get(std::string const&);

  std::map<std::string, std::string> db_;
  std::string path_;
};

}  // namespace routes
}  // namespace motis
