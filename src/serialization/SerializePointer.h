#ifndef TD_SERIALIZE_POINTER_H_
#define TD_SERIALIZE_POINTER_H_

#include "serialization/Offset.h"

namespace td {

class SerializePointer
{
public:
  typedef Offset<char*>::type OffsetType;

  explicit SerializePointer(char* ptr) : _ptr(ptr), _offset(0) { }

  // Current offset
  Offset<char*>::type offset() const { return _offset; }


  // Current pointer
  char* ptr() const
  { return _ptr; }

  template <typename T>
  T* ptr() const
  { return reinterpret_cast<T*>(_ptr); }


  // Base = pointer - offset
  char* base() const
  { return _ptr - _offset; }

  template <typename T>
  T* base() const
  { return reinterpret_cast<T*>(_ptr - _offset); }


  // Absolute: base() + offsetparam
  char* absolute(OffsetType offset) const
  { return base() + offset; }

  template <typename T>
  T* absolute(OffsetType offset) const
  { return reinterpret_cast<T*>(base() + offset); }


  // Modification through operators.
  friend void operator += (SerializePointer& p, int add)
  {
    p._ptr = p._ptr + add;
    p._offset = p._offset + add;
  }

  friend SerializePointer operator + (SerializePointer const& p, int add)
  {
    SerializePointer sum = p;
    sum += add;
    return sum;
  }

private:
  // Private members: ensures that manipulation is only possible incrementing
  // both (ptr and offset) at the same time (using "+=" and "+" operators).
  char* _ptr = nullptr;
  OffsetType _offset = 0;
};

}  // namespace td

#endif  // TD_SERIALIZE_POINTER_H_