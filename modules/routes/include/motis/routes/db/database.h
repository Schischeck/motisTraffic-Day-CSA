#pragma once

#include "motis/routes/db/db_interface.h"
#include <map>

namespace motis {
namespace routes {

struct database : public kv_database {
  database(std::string file);
  ~database();

  void put(std::string const&, std::string const&);
  std::string get(std::string const&);

  std::map<std::string, std::string> db_;
};

}  // namespace routes
}  // namespace motis
