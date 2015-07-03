#include "motis/reliability/pd_calc_data.h"

#include <algorithm>

#include "motis/core/schedule/Schedule.h"

#include "motis/reliability/graph_accessor.h"
#include "motis/reliability/probability_distribution.h"
#include "motis/reliability/tt_distribution_manager.h"

namespace motis {
namespace reliability {

pd_calc_data_departure::pd_calc_data_departure(
    td::Node& route_node, td::LightConnection const& light_connection,
    bool const is_first_route_node, td::Schedule const& schedule,
    tt_distribution_manager const& tt_dist_manager)
    : route_node_(route_node),
      light_connection_(light_connection),
      is_first_route_node_(is_first_route_node) {

  maximum_waiting_time_ = 0;

  if (is_first_route_node_) {
    auto const& train_category =
        schedule.categoryNames[light_connection_._fullCon->conInfo->family];
    train_info_.first_departure_distribution =
        &tt_dist_manager.get_start_distribution(train_category);
  } else {
    td::LightConnection const* arriving_light_conn;
    probability_distribution const* arriving_light_conn_distribution;
    std::tie(arriving_light_conn, arriving_light_conn_distribution) =
        graph_accessor::get_previous_light_connection(route_node_,
                                                      light_connection_);

    train_info_.preceding_arrival_info_.arrival_time_ =
        arriving_light_conn->aTime;
    train_info_.preceding_arrival_info_.arrival_distribution_ =
        arriving_light_conn_distribution;

    // the standing-time is always less or equal 2 minutes
    train_info_.preceding_arrival_info_.min_standing_ =
        std::min(2, light_connection_.dTime - arriving_light_conn->aTime);
  }

  auto const feeder_infos =
      graph_accessor::get_feeders(route_node, light_connection);
  for (unsigned int i = 0; i < feeder_infos.size(); i++) {
    td::Node const* feeder_route_node;
    td::LightConnection const* feeder_light_conn;
    probability_distribution const* feeder_distribution;
    // TODO: anstatt sich hier ein tuple zu holen, sollte ich direkt struct
    // feeder_info
    // aus dieser Klasse verwenden. Dann koennte man die Daten direkt in den
    // vector "feeders_" speichern.
    std::tie(feeder_route_node, feeder_light_conn, feeder_distribution) =
        graph_accessor::get_feeders(route_node_, light_connection_);
  }

#if 0
  if(numFeeders > 0) {

    unsigned short wait[numFeeders];
    const FeederEdge* feeder = NULL;

    for(unsigned int i=0 ; i<depNode->getFeederDegree() ; i++) {
      feeder = (const FeederEdge*) depNode->getFeederEdge(i);
      feeders[i] = feeder->getTail()->getProbDist();
      arrTimeFeeders[i] = feeder->getTail()->getRegularTimeMT();
      if(!arrTimeFeeders[i].isValid())
        success = false;
      transferTimes[i] = feeder->getChangeTime();
      wait[i] = feeder->getWaitingTime();
      if(storeTrainIDs)
        trainIDs[i] = feeder->getTail()->getExternalTrainIndex();
    }

    initLFA(wait);
  }
#endif
}

td::Duration pd_calc_data_departure::get_largest_delay(void) const {
  td::Duration maximum_train_delay = 0;
  if (is_first_route_node_) {
    maximum_train_delay =
        light_connection_.dTime +
        train_info_.first_departure_distribution->get_last_minute();
  } else {
    maximum_train_delay = (train_info_.preceding_arrival_info_.arrival_time_ +
                           train_info_.preceding_arrival_info_
                               .arrival_distribution_->get_last_minute() +
                           train_info_.preceding_arrival_info_.min_standing_) -
                          light_connection_.dTime;
  }

  return std::max(maximum_train_delay, maximum_waiting_time_);
}

void pd_calc_data_departure::debug_output(std::ostream& os) const {
  os << "scheduled-departure-time: " << light_connection_.dTime
     << " minimum-standing: "
     << train_info_.preceding_arrival_info_.min_standing_
     << " preceding-arrival-time: "
     << train_info_.preceding_arrival_info_.arrival_time_
     << " preceding-arrival-distribution: "
     << train_info_.preceding_arrival_info_.arrival_distribution_
     << " maximum-waiting-time: " << maximum_waiting_time_ << "\n";
  for (auto const& feeder : feeders_) {
    os << "Feeder -- arrival-time: " << feeder.arrival_time_
       //<< " distribution: " << feeder.distribution_
       << " latest-feasible-arr: " << feeder.latest_feasible_arrival_
       << " transfer-time: " << feeder.transfer_time_ << "\n";
  }
  os << std::flush;
}

}  // namespace reliability
}  // namespace motis
