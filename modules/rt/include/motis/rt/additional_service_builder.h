#pragma once

#include <algorithm>
#include <iterator>
#include <set>
#include <string>
#include <vector>

#include "motis/core/common/get_or_create.h"
#include "motis/core/schedule/graph_build_utils.h"
#include "motis/core/schedule/schedule.h"
#include "motis/core/access/station_access.h"
#include "motis/core/access/time_access.h"
#include "motis/core/conv/event_type_conv.h"
#include "motis/loader/classes.h"
#include "motis/rt/separate_trip.h"

#include "motis/protocol/RISMessage_generated.h"

namespace motis {
namespace rt {

struct additional_service_builder {
  using section = std::tuple<light_connection, station_node*, station_node*>;

  enum class status {
    OK,
    TRIP_ID_MISMATCH,
    EVENT_COUNT_MISMATCH,
    EVENT_ORDER_MISMATCH,
    STATION_NOT_FOUND,
    EVENT_TIME_OUT_OF_RANGE,
    STATION_MISMATCH,
    DECREASING_TIME
  };

  explicit additional_service_builder(schedule& sched) : sched_(sched) {}

  size_t get_family(std::string const& cat_name) const {
    auto const it = std::find_if(
        begin(sched_.categories_), end(sched_.categories_),
        [&cat_name](auto const& cat) { return cat_name == cat->name_; });
    if (it == end(sched_.categories_)) {
      sched_.categories_.emplace_back(std::make_unique<category>(cat_name, 0));
      return sched_.categories_.size() - 1;
    } else {
      return static_cast<size_t>(std::distance(begin(sched_.categories_), it));
    }
  }

  connection_info* get_con_info(std::string const& category,
                                std::string const& line_id, int train_nr) {
    connection_info con_info;
    con_info.family_ = get_family(category);
    con_info.line_identifier_ = line_id;
    con_info.train_nr_ = train_nr;

    return get_or_create(con_infos_, con_info, [this, &con_info]() {
      sched_.connection_infos_.emplace_back(
          std::make_unique<connection_info>(con_info));
      return sched_.connection_infos_.back().get();
    });
  }

  size_t get_track(std::string const& track_name) {
    auto const it = std::find_if(
        begin(sched_.tracks_), end(sched_.tracks_),
        [&track_name](std::string const& t) { return t == track_name; });
    if (it == end(sched_.tracks_)) {
      sched_.tracks_.emplace_back(track_name);
      return sched_.tracks_.size() - 1;
    } else {
      return static_cast<size_t>(std::distance(begin(sched_.tracks_), it));
    }
  }

  static int get_clasz(std::string const& category) {
    static auto const clasz_map = loader::class_mapping();
    auto const it = clasz_map.find(category);
    return it == end(clasz_map) ? 9 : it->second;
  }

  connection* get_full_con(std::string const& dep_track,
                           std::string const& arr_track,
                           std::string const& category,
                           std::string const& line_id, int train_nr) {
    connection c;
    c.con_info_ = get_con_info(category, line_id, train_nr);
    c.d_track_ = get_track(dep_track);
    c.a_track_ = get_track(arr_track);
    c.clasz_ = get_clasz(category);
    sched_.full_connections_.emplace_back(std::make_unique<connection>(c));
    return sched_.full_connections_.back().get();
  }

  status check_events(
      flatbuffers::Vector<flatbuffers::Offset<ris::AdditionalEvent>> const*
          events) const {
    if (events->size() == 0 || events->size() % 2 != 0) {
      return status::EVENT_COUNT_MISMATCH;
    }

    station const* arr_station = nullptr;
    uint64_t prev_time = 0;
    event_type next = event_type::DEP;
    for (auto const& ev : *events) {
      auto ev_type = from_fbs(ev->base()->type());
      if (ev_type != next) {
        return status::EVENT_ORDER_MISMATCH;
      }

      if (unix_to_motistime(sched_, ev->base()->schedule_time()) ==
          INVALID_TIME) {
        return status::EVENT_TIME_OUT_OF_RANGE;
      }

      auto station = find_station(sched_, ev->base()->station_id()->str());
      if (station == nullptr) {
        return status::STATION_NOT_FOUND;
      }

      if (prev_time > ev->base()->schedule_time()) {
        return status::DECREASING_TIME;
      }

      if (ev_type == event_type::DEP && arr_station != nullptr &&
          arr_station != station) {
        return status::STATION_MISMATCH;
      }

      prev_time = ev->base()->schedule_time();
      next = (next == event_type::DEP) ? event_type::ARR : event_type::DEP;
    }

    return status::OK;
  }

