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

void handle_add(schedule& sched, ris::AdditionMessage const* msg) {
  auto const clasz_map = loader::class_mapping();
  std::map<connection_info, connection_info*> con_infos;

  auto const get_family = [&sched](std::string const& cat_name) {
    auto const it = std::find_if(
        begin(sched.categories_), end(sched.categories_),
        [&cat_name](auto const& cat) { return cat_name == cat->name_; });
    if (it == end(sched.categories_)) {
      sched.categories_.emplace_back(std::make_unique<category>(cat_name, 0));
      return sched.categories_.size() - 1;
    } else {
      return static_cast<size_t>(std::distance(begin(sched.categories_), it));
    }
  };

  auto const get_con_info = [&sched, &con_infos, &get_family](
      std::string const& category, std::string const& line_id, int train_nr) {
    connection_info con_info;
    con_info.family_ = get_family(category);
    con_info.line_identifier_ = line_id;
    con_info.train_nr_ = train_nr;

    return get_or_create(con_infos, con_info, [&sched, &con_info]() {
      sched.connection_infos_.emplace_back(
          std::make_unique<connection_info>(con_info));
      return sched.connection_infos_.back().get();
    });
  };

  auto const get_track = [&sched](std::string const& track_name) {
    auto const it = std::find_if(
        begin(sched.tracks_), end(sched.tracks_),
        [&track_name](std::string const& t) { return t == track_name; });
    if (it == end(sched.tracks_)) {
      sched.tracks_.emplace_back(track_name);
      return sched.tracks_.size() - 1;
    } else {
      return static_cast<size_t>(std::distance(begin(sched.tracks_), it));
    }
  };

  auto const get_clasz = [&clasz_map](std::string const& category) {
    auto const it = clasz_map.find(category);
    if (it == end(clasz_map)) {
      return 9;
    } else {
      return it->second;
    }
  };

  auto const get_full_con = [&sched, &get_con_info, &get_track, get_clasz](
      std::string const& dep_track, std::string const& arr_track,
      std::string const& category, std::string const& line_id, int train_nr) {
    connection c;
    c.con_info_ = get_con_info(category, line_id, train_nr);
    c.a_track_ = get_track(dep_track);
    c.d_track_ = get_track(arr_track);
    c.clasz_ = get_clasz(category);
    sched.full_connections_.emplace_back(std::make_unique<connection>(c));
    return sched.full_connections_.back().get();
  };

  // Check times and stations.
  for (auto const& ev : *msg->events()) {
    auto const time = unix_to_motistime(sched, ev->base()->schedule_time());
    if (time == INVALID_TIME) {
      return;
    }

    auto const station = find_station(sched, ev->base()->station_id()->str());
    if (station == nullptr) {
      return;
    }
  }

  // Build light connections.
  std::vector<std::tuple<light_connection, station_node*, station_node*>> lcons;
  for (auto it = std::begin(*msg->events()); it != std::end(*msg->events());) {
    light_connection lcon;

    // DEP
    auto dep_station = get_station_node(sched, it->base()->station_id()->str());
    auto dep_track = it->track()->str();
    lcon.d_time_ = unix_to_motistime(sched, it->base()->schedule_time());
    ++it;

    // ARR
    auto arr_station = get_station_node(sched, it->base()->station_id()->str());
    lcon.a_time_ = unix_to_motistime(sched, it->base()->schedule_time());
    lcon.full_con_ =
        get_full_con(dep_track, it->track()->str(), it->category()->str(),
                     it->base()->line_id()->str(), it->base()->service_num());
    ++it;

    lcons.emplace_back(lcon, dep_station, arr_station);
  }

  // Remember incoming non-station edges.
  std::set<station_node*> station_nodes;
  for (auto& c : lcons) {
    station_nodes.insert(std::get<1>(c));
    station_nodes.insert(std::get<2>(c));
  }
  auto incoming = incoming_non_station_edges(station_nodes);

  // Build route.
  auto const route_id = sched.route_count_++;
  std::vector<trip::route_edge> trip_edges;
  node* prev_route_node = nullptr;
  for (auto const& lcon : lcons) {
    light_connection l;
    station_node *from_station, *to_station;
    std::tie(l, from_station, to_station) = lcon;

    auto const from_station_transfer_time =
        sched.stations_.at(from_station->id_)->transfer_time_;
    auto const to_station_transfer_time =
        sched.stations_.at(to_station->id_)->transfer_time_;

    node *from_route_node =
             prev_route_node
                 ? prev_route_node
                 : build_route_node(route_id, sched.node_count_++, from_station,
                                    from_station_transfer_time, true, true),
         *to_route_node =
             build_route_node(route_id, sched.node_count_++, to_station,
                              to_station_transfer_time, true, true);

    from_route_node->edges_.push_back(
        make_route_edge(from_route_node, to_route_node, {l}));

    auto const route_edge = &from_route_node->edges_.back();
    incoming[to_route_node].push_back(route_edge);
    trip_edges.emplace_back(route_edge);

    prev_route_node = to_route_node;
  }

  // Rebuild incoming edges.
  add_incoming_station_edges(station_nodes, incoming);
  rebuild_incoming_edges(station_nodes, incoming);

  // Create trip information.
  station_node* first_station;
  light_connection first_lcon;
  std::tie(first_lcon, first_station, std::ignore) = lcons.front();

  station_node* last_station;
  light_connection last_lcon;
  std::tie(last_lcon, std::ignore, last_station) = lcons.back();

  sched.trip_edges_.emplace_back(new std::vector<trip::route_edge>(trip_edges));
  sched.trip_mem_.emplace_back(new trip(
      full_trip_id{
          primary_trip_id{first_station->id_,
                          first_lcon.full_con_->con_info_->train_nr_,
                          first_lcon.d_time_},
          secondary_trip_id{last_station->id_, last_lcon.a_time_,
                            first_lcon.full_con_->con_info_->line_identifier_}},
      sched.trip_edges_.back().get(), 0));
}

void handle_cancel(schedule&, ris::CancelMessage const*) {}

void handle_reroute(schedule&, ris::RerouteMessage const*) {}

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
          handle_add(s, reinterpret_cast<AdditionMessage const*>(c));
          break;

        case MessageUnion_CancelMessage:
          handle_cancel(s, reinterpret_cast<CancelMessage const*>(c));
          break;

        case MessageUnion_RerouteMessage:
          handle_reroute(s, reinterpret_cast<RerouteMessage const*>(c));
          break;

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
