#pragma once

#include <cassert>
#include <cstdlib>
#include <memory>

namespace motis {
namespace routing {

struct mem_manager {
public:
  explicit mem_manager(std::size_t size)
      : size_(size),
        memory_buffer_(reinterpret_cast<unsigned char*>(operator new(size_))),
        next_position_(memory_buffer_.get()) {}

  mem_manager(mem_manager const&) = delete;
  mem_manager& operator=(mem_manager const&) = delete;

  void reset() { next_position_ = memory_buffer_.get(); }

  template <typename T, typename... Args>
  T* create(Args&&... args) {
    assert(next_position_ + sizeof(T) < memory_buffer_.get() + size());
    auto el = reinterpret_cast<T*>(next_position_);
    next_position_ += sizeof(T);
    return new (el) T(std::forward<Args>(args)...);
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
