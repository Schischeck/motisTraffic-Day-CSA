#pragma once

#include <ctime>
#include <string>
#include <vector>

#include "parser/buffer.h"

#include "motis/bikesharing/terminal.h"

namespace motis {
namespace bikesharing {

std::time_t nextbike_filename_to_timestamp(std::string const& filename);

std::vector<terminal_snapshot> nextbike_parse_xml(parser::buffer&& buffer);

}  // namespace bikesharing
}  // namespace motis
