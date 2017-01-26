#pragma once

#include <string>
#include <utility>

#include "motis/module/module.h"

namespace motis {
namespace path {

struct kv_database;
struct lookup_index;

struct path : public motis::module::module {
  path();
  ~path() override;

  std::string name() const override { return "path"; }
  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr station_seq_path(motis::module::msg_ptr const&) const;
  motis::module::msg_ptr id_train_path(motis::module::msg_ptr const&) const;

  motis::module::msg_ptr get_response(std::string const&, int const zoom_level,
                                      bool const debug_info) const;

  std::string database_path_;

  std::unique_ptr<kv_database> db_;
  std::unique_ptr<lookup_index> lookup_;
};

}  // namespace path
}  // namespace motis
