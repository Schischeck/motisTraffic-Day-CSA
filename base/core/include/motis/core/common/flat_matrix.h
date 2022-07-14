#pragma once

#include <cinttypes>

#include <vector>

namespace motis {

template <typename T>
struct flat_matrix {
  struct row {
    row(flat_matrix& matrix, int row_index)
        : matrix_(matrix), row_index_(row_index) {}

    T& operator[](int column_index) {
      auto pos = matrix_.column_count_ * row_index_ + column_index;
      return matrix_.entries_[pos];
    }

    flat_matrix& matrix_;
    int row_index_;
  };

  struct const_row {
    const_row(flat_matrix const& matrix, int row_index)
        : matrix_(matrix), row_index_(row_index) {}

    T const& operator[](int column_index) const {
      auto pos = matrix_.column_count_ * row_index_ + column_index;
      return matrix_.entries_[pos];
    }

    flat_matrix const& matrix_;
    int row_index_;
  };

  flat_matrix() : column_count_{0} {}

  explicit flat_matrix(int column_count)
      : column_count_(column_count), entries_(column_count * column_count) {}

  flat_matrix(std::size_t const column_count, std::vector<T> const& entries)
      : column_count_(column_count), entries_(entries) {}

  flat_matrix(std::size_t const column_count, std::vector<T>&& entries)
      : column_count_(column_count), entries_(entries) {}

  row operator[](int row_index) { return {*this, row_index}; }
  const_row operator[](int row_index) const { return {*this, row_index}; }

  T& operator()(int const row_index, int const column_index) {
    return entries_[column_count_ * row_index + column_index];
  }

  std::size_t column_count_;
  std::vector<T> entries_;
};

template <typename T>
inline flat_matrix<T> make_flat_matrix(uint32_t const column_count,
                                       T const& init = T{}) {
  auto v = std::vector<T>{};
  v.resize(column_count * column_count, init);
  return flat_matrix<T>{column_count, std::move(v)};
}

}  // namespace motis
