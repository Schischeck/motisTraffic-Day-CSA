#pragma once

#include <vector>

#include "motis/module/message.h"

namespace motis {
namespace ris {
namespace detail {

template <typename T>
motis::module::msg_ptr pack_msgs(std::vector<T> const& messages) {
  motis::module::MessageCreator b;
  std::vector<flatbuffers::Offset<MessageHolder>> message_offsets;
  for (auto& message : messages) {
    message_offsets.push_back(
        CreateMessageHolder(b, b.CreateVector(message.data(), message.size())));
  }

  b.CreateAndFinish(MsgContent_RISBatch,
                    CreateRISBatch(b, b.CreateVector(message_offsets)).Union(),
                    "/ris/messages");
  return make_msg(b);
}

}  // namespace detail
}  // namespace ris
}  // namespace motis
