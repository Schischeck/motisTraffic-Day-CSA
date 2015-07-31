#ifndef TD_MEMORY_MANAGER_H_
#define TD_MEMORY_MANAGER_H_

#include <cstdlib>
#include <memory>

namespace td
{

template<typename T>
class MemoryManager
{
public:
  MemoryManager(std::size_t size)
      : _size(size),
        _memoryBuffer(new T[_size]),
        _nextPosition(_memoryBuffer.get())
  {}

  void reset() { _nextPosition = _memoryBuffer.get(); }

  T* create()
  {
    assert(_nextPosition <= _memoryBuffer.get() + _size);
    return _nextPosition++;
  }

  std::size_t usedSize() const
  { return std::distance(_memoryBuffer.get(), _nextPosition); }

private:
  std::size_t _size;
  std::unique_ptr<T[]> _memoryBuffer;
  T* _nextPosition;
};

}  // namespace td

#endif  // TD_MEMORY_MANAGER_H_