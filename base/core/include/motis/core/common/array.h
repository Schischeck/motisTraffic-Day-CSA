#pragma once

#include <cassert>
#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <ostream>
#include <string>
#include <type_traits>

namespace motis {

inline uint64_t next_power_of_two(uint64_t n) {
  n--;
  n |= n >> 1u;
  n |= n >> 2u;
  n |= n >> 4u;
  n |= n >> 8u;
  n |= n >> 16u;
  n |= n >> 32u;
  n++;
  return n;
}

template <typename T, typename TemplateSizeType = uint32_t>
struct array final {
  using size_type = TemplateSizeType;

  explicit array(TemplateSizeType size = 0)
      : el_(nullptr),
        used_size_(0),
        self_allocated_(false),
        allocated_size_(0) {
    resize(size);
  }

  explicit array(const char* str)
      : el_(nullptr),
        used_size_(0),
        self_allocated_(false),
        allocated_size_(0) {
    auto length = std::strlen(str) + 1;
    reserve(length);
    std::memcpy(el_, str, length);
    used_size_ = length;
  }

  template <typename It>
  array(It begin_it, It end_it)
      : el_(nullptr),
        used_size_(0),
        self_allocated_(false),
        allocated_size_(0) {
    set(begin_it, end_it);
  }

  array(array&& arr) noexcept
      : el_(arr.el_),
        used_size_(arr.used_size_),
        self_allocated_(arr.self_allocated_),
        allocated_size_(arr.allocated_size_) {
    arr.el_ = nullptr;
    arr.used_size_ = 0;
    arr.self_allocated_ = false;
    arr.allocated_size_ = 0;
  }

  array(array const& arr)
      : el_(nullptr),
        used_size_(0),
        self_allocated_(false),
        allocated_size_(0) {
    set(std::begin(arr), std::end(arr));
  }

  array& operator=(array&& arr) noexcept {
    deallocate();

    el_ = arr.el_;
    used_size_ = arr.used_size_;
    self_allocated_ = arr.self_allocated_;
    allocated_size_ = arr.allocated_size_;

    arr.el_ = nullptr;
    arr.used_size_ = 0;
    arr.self_allocated_ = false;
    arr.allocated_size_ = 0;

    return *this;
  }

  array& operator=(array const& arr) {
    set(std::begin(arr), std::end(arr));
    return *this;
  }

  ~array() { deallocate(); }

  void deallocate() {
    if (!self_allocated_ || el_ == nullptr) {
      return;
    }

    for (auto& el : *this) {
      el.~T();
    }

    std::free(el_);  // NOLINT
    el_ = nullptr;
    used_size_ = 0;
    allocated_size_ = 0;
    self_allocated_ = 0;
  }

  T const* begin() const { return el_; }
  T const* end() const { return el_ + used_size_; }  // NOLINT
  T* begin() { return el_; }
  T* end() { return el_ + used_size_; }  // NOLINT

  std::reverse_iterator<T const*> rbegin() const {
    return std::reverse_iterator<T*>(el_ + size());  // NOLINT
  }
  std::reverse_iterator<T const*> rend() const {
    return std::reverse_iterator<T*>(el_);
  }
  std::reverse_iterator<T*> rbegin() {
    return std::reverse_iterator<T*>(el_ + size());  // NOLINT
  }
  std::reverse_iterator<T*> rend() { return std::reverse_iterator<T*>(el_); }

  friend T const* begin(array const& a) { return a.begin(); }
  friend T const* end(array const& a) { return a.end(); }

  friend T* begin(array& a) { return a.begin(); }
  friend T* end(array& a) { return a.end(); }

  inline T const& operator[](int index) const { return el_[index]; }
  inline T& operator[](int index) { return el_[index]; }

  T const& back() const { return el_[used_size_ - 1]; }
  T& back() { return el_[used_size_ - 1]; }

  T& front() { return el_[0]; }
  T const& front() const { return el_[0]; }

  inline TemplateSizeType size() const { return used_size_; }
  inline bool empty() const { return size() == 0; }

  array& operator=(std::string const& str) {
    *this = array(str.c_str());
    return *this;
  }

  template <typename It>
  void set(It begin_it, It end_it) {
    auto range_size = std::distance(begin_it, end_it);
    reserve(range_size);

    auto copy_source = begin_it;
    auto copy_target = el_;
    for (; copy_source != end_it; ++copy_source, ++copy_target) {
      new (copy_target) T(*copy_source);
    }

    used_size_ = range_size;
  }

  void push_back(T const& el) {
    reserve(used_size_ + 1);
    new (el_ + used_size_) T(el);
    ++used_size_;
  }

