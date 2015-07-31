#ifndef TD_ARRAY_H_
#define TD_ARRAY_H_

#include <cassert>
#include <cinttypes>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <ostream>
#include <type_traits>
#include <string>

#include "motis/core/common/Pointer.h"

namespace td {

template <typename IntType>
constexpr IntType shiftLeft(IntType i, std::size_t shift) { return i << shift; }

template <typename IntType, IntType Shift>
inline
typename std::enable_if<Shift == shiftLeft(1, sizeof(IntType)), IntType>::type
  nextPowerOfTwoMinusOne(IntType n)
{ return n | n >> Shift; }

template <typename IntType, IntType Shift>
inline
typename std::enable_if<Shift != shiftLeft(1, sizeof(IntType)), IntType>::type
  nextPowerOfTwoMinusOne(IntType n)
{
  auto shifted = n >> Shift;
  return shifted |
         nextPowerOfTwoMinusOne<IntType, shiftLeft(Shift, 1)>(n | shifted);
}

template <typename IntType>
inline IntType nextPowerOfTwo(IntType n)
{ return 1 + nextPowerOfTwoMinusOne<IntType, 1>(n - 1); }


template <typename T, typename TemplateSizeType = uint32_t>
struct Array final
{
  typedef TemplateSizeType SizeType;

  explicit
  Array(TemplateSizeType size = 0)
      : _el(nullptr),
        _usedSize(0),
        _selfAllocated(false),
        _allocatedSize(0)
  { resize(size); }

  explicit
  Array(const char* str)
      : _el(nullptr),
        _usedSize(0),
        _selfAllocated(false),
        _allocatedSize(0)
  {
    auto length = std::strlen(str) + 1;
    reserve(length);
    std::memcpy(_el, str, length);
    _usedSize = length;
  }

  template<typename It>
  Array(It beginIt, It endIt)
      : _el(nullptr),
        _usedSize(0),
        _selfAllocated(false),
        _allocatedSize(0)
  { set(beginIt, endIt); }

  Array(Array&& arr)
      : _el(arr._el),
        _usedSize(arr._usedSize),
        _selfAllocated(arr._selfAllocated),
        _allocatedSize(arr._allocatedSize)
  {
    arr._el = nullptr;
    arr._usedSize = 0;
    arr._selfAllocated = false;
    arr._allocatedSize = 0;
  }

  Array(Array const& arr)
      : _el(nullptr),
        _usedSize(0),
        _selfAllocated(false),
        _allocatedSize(0)
  { set(std::begin(arr), std::end(arr)); }

  Array& operator = (Array&& arr)
  {
    deallocate();

    _el = arr._el;
    _usedSize = arr._usedSize;
    _selfAllocated = arr._selfAllocated;
    _allocatedSize = arr._allocatedSize;

    arr._el = nullptr;
    arr._usedSize = 0;
    arr._selfAllocated = false;
    arr._allocatedSize = 0;

    return *this;
  }

  Array& operator = (Array const& arr)
  {
    set(std::begin(arr), std::end(arr));
    return *this;
  }

  ~Array()
  { deallocate(); }

  void deallocate()
  {
    if (!_selfAllocated || _el == nullptr)
      return;

    for (auto& el : *this)
      el.~T();

    std::free(_el);
    _el = nullptr;
    _usedSize = 0;
    _allocatedSize = 0;
    _selfAllocated = 0;
  }

  T const* begin() const { return _el; }
  T const* end()   const { return _el + _usedSize; }
  T* begin() { return _el; }
  T* end()   { return _el + _usedSize; }

  inline T const& operator [] (int index) const { return _el[index]; }
  inline T& operator [] (int index) { return _el[index]; }

  T const& back() const { return _el[_usedSize - 1]; }
  T& back() { return _el[_usedSize - 1]; }

  T& front() { return _el[0]; }
  T const& front() const { return _el[0]; }

  inline TemplateSizeType size() const { return _usedSize; }
  inline bool empty() const { return size() == 0; }

  Array& operator = (std::string const& str)
  { return *this = Array(str.c_str()); }

  template<typename It>
  void set(It beginIt, It endIt)
  {
    auto rangeSize = std::distance(beginIt, endIt);
    reserve(rangeSize);

    auto copySource = beginIt;
    auto copyTarget = _el.ptr();
    for (; copySource != endIt; ++copySource, ++copyTarget)
      new (copyTarget) T(*copySource);

    _usedSize = rangeSize;
  }

  void push_back(T const& el)
  {
    reserve(_usedSize + 1);
    new (_el + _usedSize) T(el);
    ++_usedSize;
  }

