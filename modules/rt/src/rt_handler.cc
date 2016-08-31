#include "motis/rt/rt_handler.h"

#include "motis/core/common/logging.h"
#include "motis/core/common/raii.h"
#include "motis/core/common/transform_to_vec.h"

#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_publish.h"

#include "motis/rt/event_resolver.h"
#include "motis/rt/reroute.h"
#include "motis/rt/separate_trip.h"
#include "motis/rt/shifted_nodes_msg_builder.h"
#include "motis/rt/trip_correction.h"
#include "motis/rt/validy_check.h"

using motis::module::msg_ptr;
namespace fbs = flatbuffers;
using namespace motis::logging;

namespace motis {
namespace rt {

void fix_time(ev_key const& k) {
  auto const mutable_t = [&k]() -> motis::time& {
    auto& t =
        k.ev_type_ == event_type::DEP ? k.lcon()->d_time_ : k.lcon()->a_time_;
    return const_cast<motis::time&>(t);  // NOLINT
  };

  bool last = k.lcon_idx_ == k.route_edge_->m_.route_edge_.conns_.size() - 1;
  if (last) {
    mutable_t() = INVALID_TIME;
  } else {
    mutable_t() = ev_key{k.route_edge_, k.lcon_idx_ + 1, k.ev_type_}.get_time();
  }
}

rt_handler::rt_handler(schedule& sched) : sched_(sched), propagator_(sched) {}

msg_ptr rt_handler::update(msg_ptr const& msg) {
  using ris::RISBatch;

  auto& s = module::get_schedule();
  for (auto const& m : *motis_content(RISBatch, msg)->messages()) {
    auto const& nested = m->message_nested_root();
    stats_.count_message(nested->content_type());

    auto c = nested->content();
    try {
      switch (nested->content_type()) {
        case ris::MessageUnion_DelayMessage: {
          auto const msg = reinterpret_cast<ris::DelayMessage const*>(c);
          stats_.total_updates_ += msg->events()->size();

          auto const reason = (msg->type() == ris::DelayType_Is)
                                  ? timestamp_reason::IS
                                  : timestamp_reason::FORECAST;

          auto const resolved = resolve_events(
              s, msg->trip_id(),
              transform_to_vec(*msg->events(), [](ris::UpdatedEvent const* ev) {
                return ev->base();
              }));

          for (unsigned i = 0; i < resolved.size(); ++i) {
            auto const& resolved_ev = resolved[i];
            if (!resolved_ev) {
              continue;
            }

            auto const upd_time =
                unix_to_motistime(s, msg->events()->Get(i)->updated_time());
            if (upd_time == INVALID_TIME) {
              continue;
            }

            propagator_.add_delay(*resolved_ev, reason, upd_time);
            ++stats_.found_updates_;
          }

          break;
        }

        case ris::MessageUnion_AdditionMessage: {
          auto result = additional_service_builder(s).build_additional_train(
              reinterpret_cast<ris::AdditionMessage const*>(c));
          stats_.count_additional(result);
          break;
        }

        case ris::MessageUnion_CancelMessage: {
          auto const msg = reinterpret_cast<ris::CancelMessage const*>(c);

          auto trp = find_trip_fuzzy(s, msg->trip_id());
          if (trp == nullptr) {
            ++stats_.canceled_trp_not_found_;
            break;
          }

          seperate_trip(s, trp);

          auto const resolved = resolve_events(
              s, msg->trip_id(),
              transform_to_vec(*msg->events(),
                               [](ris::Event const* ev) { return ev; }));

          for (auto const& ev : resolved) {
            if (!ev) {
              continue;
            }

            auto mutable_lcon =
                const_cast<light_connection*>(ev->lcon());  // NOLINT
            mutable_lcon->valid_ = false;
          }

          break;
        }

        case ris::MessageUnion_RerouteMessage:
          propagate();
          reroute(s, cancelled_delays_,
                  reinterpret_cast<ris::RerouteMessage const*>(c));
          break;

        default: break;
      }
    } catch (std::exception const& e) {
      printf("rt::on_message: UNEXPECTED ERROR: %s\n", e.what());
      continue;
    } catch (...) {
      continue;
    }
  }

  return nullptr;
}

void rt_handler::propagate() {
  scoped_timer timer("rt update");
  manual_timer graph_update("graph update");

  auto& sched = module::get_schedule();

  LOG(info) << "rt propagating " << propagator_.events().size() << " events";
  propagator_.propagate();
  LOG(info) << "rt total changes " << propagator_.events().size();

  // Update graph.
  for (auto const& di : propagator_.events()) {
    auto const& k = di->get_ev_key();

    auto& event_time =
        k.ev_type_ == event_type::DEP ? k.lcon()->d_time_ : k.lcon()->a_time_;
    const_cast<time&>(event_time) = di->get_current_time();  // NOLINT
  }

  // Check for graph corruption and revert if necessary.
  shifted_nodes_msg_builder shifted_nodes(sched);
  for (auto const& di : propagator_.events()) {
    auto const& k = di->get_ev_key();

    if (!k.lcon()->valid_) {
      continue;
    } else if (conflicts(k)) {
      for (auto const& di : trip_corrector(sched, k).fix_times()) {
        shifted_nodes.add(di);
        ++stats_.conflicting_moved_;
      }
      ++stats_.conflicting_events_;
    } else if (overtakes(k)) {
      ++stats_.route_overtake_;
      seperate_trip(sched, k);
      fix_time(k);
    }

    shifted_nodes.add(di);
  }

  stats_.propagated_updates_ = propagator_.events().size();
  stats_.graph_updates_ = shifted_nodes.size();

  graph_update.stop_and_print();

  if (!shifted_nodes.empty()) {
    motis_publish(shifted_nodes.finish());
  }
}

msg_ptr rt_handler::flush(msg_ptr const&) {
  MOTIS_FINALLY([this]() {
    stats_ = statistics();
    propagator_.reset();
  });

  propagate();

  manual_timer lb_update("lower bound graph update");
  sched_.transfers_lower_bounds_fwd_ = build_interchange_graph(
      sched_.station_nodes_, sched_.route_count_, search_dir::FWD);
  sched_.transfers_lower_bounds_bwd_ = build_interchange_graph(
      sched_.station_nodes_, sched_.route_count_, search_dir::BWD);
  sched_.travel_time_lower_bounds_fwd_ =
      build_station_graph(sched_.station_nodes_, search_dir::FWD);
  sched_.travel_time_lower_bounds_bwd_ =
      build_station_graph(sched_.station_nodes_, search_dir::BWD);
  lb_update.stop_and_print();

  std::cout << stats_ << std::endl;

  return nullptr;
}

}  // namespace rt
}  // namespace motis
