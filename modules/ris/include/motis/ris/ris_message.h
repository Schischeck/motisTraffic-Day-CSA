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

  ris_message(ris_message&&) = default;
  ris_message& operator=(ris_message&&) = default;
  ~ris_message() = default;

  std::time_t scheduled;
  std::time_t timestamp;

  std::size_t buffer_size;
  std::unique_ptr<uint8_t, std::function<void(uint8_t*)>> buffer;

  const uint8_t* data() const { return buffer.get(); }
  std::size_t size() const { return buffer_size; }
};

}  // namespace ris
}  // namespace motis
