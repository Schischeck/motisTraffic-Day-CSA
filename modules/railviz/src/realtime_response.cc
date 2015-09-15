#include "motis/railviz/realtime_response.h"

namespace motis {
namespace railviz {

  realtime_response::realtime_response( motis::module::msg_ptr msg ) :
    msg(msg), rsp(nullptr) {
    const motis::Message* parsed_msg = GetMessage(msg->buf_);
    rsp = reinterpret_cast<motis::realtime::RealtimeTrainInfoResponse const*> (parsed_msg->content());
  }

  std::pair<unsigned int, unsigned int> realtime_response::delay_of_train( const route_entry& route_entry_ ) const {
    std::pair<unsigned int, unsigned int> delays = std::make_pair(0,0);
    if( std::get<1>(route_entry_)->_route != rsp->route_id() )
      return delays;

    int num_stops = rsp->stops()->Length();
    for( unsigned int i = 0; i < num_stops; i++ ) {
      const realtime::EventInfo* event_info = rsp->stops()->Get(i);
      if( event_info->departure() ) {
        if( event_info->real_time() == std::get<2>(route_entry_)->d_time &&
            event_info->train_nr() == std::get<2>(route_entry_)->_full_con->con_info->train_nr &&
            event_info->station_index() == std::get<0>(route_entry_)->_id)
        {
          delays.first = event_info->real_time() - event_info->scheduled_time();
        }

      } else {
        if( event_info->real_time() == std::get<2>(route_entry_)->a_time &&
            event_info->train_nr() == std::get<2>(route_entry_)->_full_con->con_info->train_nr &&
            event_info->station_index() == next_station_id( std::get<1>(route_entry_), std::get<1>(route_entry_)->_route ))
        {
          delays.second = event_info->real_time() - event_info->scheduled_time();
        }
      }
    }

    return delays;
  }

  int realtime_response::next_station_id( const motis::node* node_, unsigned int route_id ) const {
    for( const motis::edge& e : node_->_edges ) {
      if( e.type() == motis::edge::ROUTE_EDGE ) {
        if( e._to->is_route_node() && e._to->_route == route_id )
          return e._to->get_station()->_id;
      }
    }
    return -1;
  }
}
}
