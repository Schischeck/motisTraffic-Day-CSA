#include "motis/rt/rt.h"

#include <queue>

#include "boost/program_options.hpp"

#include "motis/core/common/logging.h"
#include "motis/core/schedule/event_type.h"
#include "motis/core/schedule/graph_build_utils.h"
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
#include "motis/loader/util.h"
#include "motis/rt/additional_service_builder.h"
#include "motis/rt/bfs.h"
#include "motis/rt/delay_propagator.h"
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

struct update {
  update() = default;
  update(uint32_t station_id, time schedule_time, event_type ev_type,
         timestamp_reason reason, time new_time)
      : station_id_(station_id),
        sched_time_(schedule_time),
        ev_type_(ev_type),
        reason_(reason),
        upd_time_(new_time) {}

  friend bool operator<(update const& a, update const& b) {
    return std::tie(a.station_id_, a.reason_, a.sched_time_, a.upd_time_) <
           std::tie(b.station_id_, b.reason_, b.sched_time_, b.upd_time_);
  }

  friend bool operator==(update const& a, update const& b) {
    return std::tie(a.station_id_, a.reason_, a.sched_time_, a.upd_time_) ==
           std::tie(b.station_id_, b.reason_, b.sched_time_, b.upd_time_);
  }

  uint32_t station_id_;
  time sched_time_;
  event_type ev_type_;

  timestamp_reason reason_;
  time upd_time_;
};

std::vector<update> get_updates(schedule const& sched,
                                timestamp_reason const reason,
                                Vector<Offset<ris::UpdatedEvent>> const* events,
                                statistics& stats) {
  std::vector<update> updates;

  for (auto const& ev : *events) {
    try {
      auto const station_id = ev->base()->station_id()->str();
      auto const station_node = get_station_node(sched, station_id)->id_;
      auto const time = unix_to_motistime(sched, ev->base()->schedule_time());
      auto const upd_time = unix_to_motistime(sched, ev->updated_time());
      if (time != INVALID_TIME && upd_time != INVALID_TIME) {
        updates.emplace_back(station_node, time, from_fbs(ev->base()->type()),
                             reason, upd_time);
      } else {
        ++stats.ev_invalid_time_;
      }
    } catch (...) {
      ++stats.ev_station_not_found_;
      continue;
    }
  }

  return updates;
}

trip const* find_trip_fuzzy(schedule const& sched, ris::IdEvent const* id) {
  try {
    // first try: exact match of first departure
    auto trp = find_trip(sched, id->station_id()->str(), id->service_num(),
                         id->schedule_time());

    // second try: match of first departure with train number set to 0
    if (trp == nullptr) {
      trp = find_trip(sched, id->station_id()->str(), 0, id->schedule_time());
    }

    return trp;
  } catch (std::system_error const& e) {
    if (e.code() != access::error::station_not_found) {
      throw;
    }
    return nullptr;
  }
}

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

void rt::add_to_propagator(schedule const& sched,
                           ris::DelayMessage const* msg) {
  stats_.total_evs_ += msg->events()->size();

  // TODO(Felix Guendling) remove this when additional trains are supported
  if (msg->trip_id()->trip_type() != ris::IdEventType_Schedule) {
    stats_.additional_not_found_ += msg->events()->size();
    return;
  }

  auto trp = find_trip_fuzzy(sched, msg->trip_id());
  if (trp == nullptr) {
    stats_.ev_trp_not_found_ += msg->events()->size();
    return;
  }

  // Translate external identifiers to internal identifiers.
  auto const updates = get_updates(sched, msg->type() == ris::DelayType_Is
                                              ? timestamp_reason::IS
                                              : timestamp_reason::FORECAST,
                                   msg->events(), stats_);
  stats_.total_updates_ += updates.size();

  // For each edge of the trip:
  for (auto const& trp_e : *trp->edges_) {
    auto const e = trp_e.get_edge();

    // For each event of the edge / light connection:
    for (auto ev_type : {event_type::DEP, event_type::ARR}) {
      auto const route_node = get_route_node(*e, ev_type);

      // Try updates:
      for (auto const& upd : updates) {
        // Check whether station matches update station.
        if (upd.ev_type_ != ev_type ||
            upd.station_id_ != route_node->get_station()->id_) {
          continue;
        }

        // Check whether schedule time matches update message schedule time.
        auto const k = ev_key{e, trp->lcon_idx_, ev_type};
        auto const diff =
            std::abs(static_cast<int>(upd.sched_time_) -
                     static_cast<int>(get_schedule_time(sched, k)));
        if (diff != 0) {
          stats_.log_sched_time_mismatch(diff);
          if (diff > 5) {
            continue;
          }
        }

        propagator_->add_delay(k, upd.reason_, upd.upd_time_);
        ++stats_.found_updates_;
      }
    }
  }
}

rt::rt() = default;

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
        case MessageUnion_DelayMessage:
          add_to_propagator(s, reinterpret_cast<DelayMessage const*>(c));
          break;

        case MessageUnion_AdditionMessage:
          additional_service_builder(s).build_additional_train(
              reinterpret_cast<AdditionMessage const*>(c));
          break;

        case MessageUnion_CancelMessage: break;
        case MessageUnion_RerouteMessage: break;
        default: break;
      }
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
  hash_map<ev_key, ev_key> moved_events;
  moved_events.set_empty_key(ev_key{nullptr, 0, event_type::DEP});
  for (auto const& di : propagator_->events()) {
    auto moved_it = moved_events.find(di->get_ev_key());
    if (moved_it != end(moved_events)) {
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
      seperate_trip(sched, k, moved_events);
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
  sched.lower_bounds_ = constant_graph(sched.station_nodes_);
  lb_update.stop_and_print();

  std::cout << stats_ << std::endl;

  propagator_.reset();

  return nullptr;
}

}  // namespace rt
}  // namespace motis
