#pragma once

#include "parser/cstr.h"
#include "parser/buffer.h"

namespace motis {
namespace loader {

struct loaded_file {
  loaded_file() = default;
  loaded_file(char const* name, parser::buffer const& buf)
      : name(name), content(buf.data(), buf.size()) {}
  loaded_file(char const* name, parser::cstr content)
      : name(name), content(content) {}

  char const* name;
  parser::cstr content;
};

}  // namespace loader
}  // namespace motis
