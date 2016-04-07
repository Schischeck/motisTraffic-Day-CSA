#pragma once

#include <vector>

#include "parser/buffer.h"

namespace motis {
namespace ris {

inline std::vector<parser::buffer> pack(char const* str) {
  std::vector<parser::buffer> vec;
  parser::buffer buf(str);
  vec.emplace_back(std::move(buf));
  return vec;
}

}  // namespace ris
}  // namespace motis
