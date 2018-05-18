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

  path(path const&) = delete;
  path& operator=(path const&) = delete;

  path(path&&) = delete;
  path& operator=(path&&) = delete;

  std::string name() const override { return "path"; }
  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr boxes() const;

  motis::module::msg_ptr station_seq_path(motis::module::msg_ptr const&) const;
  motis::module::msg_ptr id_train_path(motis::module::msg_ptr const&) const;

  motis::module::msg_ptr get_response(std::string const&, int zoom_level,
                                      bool debug_info) const;

  std::string database_path_;
  size_t db_max_size_{static_cast<size_t>(1024) * 1024 * 1024 * 512};

  std::unique_ptr<kv_database> db_;
  std::unique_ptr<lookup_index> lookup_;
};

}  // namespace path
}  // namespace motis
