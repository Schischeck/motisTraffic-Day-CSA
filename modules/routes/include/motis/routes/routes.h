#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "motis/module/module.h"
#include "motis/routes/lookup_index.h"

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
  motis::module::msg_ptr id_train_routes(motis::module::msg_ptr const&);
  motis::module::msg_ptr get_response(std::string index);

  std::string database_path_;
  std::unique_ptr<rocksdb_database> db_;
  std::unique_ptr<lookup_table> lookup_;
};

}  // namespace routes
}  // namespace motis
