#pragma once

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

  row operator[](int row_index) { return {*this, row_index}; }
  const_row operator[](int row_index) const { return {*this, row_index}; }

  std::size_t column_count_;
  std::vector<T> entries_;
};

}  // namespace motis
