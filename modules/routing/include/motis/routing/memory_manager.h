#ifndef TD_MEMORY_MANAGER_H_
#define TD_MEMORY_MANAGER_H_

#include <cstdlib>
#include <memory>

namespace td
{

template<typename t>
class memory_manager
{
public:
  memory_manager(std::size_t size)
      : _size(size),
        _memory_buffer(new t[_size]),
        _next_position(_memory_buffer.get())
  {}

  void reset() { _next_position = _memory_buffer.get(); }

  t* create()
  {
    assert(_next_position <= _memory_buffer.get() + _size);
    return _next_position++;
  }

  std::size_t used_size() const
  { return std::distance(_memory_buffer.get(), _next_position); }

private:
  std::size_t _size;
  std::unique_ptr<t[]> _memory_buffer;
  t* _next_position;
};

}  // namespace td

#endif  // TD_MEMORY_MANAGER_H_
