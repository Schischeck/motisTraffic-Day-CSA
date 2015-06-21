#pragma once

#include "motis/module/module.h"

namespace motis {
namespace railviz {

struct railviz : public motis::module::module {
  railviz();

  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "railviz"; }
  virtual std::vector<json11::Json> on_msg(json11::Json const&,
                                           motis::module::sid) override;

  typedef std::function<std::vector<json11::Json>(
      railviz*, json11::Json const& msg)> operation;
  std::map<std::string, operation> ops_;
};

}  // namespace railviz
}  // namespace motis
