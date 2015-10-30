#pragma once

#include <memory>
#include <set>

#include "flatbuffers/flatbuffers.h"

#include "motis/module/message.h"

namespace motis {
namespace ris {

struct ris_message {

  ris_message(std::time_t scheduled, std::time_t timestamp,
              flatbuffers::FlatBufferBuilder&& fbb)
      : scheduled(scheduled),
        timestamp(timestamp),
        buffer_size(fbb.GetSize()),
        buffer(fbb.ReleaseBufferPointer()) {}

  // testing w/o flatbuffers
  ris_message(std::time_t scheduled, std::time_t timestamp, size_t buffer_size,
              std::unique_ptr<uint8_t, std::function<void(uint8_t*)>> buffer)
      : scheduled(scheduled),
        timestamp(timestamp),
        buffer_size(buffer_size),
        buffer(std::move(buffer)) {}

  ris_message(ris_message&&) = default;
  ris_message& operator=(ris_message&&) = default;
  ~ris_message() = default;

  std::time_t scheduled;
  std::time_t timestamp;

  size_t buffer_size;
  std::unique_ptr<uint8_t, std::function<void(uint8_t*)>> buffer;

  uint8_t const* data() const { return buffer.get(); }
  size_t size() const { return buffer_size; }
};

}  // namespace ris
}  // namespace motis
