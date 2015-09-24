#pragma once

#include <cinttypes>
#include <map>
#include <vector>
#include <functional>

#include "motis/loader/loaded_file.h"
#include "motis/loader/parsers/hrd/service/shared_data.h"
#include "motis/loader/parsers/hrd/service/specification.h"
#include "motis/schedule-format/Service_generated.h"

namespace motis {
namespace loader {
namespace hrd {

void parse_services(loaded_file const&,
                    std::function<void(specification const&)>);

}  // hrd
}  // loader
}  // motis
