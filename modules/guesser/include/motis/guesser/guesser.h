#pragma once

#include "motis/module/module.h"

namespace motis {
namespace guesser {

struct guesser : public motis::module::module {
  virtual ~guesser() {}

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "guesser"; }
  virtual std::vector<MsgContent> subscriptions() const {
    return {MsgContent_StationGuesserRequest};
  }
  virtual motis::module::msg_ptr on_msg(motis::module::msg_ptr const&,
                                        motis::module::sid) override;
};

}  // namespace guesser
}  // namespace motis
