#pragma once

#include "flatbuffers/flatbuffers.h"

namespace motis {

template <typename T>
struct typed_flatbuffer {
  typed_flatbuffer(flatbuffers::FlatBufferBuilder&& fbb)
      : buffer_size_(fbb.GetSize()), buffer_(fbb.ReleaseBufferPointer()) {}

  typed_flatbuffer(size_t buffer_size, flatbuffers::unique_ptr_t buffer)
      : buffer_size_(buffer_size), buffer_(std::move(buffer)) {}

  typed_flatbuffer(typed_flatbuffer&&) = default;
  typed_flatbuffer& operator=(typed_flatbuffer&&) = default;

  virtual ~typed_flatbuffer() {}

  uint8_t const* data() const { return buffer_.get(); }
  size_t size() const { return buffer_size_; }

  T* get() const  { return flatbuffers::GetMutableRoot<T>(buffer_.get()); };

  size_t buffer_size_;
  flatbuffers::unique_ptr_t buffer_;
};

}  // namespace motis