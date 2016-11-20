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
        memory_buffer_(static_cast<unsigned char*>(std::malloc(size_))),
        next_position_(memory_buffer_) {
    if (memory_buffer_ == nullptr) {
      throw std::runtime_error("out of memory");
    }
  }

  mem_manager(mem_manager const&) = delete;
  mem_manager& operator=(mem_manager const&) = delete;

  ~mem_manager() { std::free(memory_buffer_); }

  void reset() {
    next_position_ = memory_buffer_;
    for (auto& labels : node_labels_) {
      labels.clear();
    }
  }

  template <typename T, typename... Args>
  T* create(Args&&... args) {
    assert(next_position_ + sizeof(T) < memory_buffer_ + size());
    auto el = reinterpret_cast<T*>(next_position_);
    next_position_ += sizeof(T);
    return new (el) T(std::forward<Args>(args)...);
  }

  std::size_t used_size() const {
    return std::distance(memory_buffer_, next_position_);
  }

  std::size_t size() const { return size_; }

  template <typename T>
  std::vector<std::vector<T*>>* get_node_labels(std::size_t size) {
    node_labels_.resize(size);
    return reinterpret_cast<std::vector<std::vector<T*>>*>(&node_labels_);
  }

private:
  std::size_t size_;
  unsigned char* memory_buffer_;
  unsigned char* next_position_;

  std::vector<std::vector<void*>> node_labels_;
};

}  // namespace routing
}  // namespace motis
