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
        line_number_from_(BEFORE_FIRST_LINE),
        line_number_to_(BEFORE_FIRST_LINE),
        internal_service_(nullptr, 0) {}

  bool is_empty() const;

  bool valid() const;

  bool ignore() const;

  void reset();

  bool read_line(parser::cstr line, char const* filename, int line_number);

  char const* filename_;
  int line_number_from_;
  int line_number_to_;
  parser::cstr internal_service_;
  std::vector<parser::cstr> traffic_days_;
  std::vector<parser::cstr> categories_;
  std::vector<parser::cstr> line_information_;
  std::vector<parser::cstr> attributes_;
  std::vector<parser::cstr> directions_;
  std::vector<parser::cstr> stops_;
};

}  // namespace hrd
}  // namespace loader
}  // namespace motis
