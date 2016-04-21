#pragma once

#include <cassert>
#include <cstdlib>
#include <memory>

namespace motis {
namespace routing {

class memory_manager {
public:
  explicit memory_manager(std::size_t size)
      : size_(size),
        memory_buffer_(reinterpret_cast<unsigned char*>(operator new(size_))),
        next_position_(memory_buffer_.get()) {}

  memory_manager(memory_manager const&) = delete;
  memory_manager& operator=(memory_manager const&) = delete;

  void reset() { next_position_ = memory_buffer_.get(); }

  template <typename T>
  T* create() {
    assert(next_position_ <= memory_buffer_.get() + size());
    auto el = reinterpret_cast<T*>(next_position_);
    next_position_ += sizeof(T);
    return el;
  }

  std::size_t used_size() const {
    return std::distance(memory_buffer_.get(), next_position_);
  }

  std::size_t size() const { return size_; }

private:
  std::size_t size_;
  std::unique_ptr<unsigned char> memory_buffer_;
  unsigned char* next_position_;
};

}  // namespace routing
}  // namespace motis
