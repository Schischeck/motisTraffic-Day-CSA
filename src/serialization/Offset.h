#ifndef TD_OFFSET_H_
#define TD_OFFSET_H_

#include <cinttypes>

namespace td {

template<class T, class Enable = void>
struct Offset;

template<class T>
struct Offset<T, typename std::enable_if<sizeof(T) == 4>::type>
{ typedef uint32_t type; };

template<class T>
struct Offset<T, typename std::enable_if<sizeof(T) == 8>::type>
{ typedef uint64_t type; };

}  // namespace td

#endif  // TD_OFFSET_H_