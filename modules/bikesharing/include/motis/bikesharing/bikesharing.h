#pragma once

#include <string>
#include <vector>

#include "motis/module/module.h"

namespace motis {
namespace bikesharing {

struct bikesharing : public motis::module::module {
  bikesharing();
  virtual ~bikesharing() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return true; }

  virtual std::string name() const override { return "bikesharing"; }
  virtual void init() override;

  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_BikesharingRequest};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  std::string database_path_;
  std::string nextbike_path_;
};

}  // namespace bikesharing
}  // namespace motis
