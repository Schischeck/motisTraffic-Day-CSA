#pragma once

#include <stdexcept>

namespace motis {
namespace loader {

struct parser_error : public std::exception {
  parser_error(char const* filename, int line_number)
      : filename(filename), line_number(line_number) {}

  char const* filename;
  int line_number;

  char const* what() const noexcept { return "parser_error"; }
};

}  // namespace loader
}  // namespace motis
