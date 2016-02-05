#ifndef IMOTIS_EVAL_QUANTILE_H_
#define IMOTIS_EVAL_QUANTILE_H_

#include <algorithm>
#include <type_traits>
#include <cmath>

namespace motis {
namespace eval {

// From http://stackoverflow.com/a/10642935
template <typename T>
struct MemberTypeHelper;
template <typename R, typename T>
struct MemberTypeHelper<R(T::*)> {
  typedef R Type;
  typedef T ParentType;
};
template <typename T>
struct MemberType : public MemberTypeHelper<T> {};

template <typename Attr, typename It>
It quantile_it(Attr attr, It begin, It end, double q) {
  typedef typename std::remove_reference<
      typename std::remove_cv<decltype(*begin)>::type>::type val_type;
  std::sort(begin, end, [&attr](val_type const& r1, val_type const& r2) {
    return r1.*attr < r2.*attr;
  });
  return begin + std::round(q * std::distance(begin, end));
}

template <typename Attr, typename It>
typename MemberType<Attr>::Type quantile(Attr attr, It begin, It end,
                                         double q) {
  return *(quantile_it(attr, begin, end, q)).*attr;
}

template <typename Attr, typename Col>
typename MemberType<Attr>::Type quantile(Attr attr, Col col, double q) {
  return quantile(attr, std::begin(col), std::end(col), q);
}

}  // namespace eval
}  // namespace motis

#endif  // IMOTIS_EVAL_QUANTILE_H_