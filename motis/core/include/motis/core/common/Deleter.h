#ifndef TD_SERIALIZATION_H_
#define TD_SERIALIZATION_H_

namespace td {

template<typename T>
struct deleter
{
  deleter(bool active) : _active(active) {}
  void operator () (T* ptr) { if (_active) delete ptr; }
  bool _active;
};

}  // namespace td

#endif  // TD_SERIALIZATION_H_
