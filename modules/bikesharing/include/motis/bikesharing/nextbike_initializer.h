#pragma once

#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "motis/bikesharing/database.h"
#include "motis/bikesharing/terminal.h"

#include "parser/buffer.h"

#include "motis/bikesharing/terminal.h"

namespace motis {
namespace bikesharing {

std::time_t nextbike_filename_to_timestamp(std::string const& filename);
std::vector<terminal_snapshot> nextbike_parse_xml(parser::buffer&&);

void initialize_nextbike(std::string const& nextbike_path, database&);

}  // namespace bikesharing
}  // namespace motis
