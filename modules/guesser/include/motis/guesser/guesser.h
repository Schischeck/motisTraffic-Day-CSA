#pragma once

#include <vector>

#include "guess/guesser.h"

#include "motis/module/module.h"

namespace motis {
namespace guesser {

struct guesser : public motis::module::module {
  virtual std::string name() const override { return "guesser"; }

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;
  virtual bool empty_config() const override { return true; }

  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr guess(motis::module::msg_ptr const&);

  std::vector<unsigned> station_indices_;
  std::unique_ptr<guess::guesser> guesser_;
};

}  // namespace guesser
}  // namespace motis
