#pragma once

#include <vector>

#include "motis/core/schedule/delay_info.h"
#include "motis/core/schedule/schedule.h"
#include "motis/module/message.h"

namespace motis {
namespace rt {

struct shifted_nodes_msg_builder {
  explicit shifted_nodes_msg_builder(schedule const&);

  void add_shifted_node(delay_info const& di);
  motis::module::msg_ptr finish();
  bool empty() const;

private:
  schedule const& sched_;
  motis::module::message_creator fbb_;
  std::vector<flatbuffers::Offset<ShiftedNode>> nodes_;
};

}  // namespace rt
}  // namespace motis
