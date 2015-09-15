#pragma once

#include "motis/module/module.h"
#include "motis/railviz/timetable_retriever.h"
#include "motis/protocol/Message_generated.h"
#include "motis/protocol/RealtimeTrainInfoResponse_generated.h"

namespace motis {
namespace railviz {

struct realtime_response {
  realtime_response( motis::module::msg_ptr msg );

  std::pair<unsigned int, unsigned int> delay_of_train( const route_entry& route_entry_ ) const;
  int next_station_id( const motis::node* node_, unsigned int route_id ) const;

  motis::module::msg_ptr msg;
  const motis::realtime::RealtimeTrainInfoResponse* rsp;
};

}
}
