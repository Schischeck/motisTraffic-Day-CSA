#pragma once

#include <utility>

#define MOTIS_FINALLY(fun) auto finally##__LINE__ = make_finally(fun);

namespace motis {

template <typename T, typename DestructFun>
struct raii {
  raii(T&& el, DestructFun&& destruct)
      : el_(std::forward<T>(el)),
        destruct_(std::forward<DestructFun>(destruct)),
        omit_destruct_(false) {}

  ~raii() {
    if (!omit_destruct_) {
      destruct_(el_);
    }
  }

  T& get() { return el_; }
  operator T() { return el_; }

  T el_;
  DestructFun destruct_;
  bool omit_destruct_;
};

template <typename T>
struct resetter {
  resetter(T& value, T new_value) : value_(value) {
    old_value_ = std::forward<T>(value_);
    value_ = std::forward<T>(new_value);
  }
  ~resetter() { value_ = std::forward<T>(old_value_); }
  T& value_;
  T old_value_;
};

template <typename T, typename DestructFun>
raii<T, DestructFun> make_raii(T&& el, DestructFun&& destruct) {
  return {std::forward<T>(el), std::forward<DestructFun>(destruct)};
};

template <typename DestructFun>
struct finally {
  finally(DestructFun&& destruct)
      : destruct_(std::forward<DestructFun>(destruct)) {}
  ~finally() { destruct_(); }
  DestructFun destruct_;
};

template <typename DestructFun>
finally<DestructFun> make_finally(DestructFun&& destruct) {
  return {std::forward<DestructFun>(destruct)};
};

}  // namespace motis
