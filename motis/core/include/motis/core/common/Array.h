#pragma once

#include <cassert>
#include <cinttypes>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <ostream>
#include <type_traits>
#include <string>

#include "motis/core/common/pointer.h"

namespace motis {

inline uint64_t next_power_of_two(uint64_t n) {
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n |= n >> 32;
  n++;
  return n;
}

template <typename t, typename template_size_type = uint32_t>
struct array final {
  typedef template_size_type size_type;

  explicit array(template_size_type size = 0)
      : _el(nullptr),
        _used_size(0),
        _self_allocated(false),
        _allocated_size(0) {
    resize(size);
  }

  explicit array(const char* str)
      : _el(nullptr),
        _used_size(0),
        _self_allocated(false),
        _allocated_size(0) {
    auto length = std::strlen(str) + 1;
    reserve(length);
    std::memcpy(_el, str, length);
    _used_size = length;
  }

  template <typename it>
  array(it begin_it, it end_it)
      : _el(nullptr),
        _used_size(0),
        _self_allocated(false),
        _allocated_size(0) {
    set(begin_it, end_it);
  }

  array(array&& arr)
      : _el(arr._el),
        _used_size(arr._used_size),
        _self_allocated(arr._self_allocated),
        _allocated_size(arr._allocated_size) {
    arr._el = nullptr;
    arr._used_size = 0;
    arr._self_allocated = false;
    arr._allocated_size = 0;
  }

  array(array const& arr)
      : _el(nullptr),
        _used_size(0),
        _self_allocated(false),
        _allocated_size(0) {
    set(std::begin(arr), std::end(arr));
  }

  array& operator=(array&& arr) {
    deallocate();

    _el = arr._el;
    _used_size = arr._used_size;
    _self_allocated = arr._self_allocated;
    _allocated_size = arr._allocated_size;

    arr._el = nullptr;
    arr._used_size = 0;
    arr._self_allocated = false;
    arr._allocated_size = 0;

    return *this;
  }

  array& operator=(array const& arr) {
    set(std::begin(arr), std::end(arr));
    return *this;
  }

  ~array() { deallocate(); }

  void deallocate() {
    if (!_self_allocated || _el == nullptr) return;

    for (auto& el : *this) el.~t();

    std::free(_el);
    _el = nullptr;
    _used_size = 0;
    _allocated_size = 0;
    _self_allocated = 0;
  }

  t const* begin() const { return _el; }
  t const* end() const { return _el + _used_size; }
  t* begin() { return _el; }
  t* end() { return _el + _used_size; }

  inline t const& operator[](int index) const { return _el[index]; }
  inline t& operator[](int index) { return _el[index]; }

  t const& back() const { return _el[_used_size - 1]; }
  t& back() { return _el[_used_size - 1]; }

  t& front() { return _el[0]; }
  t const& front() const { return _el[0]; }

  inline template_size_type size() const { return _used_size; }
  inline bool empty() const { return size() == 0; }

  array& operator=(std::string const& str) {
    return * this = array(str.c_str());
  }

  template <typename it>
  void set(it begin_it, it end_it) {
    auto range_size = std::distance(begin_it, end_it);
    reserve(range_size);

    auto copy_source = begin_it;
    auto copy_target = _el.ptr();
    for (; copy_source != end_it; ++copy_source, ++copy_target)
      new (copy_target) t(*copy_source);

    _used_size = range_size;
  }

  void push_back(t const& el) {
    reserve(_used_size + 1);
    new (_el + _used_size) t(el);
    ++_used_size;
  }

  template <typename... args>
  void emplace_back(args&&... el) {
    reserve(_used_size + 1);
    new (_el + _used_size) t(std::forward<args>(el)...);
    ++_used_size;
  }

  void resize(size_type size) {
    reserve(size);
    _used_size = size;
    for (size_type i = 0; i < size; ++i) new (_el + i) t();
  }

