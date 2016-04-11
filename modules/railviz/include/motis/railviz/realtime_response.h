#pragma once

#include "motis/module/module.h"
#include "motis/protocol/Message_generated.h"
#include "motis/protocol/RealtimeTrainInfoResponse_generated.h"
#include "motis/railviz/timetable_retriever.h"

namespace motis {
namespace railviz {

class realtime_response {
public:
  realtime_response(motis::module::msg_ptr msg);

  std::pair<unsigned int, unsigned int> delay(const light_connection&,
                                              const edge&) const;
  unsigned int delay(const timetable_entry&) const;
  std::pair<unsigned int, unsigned int> delay(
      const route_entry& route_entry_) const;
  unsigned int delay(const light_connection&, const station_node&,
                     bool departure, unsigned int route_id) const;

  const motis::station_node* next_station_node(const motis::node& node_,
                                               unsigned int route_id) const;

private:
  unsigned int delay_batch(const light_connection&, const station_node&,
                           bool departure, unsigned int route_id) const;
  unsigned int delay_single(const motis::realtime::RealtimeTrainInfoResponse&,
                            const light_connection&, const station_node&,
                            bool departure) const;

  motis::module::msg_ptr msg;
  const motis::realtime::RealtimeTrainInfoResponse* rsp;
  const motis::realtime::RealtimeTrainInfoBatchResponse* batch_rsp;
};
}
}
