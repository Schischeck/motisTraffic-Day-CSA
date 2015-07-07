#pragma once

#include "motis/core/common/offset.h"

namespace motis {

template <typename T>
union pointer final {
  pointer() = default;
  pointer(typename offset<T*>::type offset) : _offset(offset) {}
  pointer(T* ptr) : _ptr(ptr) {}

  inline T* operator->() { return _ptr; }
  inline T const* operator->() const { return _ptr; }
  inline T& operator*() { return *_ptr; }
  inline T const& operator*() const { return *_ptr; }
  inline operator T*() { return _ptr; }
  inline operator T const*() const { return _ptr; }

  T const* ptr() const { return _ptr; }
  T* ptr() { return _ptr; }

  typename offset<T*>::type get_offset() const { return _offset; }

  void offset_to_pointer(char* base) {
    if (_offset != 0) _ptr = reinterpret_cast<T*>(base + _offset);
  }

  T* _ptr;
  typename offset<T*>::type _offset;
};

}  // namespace motis
