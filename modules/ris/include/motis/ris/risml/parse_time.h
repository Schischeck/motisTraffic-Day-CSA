#pragma once

#include <ctime>

#include "parser/cstr.h"

namespace motis {
namespace ris {
namespace risml {

struct context;

std::time_t parse_time(parser::cstr const&);

std::time_t parse_schedule_time(context&, parser::cstr const&);

}  // namespace risml
}  // namespace ris
}  // namespace motis
