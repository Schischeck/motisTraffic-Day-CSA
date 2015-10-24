#pragma once

#include <cinttypes>
#include <map>
#include <vector>
#include <functional>

#include "motis/loader/loaded_file.h"
#include "motis/loader/model/hrd/shared_data.h"
#include "motis/loader/model/hrd/specification.h"
#include "motis/schedule-format/Service_generated.h"

namespace motis {
namespace loader {
namespace hrd {

void parse_specification(loaded_file const&,
                         std::function<void(specification const&)>);

void for_each_service(loaded_file const&, std::map<int, bitfield> const&,
                      std::function<void(hrd_service const&)>);

}  // hrd
}  // loader
}  // motis
