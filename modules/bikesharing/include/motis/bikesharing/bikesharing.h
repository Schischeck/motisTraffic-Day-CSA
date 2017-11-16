#pragma once

#include <string>
#include <vector>

#include "motis/module/module.h"

namespace motis {
namespace bikesharing {

struct database;
struct geo_index;

struct bikesharing : public motis::module::module {
  bikesharing();
  ~bikesharing() override;

  bikesharing(bikesharing const&) = delete;
  bikesharing& operator=(bikesharing const&) = delete;

  bikesharing(bikesharing&&) = delete;
  bikesharing& operator=(bikesharing&&) = delete;

  std::string name() const override { return "bikesharing"; }
  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr init_module(motis::module::msg_ptr const&);
  motis::module::msg_ptr search(motis::module::msg_ptr const&) const;
  motis::module::msg_ptr geo_terminals(motis::module::msg_ptr const&) const;

  void ensure_initialized() const;

  std::string database_path_;
  std::string nextbike_path_;

  std::unique_ptr<database> database_;
  std::unique_ptr<geo_index> geo_index_;
};

}  // namespace bikesharing
}  // namespace motis
