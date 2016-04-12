#pragma once

#include <vector>

#include "parser/buffer.h"

#include "motis/ris/ris_message.h"

namespace motis {
namespace ris {
namespace risml {

std::vector<ris_message> parse_xmls(std::vector<parser::buffer>&& strings);

}  // namespace risml
}  // namespace ris
}  // namespace motis
