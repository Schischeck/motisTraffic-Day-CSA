#pragma once

#include "boost/optional.hpp"

#include "parser/cstr.h"

#include "motis/protocol/RISMessage_generated.h"

namespace motis {
namespace ris {
namespace risml {

boost::optional<EventType> parse_type(
    parser::cstr const& raw,
    boost::optional<EventType> const& default_value = boost::none);

}  // namespace risml
}  // namespace ris
}  // namespace motis
