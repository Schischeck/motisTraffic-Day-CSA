#pragma once

#include <ctime>
#include <memory>
#include <string>
#include <vector>

#include "motis/module/callbacks.h"
#include "motis/bikesharing/database.h"
#include "motis/bikesharing/terminal.h"

#include "parser/buffer.h"

#include "motis/bikesharing/terminal.h"

namespace motis {
namespace bikesharing {

std::time_t nextbike_filename_to_timestamp(std::string const& filename);
std::vector<terminal_snapshot> nextbike_parse_xml(parser::buffer&& buffer);

using dispatch_fun = std::function<void(module::msg_ptr, module::callback)>;
void initialize_nextbike(std::string const& nextbike_path, database& db,
                         dispatch_fun dispatch_fun,
                         module::callback finished_cb);

}  // namespace bikesharing
}  // namespace motis
