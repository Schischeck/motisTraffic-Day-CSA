#pragma once

#include "motis/module/module.h"

namespace motis {
namespace address {

struct address : public motis::module::module {
  address();
  ~address() override;

  void init(motis::module::registry&) override;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

}  // namespace address
}  // namespace motis
