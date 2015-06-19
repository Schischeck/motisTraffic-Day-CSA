#ifndef TD_OFFSET_H_
#define TD_OFFSET_H_

#include <cinttypes>

namespace td {

template<class t, class enable = void>
struct offset;

template<class t>
struct offset<t, typename std::enable_if<sizeof(t) == 4>::type>
{ typedef uint32_t type; };

template<class t>
struct offset<t, typename std::enable_if<sizeof(t) == 8>::type>
{ typedef uint64_t type; };

}  // namespace td

#endif  // TD_OFFSET_H_
