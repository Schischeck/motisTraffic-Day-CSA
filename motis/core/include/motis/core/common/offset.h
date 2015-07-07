#pragma once

#include <cinttypes>

namespace motis {

template <class t, class enable = void>
struct offset;

template <class t>
struct offset<t, typename std::enable_if<sizeof(t) == 4>::type> {
  typedef uint32_t type;
};

template <class t>
struct offset<t, typename std::enable_if<sizeof(t) == 8>::type> {
  typedef uint64_t type;
};

}  // namespace motis