  std::vector<section> build_sections(
      flatbuffers::Vector<flatbuffers::Offset<ris::AdditionalEvent>> const*
          events) {
    std::vector<section> sections;
    for (auto it = std::begin(*events); it != std::end(*events);) {
      light_connection lcon;
      lcon.valid_ = true;

      // DEP
      auto dep_station =
          get_station_node(sched_, it->base()->station_id()->str());
      auto dep_track = it->track()->str();
      lcon.d_time_ = unix_to_motistime(sched_, it->base()->schedule_time());
      ++it;

      // ARR
      auto arr_station =
          get_station_node(sched_, it->base()->station_id()->str());
      lcon.a_time_ = unix_to_motistime(sched_, it->base()->schedule_time());
      lcon.full_con_ =
          get_full_con(dep_track, it->track()->str(), it->category()->str(),
                       it->base()->line_id()->str(), it->base()->service_num());
      ++it;

      sections.emplace_back(lcon, dep_station, arr_station);
    }
    return sections;
  }

  static std::set<station_node*> get_station_nodes(
      std::vector<section> const& sections) {
    std::set<station_node*> station_nodes;
    for (auto& c : sections) {
      station_nodes.insert(std::get<1>(c));
      station_nodes.insert(std::get<2>(c));
    }
    return station_nodes;
  }

  std::vector<trip::route_edge> build_route(
      std::vector<section> const& sections,
      std::map<node const*, std::vector<edge*>>& incoming) {
    auto const route_id = sched_.route_count_++;

    std::vector<trip::route_edge> trip_edges;
    node* prev_route_node = nullptr;
    for (auto const& s : sections) {
      light_connection l;
      station_node *from_station, *to_station;
      std::tie(l, from_station, to_station) = s;

      auto const from_station_transfer_time =
          sched_.stations_.at(from_station->id_)->transfer_time_;
      auto const to_station_transfer_time =
          sched_.stations_.at(to_station->id_)->transfer_time_;

      auto const from_route_node =
          prev_route_node
              ? prev_route_node
              : build_route_node(route_id, sched_.node_count_++, from_station,
                                 from_station_transfer_time, true, true);
      auto const to_route_node =
          build_route_node(route_id, sched_.node_count_++, to_station,
                           to_station_transfer_time, true, true);

      from_route_node->edges_.push_back(
          make_route_edge(from_route_node, to_route_node, {l}));

      auto const route_edge = &from_route_node->edges_.back();
      incoming[to_route_node].push_back(route_edge);
      trip_edges.emplace_back(route_edge);

      prev_route_node = to_route_node;
    }

    return trip_edges;
  }

  trip const* update_trips(std::vector<section> const& sections,
                           std::vector<trip::route_edge> const& trip_edges) {
    station_node* first_station;
    light_connection first_lcon;
    std::tie(first_lcon, first_station, std::ignore) = sections.front();

    station_node* last_station;
    light_connection last_lcon;
    std::tie(last_lcon, std::ignore, last_station) = sections.back();

    sched_.trip_edges_.emplace_back(
        std::make_unique<std::vector<trip::route_edge>>(trip_edges));
    sched_.trip_mem_.emplace_back(std::make_unique<trip>(
        full_trip_id{primary_trip_id{first_station->id_,
                                     first_lcon.full_con_->con_info_->train_nr_,
                                     first_lcon.d_time_},
                     secondary_trip_id{
                         last_station->id_, last_lcon.a_time_,
                         first_lcon.full_con_->con_info_->line_identifier_}},
        sched_.trip_edges_.back().get(), 0));

    auto const trp = sched_.trip_mem_.back().get();
    auto const trp_entry = std::make_pair(trp->id_.primary_, trp);
    sched_.trips_.insert(
        std::lower_bound(begin(sched_.trips_), end(sched_.trips_), trp_entry),
        trp_entry);

    auto const new_trps_id = sched_.merged_trips_.size();
    sched_.merged_trips_.emplace_back(
        std::make_unique<std::vector<trip*>, std::initializer_list<trip*>>(
            {trp}));

    for (auto const& trp_edge : trip_edges) {
      trp_edge.get_edge()->m_.route_edge_.conns_[0].trips_ = new_trps_id;
    }

    return trp;
  }

  status verify_trip_id(trip const* trp, ris::IdEvent const* id_ev) {
    auto const id_station = find_station(sched_, id_ev->station_id()->str());
    auto const id_event_time =
        unix_to_motistime(sched_, id_ev->schedule_time());
    auto const trp_id = trp->id_.primary_;
    if (id_station != nullptr && id_event_time != INVALID_TIME &&
        id_station->index_ == trp_id.station_id_ &&
        id_ev->service_num() == trp_id.train_nr_ &&
        id_event_time == trp_id.time_) {
      return status::OK;
    } else {
      return status::TRIP_ID_MISMATCH;
    }
  }

  status build_additional_train(ris::AdditionMessage const* msg) {
    auto const result = check_events(msg->events());
    if (result != status::OK) {
      return result;
    }

    auto const sections = build_sections(msg->events());
    auto const station_nodes = get_station_nodes(sections);
    auto incoming = incoming_non_station_edges(station_nodes);
    auto const route = build_route(sections, incoming);
    add_incoming_station_edges(station_nodes, incoming);
    rebuild_incoming_edges(station_nodes, incoming);
    auto const trp = update_trips(sections, route);

    return verify_trip_id(trp, msg->trip_id());
  }

  schedule& sched_;
  std::map<connection_info, connection_info*> con_infos_;
};

}  // namespace rt
}  // namespace motis
