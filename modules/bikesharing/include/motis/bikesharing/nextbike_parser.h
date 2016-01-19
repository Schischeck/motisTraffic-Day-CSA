#pragma once

#include <vector>

#include "parser/buffer.h"

namespace motis {
namespace bikesharing {

struct parsed_bikesharing_terminal {
  int uid;
  double lat, lng;
  std::string name;
  int available_bikes;
};

std::vector<parsed_bikesharing_terminal> parse_nextbike_xml(
    parser::buffer&& buffer);

}  // namespace bikesharing
}  // namespace motis
