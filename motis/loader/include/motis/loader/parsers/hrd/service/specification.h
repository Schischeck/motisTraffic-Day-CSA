#pragma once

#include <vector>

#include "parser/cstr.h"

namespace motis {
namespace loader {
namespace hrd {

struct specification {
  specification() : internal_service(nullptr, 0) {}

  bool is_empty() const;

  bool valid() const;

  bool ignore() const;

  void reset();

  bool read_line(parser::cstr line, char const* filename, int line_number);

  parser::cstr internal_service;
  std::vector<parser::cstr> traffic_days;
  std::vector<parser::cstr> categories;
  std::vector<parser::cstr> line_information;
  std::vector<parser::cstr> attributes;
  std::vector<parser::cstr> directions;
  std::vector<parser::cstr> stops;
};

}  // hrd
}  // loader
}  // motis
