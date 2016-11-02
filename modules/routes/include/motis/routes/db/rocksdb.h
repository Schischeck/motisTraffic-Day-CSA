#pragma once

#include <memory>

#include "motis/routes/db/kv_database.h"

namespace motis {
namespace routes {

struct rocksdb_database : public kv_database {
  rocksdb_database(std::string path);
  ~rocksdb_database();

  void put(std::string const&, std::string const&);
  std::string get(std::string const&);
  boost::optional<std::string> try_get(std::string const&);

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace routes
}  // namespace motis
