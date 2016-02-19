#pragma once

#include <vector>

#include "motis/core/schedule/schedule.h"
#include "motis/protocol/Message_generated.h"

namespace motis {
namespace lookup {

flatbuffers::Offset<routing::Connection> lookup_id_train(
    flatbuffers::FlatBufferBuilder&, schedule const&, IdEvent const*);

}  // namespace lookup
}  // namespace motis
