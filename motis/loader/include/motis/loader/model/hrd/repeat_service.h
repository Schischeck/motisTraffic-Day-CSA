#pragma once

#include <vector>

#include "motis/loader/parsers/hrd/service/hrd_service.h"

namespace motis {
namespace loader {
namespace hrd {

void expand_repetitions(std::vector<hrd_service>&);

}  // hrd
}  // loader
}  // motis
