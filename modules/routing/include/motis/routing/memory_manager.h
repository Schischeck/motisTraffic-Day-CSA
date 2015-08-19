#pragma once

#include <cstdlib>
#include <memory>

namespace motis {

template <typename T>
class memory_manager {
public:
  memory_manager(std::size_t size)
      : _size(size),
        _memory_buffer(new T[_size]),
        _next_position(_memory_buffer.get()) {}

  void reset() { _next_position = _memory_buffer.get(); }

  T* create() {
    assert(_next_position <= _memory_buffer.get() + _size);
    return _next_position++;
  }

  std::size_t used_size() const {
    return std::distance(_memory_buffer.get(), _next_position);
  }

private:
  std::size_t _size;
  std::unique_ptr<T[]> _memory_buffer;
  T* _next_position;
};

}  // namespace motis
