#pragma once

#include <vector>

#include "parser/buffer.h"

#include "motis/ris/ris_message.h"

namespace motis {
namespace ris {

std::vector<ris_message> parse_xmls(std::vector<parser::buffer>&& xml_strings);

} // namespace ris
} // namespace motis
