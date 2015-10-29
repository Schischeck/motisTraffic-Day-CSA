#pragma once

#include <vector>

#include "parser/cstr.h"

namespace motis {
namespace loader {
namespace hrd {

constexpr char const* UNKNOWN_FILE = "unknown_file";
constexpr int BEFORE_FIRST_LINE = -1;

struct specification {
  specification()
      : filename_(UNKNOWN_FILE),
        line_number_(BEFORE_FIRST_LINE),
        internal_service(nullptr, 0) {}

  bool is_empty() const;

  bool valid() const;

  bool ignore() const;

  void reset();

  bool read_line(parser::cstr line, char const* filename, int line_number);

  char const* filename_;
  int line_number_;
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
