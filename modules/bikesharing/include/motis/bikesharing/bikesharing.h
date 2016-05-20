#pragma once

#include <string>
#include <vector>

#include "motis/module/module.h"

namespace motis {
namespace bikesharing {

struct database;
struct bikesharing_search;

struct bikesharing : public motis::module::module {
  bikesharing();

  std::string name() const override { return "bikesharing"; }
  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;
  bool empty_config() const override { return false; }

  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr init_module(motis::module::msg_ptr const&);
  motis::module::msg_ptr request(motis::module::msg_ptr const&);

  std::string database_path_;
  std::string nextbike_path_;

  std::unique_ptr<database> database_;
  std::unique_ptr<bikesharing_search> search_;
};

}  // namespace bikesharing
}  // namespace motis
