#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "motis/routes/index_lookup.h"

#include "motis/module/module.h"

namespace motis {
namespace routes {

struct rocksdb_database;

struct routes : public motis::module::module {
  routes();
  ~routes() override;

  std::string name() const override { return "routes"; }
  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr index_routes(motis::module::msg_ptr const&);
  motis::module::msg_ptr station_seq_routes(motis::module::msg_ptr const&);

  std::string database_path_;
  std::unique_ptr<rocksdb_database> db_;
  std::unique_ptr<index_lookup> lookup_;
};

}  // namespace routes
}  // namespace motis
