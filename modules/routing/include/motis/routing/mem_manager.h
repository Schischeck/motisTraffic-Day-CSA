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
      : used_size_(0),
        max_size_(max_size),
        allocations_(0),
        deallocations_(0) {}

  mem_manager(mem_manager const&) = delete;
  mem_manager& operator=(mem_manager const&) = delete;

  void reset() {
    auto percent = allocations_ == 0
                       ? 0.0
                       : 100.0 * static_cast<double>(deallocations_) /
                             (allocations_ + deallocations_);
    std::cout << "memory_manager: " << allocations_ << " allocations, "
              << deallocations_ << " deallocations (" << percent << "%)"
              << std::endl;
    allocator_.deallocate_all();
    used_size_ = 0;
  }

  template <typename T, typename... Args>
  T* create(Args&&... args) {
    auto ptr = allocator_.allocate(sizeof(T)).ptr_;
    used_size_ += sizeof(T);
    ++allocations_;
    return new (ptr) T(std::forward<Args>(args)...);
  }

  template <typename T>
  void release(T* ptr) {
    allocator_.deallocate({ptr, sizeof(T)});
    ++deallocations_;
    used_size_ -= sizeof(T);
  }

  std::size_t used_size() const { return used_size_; }

  std::size_t size() const { return max_size_; }

private:
  freelist_allocator<
      in_block_allocator<increasing_block_allocator<default_allocator>>>
      allocator_;
  std::size_t used_size_;
  std::size_t max_size_;

  unsigned long allocations_;
  unsigned long deallocations_;
};

}  // namespace routing
}  // namespace motis
