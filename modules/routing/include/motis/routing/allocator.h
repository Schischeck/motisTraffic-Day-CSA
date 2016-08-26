#pragma once

#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <list>
#include <memory>

namespace motis {
namespace routing {

struct memory_block {
  void* ptr_;
  std::size_t size_;
};

struct default_allocator {
public:
  memory_block allocate(std::size_t size) { return {operator new(size), size}; }

  void deallocate(memory_block block) { operator delete(block.ptr_); }
};

template <typename BaseAllocator>
struct freelist_allocator {
private:
  BaseAllocator parent_;
  struct node {
    node* next_;
  } list_;
  unsigned long parent_allocations_;
  unsigned long freelist_allocations_;

public:
  freelist_allocator()
      : list_{nullptr}, parent_allocations_(0), freelist_allocations_(0) {}

  memory_block allocate(std::size_t size) {
    assert(size >= sizeof(node*));
    if (list_.next_ != nullptr) {
      ++freelist_allocations_;
      auto ptr = list_.next_;
      list_.next_ = ptr->next_;
      return {ptr, size};
    } else {
      ++parent_allocations_;
      return parent_.allocate(size);
    }
  }

  void deallocate(memory_block block) {
    assert(block.size_ >= sizeof(node*));
    auto p = reinterpret_cast<node*>(block.ptr_);
    p->next_ = list_.next_;
    list_.next_ = p;
  }

  void deallocate_all() {
    auto percent = parent_allocations_ == 0
                       ? 0.0
                       : 100.0 * static_cast<double>(freelist_allocations_) /
                             (parent_allocations_ + freelist_allocations_);
    std::cout << "freelist_allocator: " << parent_allocations_
              << " parent allocations, " << freelist_allocations_
              << " freelist allocations (" << percent << "%)" << std::endl;
    list_ = {nullptr};
    parent_.deallocate_all();
  }
};

template <typename BaseAllocator, std::size_t InitialSize = 1024 * 1024,
          std::size_t Factor = 2>
struct increasing_block_allocator {
public:
  increasing_block_allocator() : next_size_(InitialSize) {}

  memory_block allocate() {
    memory_block block = parent_.allocate(next_size_);
    next_size_ *= Factor;
    allocated_blocks_.emplace_back(block);
    return block;
  }

  void deallocate(memory_block block) {
    allocated_blocks_.erase(
        std::remove_if(begin(allocated_blocks_), end(allocated_blocks_),
                       [&](auto& b) { return b.ptr_ == block.ptr_; }));
    parent_.deallocate(block);
  }

  void deallocate_all() {
    std::cout << "increasing_block_allocator: " << allocated_blocks_.size()
              << " blocks ("
              << (std::pow(Factor, allocated_blocks_.size()) - 1) *
                     InitialSize / (1024 * 1024)
              << " MiB)" << std::endl;
    next_size_ = InitialSize;
    std::for_each(begin(allocated_blocks_), end(allocated_blocks_),
                  [this](auto& b) { parent_.deallocate(b); });
    allocated_blocks_.clear();
  }

private:
  BaseAllocator parent_;
  std::size_t next_size_;
  std::list<memory_block> allocated_blocks_;
};

template <typename BaseAllocator>
struct in_block_allocator {
public:
  in_block_allocator() : current_block_{nullptr, 0}, pos_(nullptr) {}

  memory_block allocate(std::size_t size) {
    if (pos_ != nullptr &&
        pos_ + size < reinterpret_cast<unsigned char*>(current_block_.ptr_) +
                          current_block_.size_) {
      void* ptr = pos_;
      pos_ += size;
      return {ptr, size};
    } else {
      current_block_ = parent_.allocate();
      assert(current_block_.size_ >= size);
      void* ptr = current_block_.ptr_;
      pos_ = reinterpret_cast<unsigned char*>(current_block_.ptr_) + size;
      return {ptr, size};
    }
  }

  void deallocate(memory_block block) { (void)block; }

  void deallocate_all() {
    current_block_ = {nullptr, 0};
    pos_ = nullptr;
    parent_.deallocate_all();
  }

private:
  BaseAllocator parent_;
  memory_block current_block_;
  unsigned char* pos_;
};

}  // namespace routing
}  // namespace motis