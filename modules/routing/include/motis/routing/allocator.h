#pragma once

#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <vector>

namespace motis {
namespace routing {

#ifndef N_MOTIS_ROUTING_MEMORY_STATS
struct mem_stats {
  mem_stats() : name_(""), hits_(0), alloc_(0), dealloc_(0) {}
  mem_stats(char const* name) : name_(name), hits_(0), alloc_(0), dealloc_(0) {}

  std::size_t get_hits() const { return hits_; }
  std::size_t get_allocs() const { return alloc_; }
  std::size_t get_deallocs() const { return dealloc_; }
  char const* get_name() const { return name_; }

  void count_allocation(std::size_t c = 1) { alloc_ += c; }
  void count_deallocation(std::size_t c = 1) { dealloc_ += c; }
  void count_hit(std::size_t c = 1) { hits_ += c; }
  void reset() {
    hits_ = 0;
    alloc_ = 0;
    dealloc_ = 0;
  }

  char const* name_;
  std::size_t hits_, alloc_, dealloc_;
};
#else
struct mem_stats {
  mem_stats() = default;
  mem_stats(char const*) {}

  std::size_t get_hits() const { return 0; }
  std::size_t get_allocs() const { return 0; }
  std::size_t get_deallocs() const { return 0; }
  char const* get_name() const { return ""; }

  void count_allocation(std::size_t c = 1) { (void)(c); }
  void count_deallocation(std::size_t c = 1) { (void)(c); }
  void count_hit(std::size_t c = 1) { (void)(c); }
  void reset() {}
};
#endif

struct memory_block {
  void* ptr_;
  std::size_t size_;
};

struct default_allocator {
public:
  memory_block allocate(std::size_t size) { return {operator new(size), size}; }
  void deallocate(memory_block block) { operator delete(block.ptr_); }
  void reset() {}
  void add_stats(std::vector<mem_stats>&) const {}
};

template <typename BaseAllocator>
struct freelist_allocator {
  freelist_allocator() : list_{nullptr}, stats_("freelist") {}

  ~freelist_allocator() { reset(); }

  memory_block allocate(std::size_t size) {
    assert(size >= sizeof(node*));
    stats_.count_allocation();
    if (list_.next_ != nullptr) {
      stats_.count_hit();
      auto ptr = list_.next_;
      list_.next_ = ptr->next_;
      return {ptr, size};
    } else {
      return parent_.allocate(size);
    }
  }

  void deallocate(memory_block block) {
    assert(block.size_ >= sizeof(node*));
    stats_.count_deallocation();
    auto p = reinterpret_cast<node*>(block.ptr_);
    p->next_ = list_.next_;
    list_.next_ = p;
  }

  void reset() {
    list_ = {nullptr};
    stats_.reset();
    parent_.reset();
  }

  void add_stats(std::vector<mem_stats>& stats) const {
    stats.push_back(stats_);
    parent_.add_stats(stats);
  }

private:
  BaseAllocator parent_;
  struct node {
    node* next_;
  } list_;
  mem_stats stats_;
};

template <typename BaseAllocator, std::size_t InitialSize = 1024 * 1024,
          std::size_t Factor = 2>
struct increasing_block_allocator {
  increasing_block_allocator() : stats_("inc_block"), next_size_(InitialSize) {}

  ~increasing_block_allocator() { reset(); }

  memory_block allocate() {
    stats_.count_allocation();
    auto block = parent_.allocate(next_size_);
    next_size_ *= Factor;
    allocated_blocks_.emplace_back(block);
    return block;
  }

  void reset() {
    next_size_ = InitialSize;
    std::for_each(begin(allocated_blocks_), end(allocated_blocks_),
                  [this](auto& b) { parent_.deallocate(b); });
    allocated_blocks_.clear();
    stats_.reset();
    parent_.reset();
  }

  void add_stats(std::vector<mem_stats>& stats) const {
    stats.push_back(stats_);
    parent_.add_stats(stats);
  }

private:
  mem_stats stats_;
  BaseAllocator parent_;
  std::size_t next_size_;
  std::vector<memory_block> allocated_blocks_;
};

template <typename BaseAllocator>
struct in_block_allocator {
public:
  in_block_allocator()
      : stats_("in_block"), current_block_{nullptr, 0}, pos_(nullptr) {}

  ~in_block_allocator() { reset(); }

  memory_block allocate(std::size_t size) {
    stats_.count_allocation();
    if (pos_ != nullptr &&
        pos_ + size < reinterpret_cast<unsigned char*>(current_block_.ptr_) +
                          current_block_.size_) {
      stats_.count_hit();
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

  void deallocate(memory_block) {}

  void reset() {
    current_block_ = {nullptr, 0};
    pos_ = nullptr;
    stats_.reset();
    parent_.reset();
  }

  void add_stats(std::vector<mem_stats>& stats) const {
    stats.push_back(stats_);
    parent_.add_stats(stats);
  }

private:
  mem_stats stats_;
  BaseAllocator parent_;
  memory_block current_block_;
  unsigned char* pos_;
};

}  // namespace routing
}  // namespace motis
