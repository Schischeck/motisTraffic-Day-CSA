#pragma once

namespace motis {
namespace db {

template <typename T>
struct nullable {
  nullable() : is_null_(true) {}
  nullable(T val) : val_(std::move(val)), is_null_(false) {}

  operator T&() { return val_; }
  operator T() const { return val_; }

  T val_;
  bool is_null_;
};

template <typename T>
inline std::ostream& operator<<(std::ostream& os, nullable<T> const& h) {
  return os << h.val_;
}

}  // namespace db
}  // namespace motis
