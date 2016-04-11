#pragma once

#include <memory>

#include "motis/module/module.h"

namespace motis {
namespace osrm {

struct osrm : public motis::module::module {
public:
  osrm();
  ~osrm() override;

  std::string name() const override { return "osrm"; }

  boost::program_options::options_description desc() override;
  void print(std::ostream& out) const override;
  bool empty_config() const override { return true; }

  void init(motis::module::registry&) override;

private:
  std::string path_;

  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace osrm
}  // namespace motis
