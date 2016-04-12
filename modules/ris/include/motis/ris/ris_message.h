#pragma once

#include <memory>
#include <set>

#include "flatbuffers/flatbuffers.h"

#include "motis/protocol/RISMessage_generated.h"

#include "motis/core/common/typed_flatbuffer.h"

namespace motis {
namespace ris {

struct ris_message : typed_flatbuffer<Message> {
  ris_message(std::time_t earliest, std::time_t latest, std::time_t timestamp,
              flatbuffers::FlatBufferBuilder&& fbb)
      : typed_flatbuffer(std::move(fbb)),
        earliest_(earliest),
        latest_(latest),
        timestamp_(timestamp) {}

  // testing w/o flatbuffers
  ris_message(std::time_t earliest, std::time_t latest, std::time_t timestamp,
              std::string const& msg)
      : typed_flatbuffer(msg),
        earliest_(earliest),
        latest_(latest),
        timestamp_(timestamp) {}

  std::time_t earliest_;
  std::time_t latest_;
  std::time_t timestamp_;
};

}  // namespace ris
}  // namespace motis
