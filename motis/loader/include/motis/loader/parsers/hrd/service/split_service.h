#pragma once

#include <vector>

#include "motis/loader/parsers/hrd/bitfields_parser.h"
#include "motis/loader/parsers/hrd/service/hrd_service.h"
#include "motis/loader/parsers/hrd/bitfield_translator.h"

namespace motis {
namespace loader {
namespace hrd {

std::vector<hrd_service> expand_traffic_days(hrd_service const& s,
                                             bitfield_translator& bitfields);

}  // hrd
}  // loader
}  // motis
