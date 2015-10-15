#pragma once

#include <string>

#include "motis/module/module.h"

namespace motis {
namespace ris {

struct ris : public motis::module::module {

  virtual std::string name() const override { return "ris"; }

};

} // namespace ris
} // namespace motis
