#pragma once

#include "motis/loader/parsers/hrd/through_trains_parser.h"
#include "motis/loader/parsers/hrd/service/hrd_service.h"

namespace motis {
namespace loader {
namespace hrd {

struct through_trains_translator {
  explicit through_trains_translator(through_trains_map& through_trains);

  void try_apply_rule(hrd_service&& s, std::vector<hrd_service>& result);

  through_trains_map& through_trains_;
};

}  // hrd
}  // loader
}  // motis
