#pragma once

#include <vector>

#include "utl/to_vec.h"

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
  return pack_msgs(b, utl::to_vec(msgs, [&](auto&& msg) {
                     return CreateMessageHolder(
                         b, b.CreateVector(msg.data(), msg.size()));
                   }));
}

inline motis::module::msg_ptr pack_msgs(
    std::vector<std::pair<std::time_t, blob>> const& msgs) {
  motis::module::message_creator b;
  return pack_msgs(b, utl::to_vec(msgs, [&](auto&& msg) {
                     return CreateMessageHolder(
                         b,
                         b.CreateVector(msg.second.data(), msg.second.size()));
                   }));
}

}  // namespace detail
}  // namespace ris
}  // namespace motis
