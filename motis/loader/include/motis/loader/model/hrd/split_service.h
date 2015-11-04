#pragma once

#include <vector>

#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/model/hrd/hrd_service.h"

namespace motis {
namespace loader {
namespace hrd {

void expand_traffic_days(hrd_service const&, std::map<int, bitfield> const&,
                         std::vector<hrd_service>&);

}  // hrd
}  // loader
}  // motis
