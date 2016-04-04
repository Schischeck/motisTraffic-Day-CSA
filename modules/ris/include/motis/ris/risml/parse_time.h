#pragma once

#include <ctime>

#include "parser/cstr.h"

namespace motis {
namespace ris {
namespace risml {

std::time_t parse_time(parser::cstr const& raw);

}  // risml
}  // namespace ris
}  // namespace motis
