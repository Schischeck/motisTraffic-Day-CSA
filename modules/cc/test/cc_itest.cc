#include "gtest/gtest.h"

#include "motis/core/conv/event_type_conv.h"

#include "motis/test/motis_instance_test.h"

#include "./schedule.h"

using namespace flatbuffers;
using namespace motis;
using namespace motis::test;
using namespace motis::module;
using namespace motis::routing;
using motis::cc::dataset_opt;

struct id_event {
  id_event(std::string station_id, int train_num, int schedule_hhmm)
      : station_id_(std::move(station_id)),
        train_num_(train_num),
        schedule_hhmm_(schedule_hhmm) {}

  Offset<ris::IdEvent> to_fbs(schedule const& sched,
                              FlatBufferBuilder& fbb) const {
    return ris::CreateIdEvent(fbb, fbb.CreateString(station_id_), train_num_,
                              unix_time(sched, schedule_hhmm_));
  }

  std::string station_id_;
  int train_num_;
  int schedule_hhmm_;
};

struct event {
  event(std::string station_id, int train_num, std::string line_id,
        event_type ev_type, int schedule_time_hhmm)
      : station_id_(std::move(station_id)),
        train_num_(train_num),
        line_id_(std::move(line_id)),
        ev_type_(ev_type),
        schedule_time_hhmm_(schedule_time_hhmm) {}

  Offset<ris::Event> to_fbs(schedule const& sched,
                            FlatBufferBuilder& fbb) const {
    return ris::CreateEvent(fbb, fbb.CreateString(station_id_), train_num_,
                            fbb.CreateString(line_id_), motis::to_fbs(ev_type_),
                            unix_time(sched, schedule_time_hhmm_));
  }

  std::string station_id_;
  int train_num_;
  std::string line_id_;
  event_type ev_type_;
  int schedule_time_hhmm_;
};

struct cc_check_routed_connection_test : public motis_instance_test {
  cc_check_routed_connection_test()
      : motis::test::motis_instance_test(dataset_opt, {"cc", "routing", "rt"}) {
  }

  msg_ptr route(std::string const& from, std::string const& to,
                int hhmm) const {
    message_creator fbb;
    fbb.create_and_finish(
        MsgContent_RoutingRequest,
        CreateRoutingRequest(
            fbb, Start_OntripStationStart,
            CreateOntripStationStart(
                fbb, CreateInputStation(fbb, fbb.CreateString(from),
                                        fbb.CreateString("")),
                unix_time(hhmm))
                .Union(),
            CreateInputStation(fbb, fbb.CreateString(to), fbb.CreateString("")),
            SearchType_SingleCriterion, SearchDir_Forward,
            fbb.CreateVector(std::vector<Offset<Via>>()),
            fbb.CreateVector(std::vector<Offset<AdditionalEdgeWrapper>>()))
            .Union(),
        "/routing");
    return make_msg(fbb);
  }

  msg_ptr check_connection(Connection const* con) const {
    message_creator fbb;
    fbb.create_and_finish(MsgContent_Connection,
                          motis_copy_table(Connection, fbb, con).Union(),
                          "/cc");
    return make_msg(fbb);
  }

  void check_schedule_okay(std::string const& from, std::string const& to,
                           int hhmm) {
    auto const routing_res = call(route(from, to, hhmm));
    auto const connections =
        motis_content(RoutingResponse, routing_res)->connections();
    ASSERT_EQ(1, connections->size());

    auto const cc_res = call(check_connection(connections->Get(0)));
    EXPECT_NE(MsgContent_MotisError, cc_res->get()->content_type());
  }

  msg_ptr reroute_cancel(schedule const& sched, id_event const& id,
                         std::vector<event> const& cancel_events) const {
    FlatBufferBuilder fbb;
    fbb.Finish(ris::CreateMessage(
        fbb, ris::MessageUnion_RerouteMessage,
        ris::CreateRerouteMessage(
            fbb, id.to_fbs(sched, fbb),
            fbb.CreateVector(transform_to_vec(
                cancel_events,
                [&](event const& ev) { return ev.to_fbs(sched, fbb); })),
            fbb.CreateVector(std::vector<Offset<ris::ReroutedEvent>>{}))
            .Union()));

    message_creator mc;
    auto const msg_holders =
        std::vector<Offset<ris::MessageHolder>>{ris::CreateMessageHolder(
            mc, mc.CreateVector(fbb.GetBufferPointer(), fbb.GetSize()))};
    mc.create_and_finish(
        MsgContent_RISBatch,
        ris::CreateRISBatch(mc, mc.CreateVector(msg_holders)).Union(),
        "/ris/messages", DestinationType_Topic);

    return make_msg(mc);
  }
};

TEST_F(cc_check_routed_connection_test, simple_result_ok) {
  check_schedule_okay("0000002", "0000011", 1700);
  check_schedule_okay("0000001", "0000005", 1700);
  check_schedule_okay("0000003", "0000005", 1700);
  check_schedule_okay("0000003", "0000008", 1700);
}

/*
 * std::string station_id,
 * int train_num,
 * std::string line_id,
   event_type ev_type,
   int schedule_time_hhmm
 */
TEST_F(cc_check_routed_connection_test, first_enter_cancelled) {
  auto const routing_res = call(route("0000003", "0000005", 1700));
  publish(reroute_cancel(sched(), id_event{"0000003", 6, 1810},
                         {event{"0000003", 6, "", event_type::DEP, 1810},
                          event{"0000004", 6, "", event_type::ARR, 1900}}));
  publish(make_no_msg("/ris/system_time_changed"));
  auto const check_res = check_connection(
      motis_content(RoutingResponse, routing_res)->connections()->Get(0));
  std::cout << check_res->to_json() << std::endl;
}
