#pragma once

#include <cassert>
#include <cstdlib>
#include <memory>

#include "motis/routing/allocator.h"

namespace motis {
namespace routing {

struct mem_manager {
public:
  explicit mem_manager(std::size_t max_size)
      : used_size_(0), max_size_(max_size) {}

  mem_manager(mem_manager const&) = delete;
  mem_manager& operator=(mem_manager const&) = delete;

  void reset() {
    allocator_.reset();
    used_size_ = 0;
  }

  template <typename T, typename... Args>
  T* create(Args&&... args) {
    auto ptr = allocator_.allocate(sizeof(T)).ptr_;
    used_size_ += sizeof(T);
    return new (ptr) T(std::forward<Args>(args)...);
  }

  template <typename T>
  void release(T* ptr) {
    allocator_.deallocate({ptr, sizeof(T)});
    used_size_ -= sizeof(T);
  }

  std::size_t used_size() const { return used_size_; }

  std::size_t size() const { return max_size_; }

  std::vector<mem_stats> get_mem_stats() {
    std::vector<mem_stats> stats;
    allocator_.add_stats(stats);
    return stats;
  }

private:
  freelist_allocator<in_block_allocator<
      increasing_block_allocator<default_allocator, 16 * 1024 * 1024>>>
      allocator_;
  std::size_t used_size_;
  std::size_t max_size_;
};

}  // namespace routing
}  // namespace motis
