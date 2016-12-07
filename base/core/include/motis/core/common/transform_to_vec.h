#pragma once

#include <algorithm>
#include <iterator>
#include <vector>

namespace motis {

template <typename It, typename UnaryOperation>
inline auto transform_to_vec(It s, It e, UnaryOperation op)
    -> std::vector<decltype(op(*s))> {
  std::vector<decltype(op(*s))> vec(std::distance(s, e));
  std::transform(s, e, std::begin(vec), op);
  return vec;
}

template <typename Container, typename UnaryOperation>
inline auto transform_to_vec(Container const& c, UnaryOperation op)
    -> std::vector<decltype(op(*std::begin(c)))> {
  std::vector<decltype(op(*std::begin(c)))> vec(
      std::distance(std::begin(c), std::end(c)));
  std::transform(std::begin(c), std::end(c), std::begin(vec), op);
  return vec;
}

template <typename Container, typename UnaryOperation>
inline auto transform_to_vec_no_default_ctor(Container const& c,
                                             UnaryOperation op)
    -> std::vector<decltype(op(*std::begin(c)))> {
  std::vector<decltype(op(*std::begin(c)))> vec;
  vec.reserve(std::distance(std::begin(c), std::end(c)));
  std::transform(std::begin(c), std::end(c), std::back_inserter(vec), op);
  return vec;
}

}  // namespace motis
