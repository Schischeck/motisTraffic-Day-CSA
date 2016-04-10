#pragma once

#include <string>
#include <vector>

#include "parser/buffer.h"

namespace motis {
namespace ris {

std::vector<parser::buffer> read_zip_buf(parser::buffer const& b);
std::vector<parser::buffer> read_zip_file(std::string const& filename);

}  // namespace ris
}  // namespace motis
