#pragma once

#include "parser/cstr.h"

#include "motis/loader/model/hrd/hrd_service.h"

namespace motis {
namespace loader {
namespace hrd {

struct range {
  range() = default;
  range(std::vector<hrd_service::stop> const& stops,
        parser::cstr from_eva_or_idx, parser::cstr to_eva_or_idx,
        parser::cstr from_hhmm_or_idx, parser::cstr to_hhmm_or_idx);

  int from_idx, to_idx;
};

}  // hrd
}  // loader
}  // motis
