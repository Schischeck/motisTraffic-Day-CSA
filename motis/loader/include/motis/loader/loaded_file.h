#pragma once

#include "parser/cstr.h"

namespace motis {
namespace loader {

struct loaded_file {
  char const* name;
  parser::cstr content;
};

}  // namespace loader
}  // namespace motis
