#pragma once

#include "motis/module/module.h"

namespace motis {
namespace routing {

struct routing : public motis::module::module {
  virtual boost::program_options::options_description desc() override;
  virtual void print(std::ostream& out) const override;

  virtual std::string name() const override { return "routing"; }
  virtual void on_msg(json11::Json const&, motis::module::sid,
                      motis::module::callback cb) override;
};

}  // namespace routing
}  // namespace motis
