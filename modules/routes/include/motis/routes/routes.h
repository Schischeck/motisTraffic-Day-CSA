#pragma once

#include <string>
#include <utility>

#include "motis/module/module.h"

namespace motis {
namespace routes {

struct kv_database;
struct lookup_index;

struct routes : public motis::module::module {
  routes();
  ~routes() override;

  std::string name() const override { return "routes"; }
  void init(motis::module::registry&) override;

  std::unique_ptr<kv_database> db_;
  std::unique_ptr<lookup_index> lookup_;

private:
  motis::module::msg_ptr index_routes(motis::module::msg_ptr const&) const;
  motis::module::msg_ptr station_seq_routes(
      motis::module::msg_ptr const&) const;
  motis::module::msg_ptr id_train_routes(motis::module::msg_ptr const&) const;
  motis::module::msg_ptr get_response(std::string const&) const;

  std::string database_path_;
  bool required_;
};

}  // namespace routes
}  // namespace motis
