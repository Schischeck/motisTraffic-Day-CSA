#pragma once

#include <vector>

#include "motis/module/message.h"
#include "motis/ris/blob.h"
#include "motis/ris/ris_message.h"

namespace motis {
namespace ris {
namespace detail {

inline motis::module::msg_ptr pack_msgs(
    motis::module::message_creator& b,
    std::vector<flatbuffers::Offset<MessageHolder>> const& offsets) {
  b.create_and_finish(MsgContent_RISBatch,
                      CreateRISBatch(b, b.CreateVector(offsets)).Union(),
                      "/ris/messages");
  return make_msg(b);
}

inline motis::module::msg_ptr pack_msgs(std::vector<ris_message> const& msgs) {
  motis::module::message_creator b;
  std::vector<flatbuffers::Offset<MessageHolder>> offsets;
  for (auto& msg : msgs) {
    offsets.push_back(
        CreateMessageHolder(b, b.CreateVector(msg.data(), msg.size())));
  }
  return pack_msgs(b, offsets);
}

inline motis::module::msg_ptr pack_msgs(
    std::vector<std::pair<std::time_t, blob>> const& msgs) {
  motis::module::message_creator b;
  std::vector<flatbuffers::Offset<MessageHolder>> offsets;
  for (auto& msg : msgs) {
    offsets.push_back(CreateMessageHolder(
        b, b.CreateVector(msg.second.data(), msg.second.size())));
  }
  return pack_msgs(b, offsets);
}

}  // namespace detail
}  // namespace ris
}  // namespace motis