  template<typename... Args>
  void emplace_back(Args&&... el)
  {
    reserve(_usedSize + 1);
    new (_el + _usedSize) T(std::forward<Args>(el)...);
    ++_usedSize;
  }

  void resize(SizeType size)
  {
    reserve(size);
    _usedSize = size;
    for (SizeType i = 0; i < size; ++i)
      new (_el + i) T();
  }

  void reserve(TemplateSizeType newSize)
  {
    newSize = std::max(_allocatedSize, newSize);

    if (_allocatedSize >= newSize)
      return;

    auto nextSize = nextPowerOfTwo(newSize);
    T* memBuf = static_cast<T*>(std::malloc(sizeof(T) * nextSize));
    if (memBuf == nullptr)
      throw std::bad_alloc();

    if (size() != 0)
    {
      try
      {
        auto moveTarget = memBuf;
        for (auto& el : *this)
          new (moveTarget++) T(std::move(el));

        for (auto& el : *this)
          el.~T();
      }
      catch (...)
      {
        assert(false && "don't throw in the destructor or move constructor");
      }
    }

    auto freeMe = _el;
    _el = memBuf;
    if (_selfAllocated)
      std::free(freeMe);

    _selfAllocated = true;
    _allocatedSize = nextSize;
  }

  std::string toString() const
  { return std::string(_el._ptr); }

  operator std::string () const
  { return toString(); }

#ifdef USE_STANDARD_LAYOUT
  Pointer<T> _el;
  TemplateSizeType _usedSize;
  TemplateSizeType _allocatedSize;
  bool _selfAllocated;
#else
  Pointer<T> _el;
  TemplateSizeType _usedSize;
  bool _selfAllocated : 1;
  TemplateSizeType _allocatedSize : 31;
#endif
};

template<typename T>
struct OffsetArrayView {
  OffsetArrayView(Array<T>& arr, char* base)
      : _arr(arr),
        _absEl(reinterpret_cast<T*>(base + _arr._el._offset))
  {}

  T const* begin() const { return _absEl; }
  T const* end()   const { return _absEl + _arr._usedSize; }

  T* begin() { return _absEl; }
  T* end()   { return _absEl + _arr._usedSize; }

  inline T const& operator [] (int index) const { return _absEl[index]; }
  inline T& operator [] (int index) { return _absEl[index]; }

  T const& back() const { return _absEl[_arr._usedSize - 1]; }
  T& back() { return _absEl[_arr._usedSize - 1]; }

  T& front() { return _absEl[0]; }
  T const& front() const { return _absEl[0]; }

  Array<T>& _arr;
  T* _absEl;
};

template<typename T>
OffsetArrayView<T> makeOffsetArrayView(Array<T>& arr, char* base)
{ return OffsetArrayView<T>(arr, base); }

template<typename T>
inline std::ostream& operator << (std::ostream& out, Array<T> const& arr)
{
  bool first = true;
  out << "[";
  for (auto const& el : arr) {
    out << (first ? "" : ", ") << el;
    first = false;
  }
  return out << "]";
}

template<typename T>
inline bool operator == (Array<T> const& a, Array<T> const& b)
{
  return a.size() == b.size() &&
         std::equal(std::begin(a), std::end(a), std::begin(b));
}

template<typename T>
inline bool operator < (Array<T> const& a, Array<T> const& b)
{
  return std::lexicographical_compare(std::begin(a), std::end(a),
                                      std::begin(b), std::end(b));
}

template<typename T>
inline bool operator <= (Array<T> const& a, Array<T> const& b)
{ return !(a > b); }

template<typename T>
inline bool operator > (Array<T> const& a, Array<T> const& b)
{ return b < a; }

template<typename T>
inline bool operator >= (Array<T> const& a, Array<T> const& b)
{ return !(a < b); }


typedef Array<char> String;
typedef OffsetArrayView<char> OffsetString;

inline std::ostream& operator << (std::ostream& out, String const& a)
{ return out << a._el; }

inline std::ostream& operator << (std::ostream& out, OffsetString const& a)
{ return out << a._absEl; }

inline std::string operator + (std::string& str, String const& a)
{ return str +  a._el._ptr; }

inline std::string& operator += (std::string& str, String const& a)
{ return str +=  a._el._ptr; }

inline void getline(std::istream& in, String& s, char delim)
{
  std::string tmp;
  std::getline(in, tmp, delim);
  s = String(tmp.c_str());
}

}  // namespace td

#endif  // TD_ARRAY_H_
