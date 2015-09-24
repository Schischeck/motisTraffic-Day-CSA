#pragma once

#include <vector>

#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/service/hrd_service.h"
#include "motis/loader/parsers/hrd/bitfield_translator.h"

namespace motis {
namespace loader {
namespace hrd {

void expand_traffic_days(hrd_service const& s, bitfield_translator& bitfields,
                         std::vector<hrd_service>& expanded_services);

}  // hrd
}  // loader
}  // motis
