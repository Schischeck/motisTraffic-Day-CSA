#pragma once

#include "parser/cstr.h"

#include "motis/loader/parsers/hrd/service/hrd_service.h"

namespace motis {
namespace loader {
namespace hrd {

struct range {
  range(hrd_service const& service, parser::cstr from_eva_or_idx,
        parser::cstr to_eva_or_idx, parser::cstr from_hhmm_or_idx,
        parser::cstr to_hhmm_or_idx);

  const int from_idx;
  const int to_idx;
};

}  // hrd
}  // loader
}  // motis
