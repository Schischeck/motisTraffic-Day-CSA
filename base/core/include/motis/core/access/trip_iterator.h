#pragma once

#include <iterator>
#include <tuple>

#include "motis/core/schedule/trip.h"
#include "motis/core/access/trip_section.h"
#include "motis/core/access/trip_stop.h"

namespace motis {
namespace access {

template <typename T>
class trip_iterator
    : public std::iterator<std::random_access_iterator_tag, T, int> {
public:
  trip_iterator(trip const* t, int const i) : trip_(t), index_(i) {}

  trip_iterator<T>& operator+=(int rhs) {
    index_ += rhs;
    return *this;
  }
  trip_iterator<T>& operator-=(int rhs) {
    index_ -= rhs;
    return *this;
  }

  T operator*() { return {trip_, index_}; }
  T operator[](int rhs) { return {trip_, rhs}; }

  trip_iterator<T>& operator++() {
    ++index_;
    return *this;
  }
  trip_iterator<T>& operator--() {
    --index_;
    return *this;
  }
  trip_iterator<T> operator++(int) {
    trip_iterator<T> tmp(*this);
    ++index_;
    return tmp;
  }
  trip_iterator<T> operator--(int) {
    trip_iterator<T> tmp(*this);
    --index_;
    return tmp;
  }
  int operator-(const trip_iterator<T>& rhs) const {
    return index_ - rhs.index_;
  }
  trip_iterator<T> operator+(int rhs) const {
    return trip_iterator<T>(trip_, index_ + rhs);
  }
  trip_iterator<T> operator-(int rhs) const {
    return trip_iterator<T>(trip_, index_ - rhs);
  }
  friend trip_iterator<T> operator+(int lhs, trip_iterator<T> const& rhs) {
    return trip_iterator<T>(rhs.trip_, lhs + rhs.index_);
  }
  friend trip_iterator<T> operator-(int lhs, trip_iterator<T> const& rhs) {
    return trip_iterator<T>(rhs.trip_, lhs - rhs.index_);
  }

  bool operator==(trip_iterator<T> const& rhs) const {
    return std::tie(trip_, index_) == std::tie(rhs.trip_, rhs.index_);
  }
  bool operator!=(trip_iterator<T> const& rhs) const {
    return std::tie(trip_, index_) != std::tie(rhs.trip_, rhs.index_);
  }
  bool operator>(trip_iterator<T> const& rhs) const {
    return std::tie(trip_, index_) > std::tie(rhs.trip_, rhs.index_);
  }
  bool operator<(trip_iterator<T> const& rhs) const {
    return std::tie(trip_, index_) < std::tie(rhs.trip_, rhs.index_);
  }
  bool operator>=(trip_iterator<T> const& rhs) const {
    return std::tie(trip_, index_) >= std::tie(rhs.trip_, rhs.index_);
  }
  bool operator<=(trip_iterator<T> const& rhs) const {
    return std::tie(trip_, index_) <= std::tie(rhs.trip_, rhs.index_);
  }

protected:
  trip const* trip_;
  int index_;
};

struct sections {
  using iterator = trip_iterator<trip_section>;

  explicit sections(trip const* t) : t_(t) {}

  iterator begin() const { return {t_, 0}; }
  iterator end() const { return {t_, static_cast<int>(t_->edges_->size())}; }

  friend iterator begin(sections const& s) { return s.begin(); }
  friend iterator end(sections const& s) { return s.end(); }

  trip const* t_;
};

struct stops {
  using iterator = trip_iterator<trip_stop>;

  explicit stops(trip const* t) : t_(t) {}

  iterator begin() const { return {t_, 0}; }
  iterator end() const { return {t_, static_cast<int>(t_->edges_->size()) + 1}; }

  friend iterator begin(stops const& s) { return s.begin(); }
  friend iterator end(stops const& s) { return s.end(); }

  trip const* t_;
};

}  // namespace access
}  // namespace motis
