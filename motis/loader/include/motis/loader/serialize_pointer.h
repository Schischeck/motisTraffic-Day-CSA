#ifndef TD_SERIALIZE_POINTER_H_
#define TD_SERIALIZE_POINTER_H_

#include "motis/core/common/offset.h"

namespace td {

class serialize_pointer {
public:
  typedef offset<char*>::type offset_type;

  explicit serialize_pointer(char* ptr) : _ptr(ptr), _offset(0) {}

  // current offset
  offset<char*>::type offset() const { return _offset; }

  // current pointer
  char* ptr() const { return _ptr; }

  template <typename T>
  T* ptr() const {
    return reinterpret_cast<T*>(_ptr);
  }

  // base = pointer - offset
  char* base() const { return _ptr - _offset; }

  template <typename T>
  T* base() const {
    return reinterpret_cast<T*>(_ptr - _offset);
  }

  // absolute: base() + offsetparam
  char* absolute(offset_type offset) const { return base() + offset; }

  template <typename T>
  T* absolute(offset_type o) const {
    return reinterpret_cast<T*>(base() + o);
  }

  // modification through operators.
  friend void operator+=(serialize_pointer& p, int add) {
    p._ptr = p._ptr + add;
    p._offset = p._offset + add;
  }

  friend serialize_pointer operator+(serialize_pointer const& p, int add) {
    serialize_pointer sum = p;
    sum += add;
    return sum;
  }

private:
  // private members: ensures that manipulation is only possible incrementing
  // both (ptr and offset) at the same time (using "+=" and "+" operators).
  char* _ptr = nullptr;
  offset_type _offset = 0;
};

}  // namespace td

#endif  // TD_SERIALIZE_POINTER_H_
