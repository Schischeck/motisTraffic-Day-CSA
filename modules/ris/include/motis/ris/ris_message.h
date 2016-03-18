#pragma once

#include <memory>
#include <set>

#include "flatbuffers/flatbuffers.h"

#include "motis/protocol/RISMessage_generated.h"

#include "motis/core/common/typed_flatbuffer.h"

namespace motis {
namespace ris {

struct ris_message : typed_flatbuffer<Message> {
  ris_message(std::time_t scheduled, std::time_t timestamp,
              flatbuffers::FlatBufferBuilder&& fbb)
      : typed_flatbuffer(std::move(fbb)),
        scheduled(scheduled),
        timestamp(timestamp) {}

  // testing w/o flatbuffers
  ris_message(std::time_t scheduled, std::time_t timestamp, size_t buffer_size,
              std::unique_ptr<uint8_t, std::function<void(uint8_t*)>>&& buffer)
      : typed_flatbuffer(buffer_size, std::move(buffer)),
        scheduled(scheduled),
        timestamp(timestamp) {}

  ris_message(ris_message&&) = default;
  ris_message& operator=(ris_message&&) = default;

  std::time_t scheduled;
  std::time_t timestamp;
};

}  // namespace ris
}  // namespace motis
