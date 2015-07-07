#pragma once

#include "motis/module/module.h"

namespace motis {
namespace guesser {

struct guesser : public motis::module::module {
  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "guesser"; }
  virtual json11::Json on_msg(json11::Json const&, motis::module::sid) override;

  typedef std::function<json11::Json(guesser*, json11::Json const& msg)> op;
  std::map<std::string, op> ops_;
};

}  // namespace guesser
}  // namespace motis
