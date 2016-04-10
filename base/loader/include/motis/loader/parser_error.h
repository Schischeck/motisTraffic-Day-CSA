#pragma once

#include <stdexcept>

namespace motis {
namespace loader {

struct parser_error : public std::exception {
  parser_error(char const* filename, int line_number)
      : filename_copy(filename),
        filename(filename_copy.c_str()),
        line_number(line_number) {}

  char const* what() const noexcept override { return "parser_error"; }

  void print_what() const noexcept {
    printf("%s:%s:%d\n", what(), filename, line_number);
  }

  std::string filename_copy;
  char const* filename;
  int line_number;
};

}  // namespace loader
}  // namespace motis
