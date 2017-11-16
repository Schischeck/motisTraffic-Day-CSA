#pragma once

#include <string>

#include "motis/module/module.h"

namespace motis {
namespace intermodal {

struct intermodal : public motis::module::module {
public:
  intermodal();
  ~intermodal() override;

  intermodal(intermodal const&) = delete;
  intermodal& operator=(intermodal const&) = delete;

  intermodal(intermodal&&) = delete;
  intermodal& operator=(intermodal&&) = delete;

  std::string name() const override { return "intermodal"; }
  void init(motis::module::registry&) override;

private:
  motis::module::msg_ptr route(motis::module::msg_ptr const&);
};

}  // namespace intermodal
}  // namespace motis
