#pragma once

#include <vector>

#include "guess/guesser.h"

#include "motis/module/module.h"

namespace motis {
namespace guesser {

struct guesser : public motis::module::module {
  virtual ~guesser() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return true; }

  virtual std::string name() const override { return "guesser"; }
  virtual void init() override;
  virtual std::vector<MsgContent> subscriptions() const override {
    return {MsgContent_StationGuesserRequest};
  }
  virtual void on_msg(motis::module::msg_ptr, motis::module::sid,
                      motis::module::callback) override;

  std::vector<int> station_indices_;
  std::unique_ptr<guess::guesser> guesser_;
};

}  // namespace guesser
}  // namespace motis