  template <typename... Args>
  void emplace_back(Args&&... el) {
    reserve(used_size_ + 1);
    new (el_ + used_size_) T(std::forward<Args>(el)...);
    ++used_size_;
  }

  void resize(size_type size) {
    reserve(size);
    used_size_ = size;
  }

  void clear() {
    used_size_ = 0;
    for (auto& el : *this) {
      el.~T();
    }
  }

  void reserve(TemplateSizeType new_size) {
    new_size = std::max(allocated_size_, new_size);

    if (allocated_size_ >= new_size) {
      return;
    }

    auto next_size = next_power_of_two(new_size);
    auto mem_buf =
        static_cast<T*>(std::malloc(sizeof(T) * next_size));  // NOLINT
    if (mem_buf == nullptr) {
      throw std::bad_alloc();
    }

    if (size() != 0) {
      try {
        auto move_target = mem_buf;
        for (auto& el : *this) {
          new (move_target++) T(std::move(el));
        }

        for (auto& el : *this) {
          el.~T();
        }
      } catch (...) {
        assert(0);
      }
    }

    auto free_me = el_;
    el_ = mem_buf;
    if (self_allocated_) {
      std::free(free_me);  // NOLINT
    }

    self_allocated_ = true;
    allocated_size_ = next_size;
  }

  T* erase(T* pos) {
    T* last = end() - 1;
    while (pos < last) {
      std::swap(*pos, *(pos + 1));
      pos = pos + 1;
    }
    pos->~T();
    --used_size_;
    return end();
  }

  bool contains(T const* el) const { return el >= begin() && el < end(); }

  std::string to_string() const { return std::string(el_); }

  explicit operator std::string() const { return to_string(); }

#ifdef USE_STANDARD_LAYOUT
  T* el_;
  TemplateSizeType used_size_;
  TemplateSizeType allocated_size_;
  bool self_allocated_;
#else
  T* el_;
  TemplateSizeType used_size_;
  bool self_allocated_ : 1;
  TemplateSizeType allocated_size_ : 31;
#endif
};

template <typename T>
struct offsetarr_ay_view {
  offsetarr_ay_view(array<T>& arr, char const* base)
      : arr_(arr), abs_el_(reinterpret_cast<T*>(base + arr_.el_._offset)) {}

  T const* begin() const { return abs_el_; }
  T const* end() const { return abs_el_ + arr_.used_size_; }

  T* begin() { return abs_el_; }
  T* end() { return abs_el_ + arr_.used_size_; }

  inline T const& operator[](int index) const { return abs_el_[index]; }
  inline T& operator[](int index) { return abs_el_[index]; }

  T const& back() const { return abs_el_[arr_.used_size_ - 1]; }
  T& back() { return abs_el_[arr_.used_size_ - 1]; }

  T& front() { return abs_el_[0]; }
  T const& front() const { return abs_el_[0]; }

  array<T>& arr_;
  T* abs_el_;
};

template <typename T>
offsetarr_ay_view<T> make_offsetarr_ay_view(array<T>& arr, char const* base) {
  return offsetarr_ay_view<T>(arr, base);
}

template <typename T>
inline std::ostream& operator<<(std::ostream& out, array<T> const& arr) {
  bool first = true;
  out << "[";
  for (auto const& el : arr) {
    out << (first ? "" : ", ") << el;
    first = false;
  }
  return out << "]";
}

template <typename T>
inline bool operator==(array<T> const& a, array<T> const& b) {
  return a.size() == b.size() &&
         std::equal(std::begin(a), std::end(a), std::begin(b));
}

template <typename T>
inline bool operator<(array<T> const& a, array<T> const& b) {
  return std::lexicographical_compare(std::begin(a), std::end(a), std::begin(b),
                                      std::end(b));
}

template <typename T>
inline bool operator<=(array<T> const& a, array<T> const& b) {
  return !(a > b);
}

template <typename T>
inline bool operator>(array<T> const& a, array<T> const& b) {
  return b < a;
}

template <typename T>
inline bool operator>=(array<T> const& a, array<T> const& b) {
  return !(a < b);
}

using string = array<char>;
using offset_string = offsetarr_ay_view<char>;

inline std::ostream& operator<<(std::ostream& out, string const& a) {
  return out << a.el_;
}

inline std::ostream& operator<<(std::ostream& out, offset_string const& a) {
  return out << a.abs_el_;
}

inline std::string operator+(std::string& str, string const& a) {
  return str + a.el_;
}

inline std::string& operator+=(std::string& str, string const& a) {
  return str += a.el_;
}

inline void getline(std::istream& in, string& s, char delim) {
  std::string tmp;
  std::getline(in, tmp, delim);
  s = string(tmp.c_str());
}

}  // namespace motis
