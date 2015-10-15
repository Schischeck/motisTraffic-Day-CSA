#pragma once

#include <vector>

#include "motis/module/message.h"

namespace motis {
namespace ris {

module::msg_ptr parse_xmls(std::vector<char const*> const& xml_strings);

} // namespace ris
} // namespace motis
