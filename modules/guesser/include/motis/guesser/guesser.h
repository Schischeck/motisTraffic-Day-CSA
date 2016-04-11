#pragma once

#include <vector>

#include "guess/guesser.h"

#include "motis/module/module.h"

namespace motis {
namespace guesser {

struct guesser : public motis::module::module {
  std::string name() const override { return "guesser"; }

  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;
  bool empty_config() const override { return true; }

  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr guess(motis::module::msg_ptr const&);

  std::vector<unsigned> station_indices_;
  std::unique_ptr<guess::guesser> guesser_;
};

}  // namespace guesser
}  // namespace motis
