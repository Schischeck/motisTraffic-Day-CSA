#pragma once

#include <functional>
#include <vector>

#include "parser/buffer.h"

#include "motis/ris/ris_message.h"

namespace motis {
namespace ris {
namespace risml {

void xml_to_ris_message(std::string_view,
                        std::function<void(ris_message&&)> const&);

}  // namespace risml
}  // namespace ris
}  // namespace motis
