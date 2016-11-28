#pragma once

#include <vector>

#include "parser/buffer.h"

namespace motis {
namespace routing {

struct allocator {
  allocator(size_t const initial_size) {
    mem_.emplace_back(parser::buffer(initial_size));
    clear();
  }

  inline void dealloc(void* p) {
    auto const mem_ptr = reinterpret_cast<node*>(p);
    mem_ptr->next_ = list_.next_;
    list_.next_ = mem_ptr;
  }

  inline void* alloc(size_t const size) {
    if (list_.next_ != nullptr) {
      return list_.take();
    } else if (next_ptr_ + size < end_ptr_) {
      auto const mem_ptr = next_ptr_;
      next_ptr_ += size;
      return mem_ptr;
    } else {
      mem_.emplace_back(parser::buffer{mem_.back().size_ * 2});
      set_range();
      assert(next_ptr_ + size < end_ptr_);
      auto const mem_ptr = next_ptr_;
      next_ptr_ += size;
      return mem_ptr;
    }
  }

  inline void clear() {
    list_.next_ = nullptr;
    mem_.resize(1);
    set_range();
  }

  inline size_t get_num_bytes_in_use() const {
    return std::accumulate(
        begin(mem_), end(mem_), 0,
        [](int sum, parser::buffer const& buf) { return sum + buf.size_; });
  }

private:
  inline void set_range() {
    next_ptr_ = mem_.back().begin();
    end_ptr_ = mem_.back().end();
  }

  std::vector<parser::buffer> mem_;
  unsigned char* next_ptr_;
  unsigned char* end_ptr_;

  struct node {
    void* take() {
      auto const ptr = next_;
      next_ = next_->next_;
      return ptr;
    }
    node* next_;
  } list_;
};

}  // namepsace routing
}  // namespace motis
