#pragma once

#include <vector>

#include "parser/buffer.h"

#include "motis/module/message.h"

namespace motis {
namespace ris {

module::msg_ptr parse_xmls(std::vector<parser::buffer>&& xml_strings);

} // namespace ris
} // namespace motis