  void reserve(template_size_type new_size) {
    new_size = std::max(_allocated_size, new_size);

    if (_allocated_size >= new_size) return;

    auto next_size = next_power_of_two(new_size);
    t* mem_buf = static_cast<t*>(std::malloc(sizeof(t) * next_size));
    if (mem_buf == nullptr) throw std::bad_alloc();

    if (size() != 0) {
      try {
        auto move_target = mem_buf;
        for (auto& el : *this) new (move_target++) t(std::move(el));

        for (auto& el : *this) el.~t();
      } catch (...) {
        assert(false && "don't throw in the destructor or move constructor");
      }
    }

    auto free_me = _el;
    _el = mem_buf;
    if (_self_allocated) std::free(free_me);

    _self_allocated = true;
    _allocated_size = next_size;
  }

  std::string to_string() const { return std::string(_el._ptr); }

  operator std::string() const { return to_string(); }

#ifdef USE_STANDARD_LAYOUT
  pointer<t> _el;
  template_size_type _used_size;
  template_size_type _allocated_size;
  bool _self_allocated;
#else
  pointer<t> _el;
  template_size_type _used_size;
  bool _self_allocated : 1;
  template_size_type _allocated_size : 31;
#endif
};

template <typename t>
struct offset_array_view {
  offset_array_view(array<t>& arr, char* base)
      : _arr(arr), _abs_el(reinterpret_cast<t*>(base + _arr._el._offset)) {}

  t const* begin() const { return _abs_el; }
  t const* end() const { return _abs_el + _arr._used_size; }

  t* begin() { return _abs_el; }
  t* end() { return _abs_el + _arr._used_size; }

  inline t const& operator[](int index) const { return _abs_el[index]; }
  inline t& operator[](int index) { return _abs_el[index]; }

  t const& back() const { return _abs_el[_arr._used_size - 1]; }
  t& back() { return _abs_el[_arr._used_size - 1]; }

  t& front() { return _abs_el[0]; }
  t const& front() const { return _abs_el[0]; }

  array<t>& _arr;
  t* _abs_el;
};

template <typename t>
offset_array_view<t> make_offset_array_view(array<t>& arr, char* base) {
  return offset_array_view<t>(arr, base);
}

template <typename t>
inline std::ostream& operator<<(std::ostream& out, array<t> const& arr) {
  bool first = true;
  out << "[";
  for (auto const& el : arr) {
    out << (first ? "" : ", ") << el;
    first = false;
  }
  return out << "]";
}

template <typename t>
inline bool operator==(array<t> const& a, array<t> const& b) {
  return a.size() == b.size() &&
         std::equal(std::begin(a), std::end(a), std::begin(b));
}

template <typename t>
inline bool operator<(array<t> const& a, array<t> const& b) {
  return std::lexicographical_compare(std::begin(a), std::end(a), std::begin(b),
                                      std::end(b));
}

template <typename t>
inline bool operator<=(array<t> const& a, array<t> const& b) {
  return !(a > b);
}

template <typename t>
inline bool operator>(array<t> const& a, array<t> const& b) {
  return b < a;
}

template <typename t>
inline bool operator>=(array<t> const& a, array<t> const& b) {
  return !(a < b);
}

typedef array<char> string;
typedef offset_array_view<char> offset_string;

inline std::ostream& operator<<(std::ostream& out, string const& a) {
  return out << a._el;
}

inline std::ostream& operator<<(std::ostream& out, offset_string const& a) {
  return out << a._abs_el;
}

inline std::string operator+(std::string& str, string const& a) {
  return str + a._el._ptr;
}

inline std::string& operator+=(std::string& str, string const& a) {
  return str += a._el._ptr;
}

inline void getline(std::istream& in, string& s, char delim) {
  std::string tmp;
  std::getline(in, tmp, delim);
  s = string(tmp.c_str());
}

}  // namespace motis
