#pragma once

#include "motis/module/module.h"

namespace motis {
namespace reliability {

struct reliability : public motis::module::module {
	reliability();

  virtual std::string name() const override { return "reliability"; }
  virtual std::vector<json11::Json> on_msg(json11::Json const&,
                                           motis::module::sid) override;

  typedef std::function<std::vector<json11::Json>(
      reliability*, json11::Json const& msg)> operation;
  std::map<std::string, operation> ops_;
};


}  // namespace reliability
}  // namespace motis
