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
  ~bikesharing();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return true; }

  virtual std::string name() const override { return "bikesharing"; }
  virtual void init_async() override;

  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_BikesharingRequest};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  std::string database_path_;
  std::string nextbike_path_;

  std::unique_ptr<database> database_;
  std::unique_ptr<bikesharing_search> search_;
};

}  // namespace bikesharing
}  // namespace motis
