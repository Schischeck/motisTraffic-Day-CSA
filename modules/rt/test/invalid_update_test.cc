#include "gtest/gtest.h"

#include <map>

#include "motis/core/access/realtime_access.h"
#include "motis/core/access/trip_access.h"
#include "motis/test/motis_instance_test.h"
#include "motis/test/schedule/invalid_realtime.h"

using namespace motis;
using namespace motis::module;
using namespace motis::test;
using namespace motis::test::schedule;
using motis::test::schedule::invalid_realtime::dataset_opt;
using motis::test::schedule::invalid_realtime::get_trip_conflict_ris_message;

struct rt_invalid_update_test : public motis_instance_test {
  struct event_info {
    motis::time graph_time_;
    delay_info const* di_;
  };

  using trip_event_info =
      std::map<std::string /* station id */,
               std::pair<event_info /* arrival */, event_info /* departure */>>;

  trip_event_info get_trip_event_info(trip const* trp) {
    trip_event_info trp_ev_info;
    for (auto const& trip_e : *trp->edges_) {
      auto const e = trip_e.get_edge();

      auto const from_id =
          sched().stations_.at(e->from_->get_station()->id_)->eva_nr_;
      auto const d_time = e->get_connection(trp->lcon_idx_)->d_time_;
      auto const dep = ev_key{e, trp->lcon_idx_, event_type::DEP};

      auto const to_id =
          sched().stations_.at(e->to_->get_station()->id_)->eva_nr_;
      auto const a_time = e->get_connection(trp->lcon_idx_)->a_time_;
      auto const arr = ev_key{e, trp->lcon_idx_, event_type::ARR};

      auto& from = trp_ev_info[from_id];
      from.second.graph_time_ = d_time;
      from.second.di_ = get_delay_info(sched(), dep);

      auto& to = trp_ev_info[to_id];
      to.first.graph_time_ = a_time;
      to.first.di_ = get_delay_info(sched(), arr);
    }
    return trp_ev_info;
  }

  rt_invalid_update_test()
      : motis::test::motis_instance_test(dataset_opt, {"rt"}) {}
};

TEST_F(rt_invalid_update_test, trip_conflict_test) {
  publish(get_trip_conflict_ris_message(sched()));
  publish(make_no_msg("/ris/system_time_changed"));

  auto trp = get_trip(sched(), "0000001", 1, unix_time(1010), "0000005",
                      unix_time(1400), "381");
  auto events = get_trip_event_info(trp);

  EXPECT_EQ(motis_time(1010), events["0000001"].second.graph_time_);
  EXPECT_EQ(motis_time(1100), events["0000002"].first.graph_time_);
  EXPECT_EQ(motis_time(1301), events["0000002"].second.graph_time_);
  EXPECT_EQ(motis_time(1301), events["0000003"].first.graph_time_);
  EXPECT_EQ(motis_time(1301), events["0000003"].second.graph_time_);
  EXPECT_EQ(motis_time(1301), events["0000004"].first.graph_time_);
  EXPECT_EQ(motis_time(1310), events["0000004"].second.graph_time_);
  EXPECT_EQ(motis_time(1400), events["0000005"].first.graph_time_);
}
