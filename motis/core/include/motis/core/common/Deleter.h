#pragma once

namespace td {

template <typename T>
struct deleter {
  deleter(bool active) : _active(active) {}
  void operator()(T* ptr) {
    if (_active) delete ptr;
  }
  bool _active;
};

}  // namespace td
