#pragma once

#include <cstdlib>
#include <string>
#include <vector>

#include "motis/db/nullable.h"

namespace motis {
namespace db {

// TODO(sebastian) null values
// TODO(sebastian) binary data

std::string serialize(char const* s) { return {s}; }
std::string serialize(std::string s) { return s; }
std::string serialize(bool b) { return b ? "TRUE" : "FALSE"; }
std::string serialize(double d) { return std::to_string(d); }
std::string serialize(int i) { return std::to_string(i); }

template <typename T>
nullable<T> deserialize(char const*);

template <>
nullable<int> deserialize<int>(char const* str) {
  return std::atoi(str);
}

template <>
nullable<double> deserialize<double>(char const* str) {
  return std::atof(str);
}

template <>
nullable<std::string> deserialize<std::string>(char const* str) {
  return std::string{str};
}

}  // namespace db
}  // namespace motis
