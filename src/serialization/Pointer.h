#ifndef TD_SERIALIZATION_POINTER_H_
#define TD_SERIALIZATION_POINTER_H_

#include "serialization/Offset.h"

namespace td {

template <typename T>
union Pointer final
{
  Pointer() = default;
  Pointer(typename Offset<T*>::type offset) : _offset(offset) {}
  Pointer(T* ptr) : _ptr(ptr) {}

  inline T* operator -> () { return _ptr; }
  inline T const* operator -> () const { return _ptr; }
  inline T& operator * () { return *_ptr; }
  inline T const& operator * () const { return *_ptr; }
  inline operator T* () { return _ptr; }
  inline operator T const* () const { return _ptr; }

  T const* ptr() const { return _ptr; }
  T* ptr() { return _ptr; }

  typename Offset<T*>::type offset() const { return _offset; }

  void offsetToPointer(char* base)
  {
    if (_offset != 0)
      _ptr = reinterpret_cast<T*>(base + _offset);
  }

  T* _ptr;
  typename Offset<T*>::type _offset;
};

}  // namespace td

#endif  // TD_SERIALIZATION_POINTER_H_
