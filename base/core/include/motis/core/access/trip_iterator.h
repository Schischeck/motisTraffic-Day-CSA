#pragma once

#include <iterator>
#include <tuple>

#include "motis/core/schedule/trip.h"
#include "motis/core/access/trip_access.h"

namespace motis {
namespace access {

class trip_iterator
    : public std::iterator<std::random_access_iterator_tag, trip_section, int> {
public:
  trip_iterator(trip const* t, int const i) : trip_(t), index_(i) {}

  trip_iterator& operator+=(int rhs) {
    index_ += rhs;
    return *this;
  }
  trip_iterator& operator-=(int rhs) {
    index_ -= rhs;
    return *this;
  }
  trip_section operator*() { return {trip_, index_}; }
  trip_section operator->() { return {trip_, index_}; }
  trip_section operator[](int rhs) { return {trip_, index_}; }

  trip_iterator& operator++() {
    ++index_;
    return *this;
  }
  trip_iterator& operator--() {
    --index_;
    return *this;
  }
  trip_iterator operator++(int) {
    trip_iterator tmp(*this);
    ++index_;
    return tmp;
  }
  trip_iterator operator--(int) {
    trip_iterator tmp(*this);
    --index_;
    return tmp;
  }
  int operator-(const trip_iterator& rhs) const { return index_ - rhs.index_; }
  trip_iterator operator+(int rhs) const {
    return trip_iterator(trip_, index_ + rhs);
  }
  trip_iterator operator-(int rhs) const {
    return trip_iterator(trip_, index_ - rhs);
  }
  friend trip_iterator operator+(int lhs, trip_iterator const& rhs) {
    return trip_iterator(rhs.trip_, lhs + rhs.index_);
  }
  friend trip_iterator operator-(int lhs, trip_iterator const& rhs) {
    return trip_iterator(rhs.trip_, lhs - rhs.index_);
  }

  bool operator==(trip_iterator const& rhs) const {
    return std::tie(trip_, index_) == std::tie(rhs.trip_, rhs.index_);
  }
  bool operator!=(trip_iterator const& rhs) const {
    return std::tie(trip_, index_) != std::tie(rhs.trip_, rhs.index_);
  }
  bool operator>(trip_iterator const& rhs) const {
    return std::tie(trip_, index_) > std::tie(rhs.trip_, rhs.index_);
  }
  bool operator<(trip_iterator const& rhs) const {
    return std::tie(trip_, index_) < std::tie(rhs.trip_, rhs.index_);
  }
  bool operator>=(trip_iterator const& rhs) const {
    return std::tie(trip_, index_) >= std::tie(rhs.trip_, rhs.index_);
  }
  bool operator<=(trip_iterator const& rhs) const {
    return std::tie(trip_, index_) <= std::tie(rhs.trip_, rhs.index_);
  }

private:
  trip const* trip_;
  int index_;
};

trip_iterator begin(trip const& t) { return {&t, 0}; }
trip_iterator end(trip const& t) { return {&t, t.path->size()}; }

}  // namespace access
}  // namespace motis
