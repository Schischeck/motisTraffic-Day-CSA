#include "motis/rt/rt.h"

#include <queue>

#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/common/transform_to_vec.h"
#include "motis/core/schedule/event_type.h"
#include "motis/core/access/edge_access.h"
#include "motis/core/access/realtime_access.h"
#include "motis/core/access/service_access.h"
#include "motis/core/access/station_access.h"
#include "motis/core/access/time_access.h"
#include "motis/core/access/trip_access.h"
#include "motis/core/conv/event_type_conv.h"
#include "motis/core/conv/timestamp_reason_conv.h"
#include "motis/module/context/get_schedule.h"
#include "motis/module/context/motis_publish.h"
#include "motis/loader/classes.h"
#include "motis/rt/additional_service_builder.h"
#include "motis/rt/bfs.h"
#include "motis/rt/delay_propagator.h"
#include "motis/rt/event_resolver.h"
#include "motis/rt/find_trip_fuzzy.h"
#include "motis/rt/reroute.h"
#include "motis/rt/separate_trip.h"
#include "motis/rt/shifted_nodes_msg_builder.h"
#include "motis/rt/trip_correction.h"
#include "motis/rt/validy_check.h"

namespace po = boost::program_options;
using namespace flatbuffers;
using namespace motis::module;
using namespace motis::logging;
using motis::ris::RISBatch;

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

rt::rt() { moved_events_.set_empty_key(ev_key{nullptr, 0, event_type::DEP}); }

rt::~rt() = default;

po::options_description rt::desc() {
  po::options_description desc("RT Module");
  return desc;
}

void rt::init(motis::module::registry& reg) {
  namespace p = std::placeholders;
  reg.subscribe("/ris/messages", std::bind(&rt::on_message, this, p::_1));
  reg.subscribe("/ris/system_time_changed",
                std::bind(&rt::on_system_time_change, this, p::_1));
}

msg_ptr rt::on_message(msg_ptr const& msg) {
  using namespace ris;

  auto& s = get_schedule();
  if (!propagator_) {
    LOG(info) << "initializing propagator";
    propagator_ = std::make_unique<delay_propagator>(s);
    stats_ = statistics();
  }

  for (auto const& m : *motis_content(RISBatch, msg)->messages()) {
    auto const& nested = m->message_nested_root();
    stats_.count_message(nested->content_type());

    auto c = nested->content();
    try {
      switch (nested->content_type()) {
        case MessageUnion_DelayMessage: {
          auto const msg = reinterpret_cast<DelayMessage const*>(c);
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

            propagator_->add_delay(*resolved_ev, reason, upd_time);
            ++stats_.found_updates_;
          }

          break;
        }

        case MessageUnion_AdditionMessage: {
          auto result = additional_service_builder(s).build_additional_train(
              reinterpret_cast<AdditionMessage const*>(c));
          stats_.count_additional(result);
          break;
        }

        case MessageUnion_CancelMessage: {
          auto const msg = reinterpret_cast<CancelMessage const*>(c);

          auto trp = find_trip_fuzzy(s, msg->trip_id());
          if (trp == nullptr) {
            ++stats_.canceled_trp_not_found_;
            break;
          }

          seperate_trip(s, trp, moved_events_);

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

        case MessageUnion_RerouteMessage:
          reroute(s, reinterpret_cast<RerouteMessage const*>(c), moved_events_);
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

msg_ptr rt::on_system_time_change(msg_ptr const&) {
  scoped_timer timer("rt update");
  manual_timer graph_update("graph update");

  auto& sched = get_schedule();

  if (!propagator_) {
    LOG(info) << "rt no updates - ignoring system time change";
    return nullptr;
  }

  LOG(info) << "rt propagating " << propagator_->events().size() << " events";
  propagator_->propagate();
  LOG(info) << "rt total changes " << propagator_->events().size();

  // Update graph.
  for (auto const& di : propagator_->events()) {
    auto const& k = di->get_ev_key();

    auto& event_time =
        k.ev_type_ == event_type::DEP ? k.lcon()->d_time_ : k.lcon()->a_time_;
    const_cast<time&>(event_time) = di->get_current_time();  // NOLINT
  }

  // Check for graph corruption and revert if necessary.
  shifted_nodes_msg_builder shifted_nodes(sched);
  for (auto const& di : propagator_->events()) {
    auto moved_it = moved_events_.find(di->get_ev_key());
    if (moved_it != end(moved_events_)) {
      di->set_ev_key(moved_it->second);
    }

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
      seperate_trip(sched, k, moved_events_);
      fix_time(k);
    }

    shifted_nodes.add(di);
  }

  stats_.propagated_updates_ = propagator_->events().size();
  stats_.graph_updates_ = shifted_nodes.size();

  graph_update.stop_and_print();

  if (!shifted_nodes.empty()) {
    motis_publish(shifted_nodes.finish());
  }

  manual_timer lb_update("lower bound graph update");
  sched.transfers_lower_bounds_fwd_ = build_interchange_graph(
      sched.station_nodes_, sched.route_count_, search_dir::FWD);
  sched.transfers_lower_bounds_bwd_ = build_interchange_graph(
      sched.station_nodes_, sched.route_count_, search_dir::BWD);
  sched.travel_time_lower_bounds_fwd_ =
      build_station_graph(sched.station_nodes_, search_dir::FWD);
  sched.travel_time_lower_bounds_bwd_ =
      build_station_graph(sched.station_nodes_, search_dir::BWD);
  lb_update.stop_and_print();

  std::cout << stats_ << std::endl;

  propagator_.reset();
  moved_events_.clear();

  return nullptr;
}

}  // namespace rt
}  // namespace motis
