#pragma once

#include <cassert>
#include <cinttypes>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <ostream>
#include <type_traits>
#include <string>

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

template <typename T, typename TemplateSizeType = uint32_t>
struct array final {
  typedef TemplateSizeType size_type;

  explicit array(TemplateSizeType size = 0)
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

  template <typename It>
  array(It begin_it, It end_it)
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
    if (!_self_allocated || _el == nullptr) {
      return;
    }

    for (auto& el : *this) {
      el.~T();
    }

    std::free(_el);
    _el = nullptr;
    _used_size = 0;
    _allocated_size = 0;
    _self_allocated = 0;
  }

  T const* begin() const { return _el; }
  T const* end() const { return _el + _used_size; }
  T* begin() { return _el; }
  T* end() { return _el + _used_size; }

  inline T const& operator[](int index) const { return _el[index]; }
  inline T& operator[](int index) { return _el[index]; }

  T const& back() const { return _el[_used_size - 1]; }
  T& back() { return _el[_used_size - 1]; }

  T& front() { return _el[0]; }
  T const& front() const { return _el[0]; }

  inline TemplateSizeType size() const { return _used_size; }
  inline bool empty() const { return size() == 0; }

  array& operator=(std::string const& str) {
    return * this = array(str.c_str());
  }

  template <typename It>
  void set(It begin_it, It end_it) {
    auto range_size = std::distance(begin_it, end_it);
    reserve(range_size);

    auto copy_source = begin_it;
    auto copy_target = _el;
    for (; copy_source != end_it; ++copy_source, ++copy_target) {
      new (copy_target) T(*copy_source);
    }

    _used_size = range_size;
  }

  void push_back(T const& el) {
    reserve(_used_size + 1);
    new (_el + _used_size) T(el);
    ++_used_size;
  }

  template <typename... Args>
  void emplace_back(Args&&... el) {
    reserve(_used_size + 1);
    new (_el + _used_size) T(std::forward<Args>(el)...);
    ++_used_size;
  }

  void resize(size_type size) {
    reserve(size);
    _used_size = size;
    for (size_type i = 0; i < size; ++i) {
      new (_el + i) T();
    }
  }

  void reserve(TemplateSizeType new_size) {
    new_size = std::max(_allocated_size, new_size);

    if (_allocated_size >= new_size) {
      return;
    }

    auto next_size = next_power_of_two(new_size);
    T* mem_buf = static_cast<T*>(std::malloc(sizeof(T) * next_size));
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
        assert(false && "don't throw in the destructor or move constructor");
      }
    }

    auto free_me = _el;
    _el = mem_buf;
    if (_self_allocated) {
      std::free(free_me);
    }

    _self_allocated = true;
    _allocated_size = next_size;
  }

  T* erase(T* pos) {
    T* last = end() - 1;
    while (pos < last) {
      std::swap(*pos, *(pos + 1));
      pos = pos + 1;
    }
    pos->~T();
    --_used_size;
    return end();
  }

  std::string to_string() const { return std::string(_el); }

  operator std::string() const { return to_string(); }

#ifdef USE_STANDARD_LAYOUT
  T* _el;
  TemplateSizeType _used_size;
  TemplateSizeType _allocated_size;
  bool _self_allocated;
#else
  T* _el;
  TemplateSizeType _used_size;
  bool _self_allocated : 1;
  TemplateSizeType _allocated_size : 31;
#endif
};

template <typename T>
struct offset_array_view {
  offset_array_view(array<T>& arr, char* base)
      : _arr(arr), _abs_el(reinterpret_cast<T*>(base + _arr._el._offset)) {}

  T const* begin() const { return _abs_el; }
  T const* end() const { return _abs_el + _arr._used_size; }

  T* begin() { return _abs_el; }
  T* end() { return _abs_el + _arr._used_size; }

  inline T const& operator[](int index) const { return _abs_el[index]; }
  inline T& operator[](int index) { return _abs_el[index]; }

  T const& back() const { return _abs_el[_arr._used_size - 1]; }
  T& back() { return _abs_el[_arr._used_size - 1]; }

  T& front() { return _abs_el[0]; }
  T const& front() const { return _abs_el[0]; }

  array<T>& _arr;
  T* _abs_el;
};

template <typename T>
offset_array_view<T> make_offset_array_view(array<T>& arr, char* base) {
  return offset_array_view<T>(arr, base);
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

typedef array<char> string;
typedef offset_array_view<char> offset_string;

inline std::ostream& operator<<(std::ostream& out, string const& a) {
  return out << a._el;
}

inline std::ostream& operator<<(std::ostream& out, offset_string const& a) {
  return out << a._abs_el;
}

inline std::string operator+(std::string& str, string const& a) {
  return str + a._el;
}

inline std::string& operator+=(std::string& str, string const& a) {
  return str += a._el;
}

inline void getline(std::istream& in, string& s, char delim) {
  std::string tmp;
  std::getline(in, tmp, delim);
  s = string(tmp.c_str());
}

}  // namespace motis
