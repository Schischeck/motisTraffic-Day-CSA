#include "motis/routing/labels_to_journey.h"

#include "boost/lexical_cast.hpp"

#include "motis/core/schedule/time.h"
#include "motis/core/schedule/schedule.h"
#include "motis/routing/label.h"
#include "motis/routing/interval_map.h"

#define UNKNOWN_TRACK (0)

namespace motis {

namespace intermediate {

struct stop {
  stop() = default;
  stop(unsigned int index, unsigned int station_id, unsigned int a_platform,
       unsigned int d_platform, time a_time, time d_time, bool interchange)
      : index(index),
        station_id(station_id),
        a_platform(a_platform),
        d_platform(d_platform),
        a_time(a_time),
        d_time(d_time),
        interchange(interchange) {}

  unsigned int index;
  unsigned int station_id;
  unsigned int a_platform, d_platform;
  time a_time, d_time;
  bool interchange;
};

struct transport {
  transport() = default;
  transport(unsigned int from, unsigned int to, light_connection const* con,
            unsigned int route_id)
      : from(from),
        to(to),
        con(con),
        duration(con->a_time - con->d_time),
        slot(-1),
        route_id(route_id),
        type(journey::transport::PublicTransport),
        mumo_type_name(""),
        mumo_price(0) {}

  transport(unsigned int from, unsigned int to, unsigned int duration, int slot,
            unsigned int route_id, journey::transport::transport_type type,
            std::string mumo_type_name = "", unsigned int mumo_price = 0)
      : from(from),
        to(to),
        con(nullptr),
        duration(duration),
        slot(slot),
        route_id(route_id),
        type(type),
        mumo_type_name(mumo_type_name),
        mumo_price(mumo_price) {}

  unsigned int from, to;
  light_connection const* con;
  unsigned int duration;
  int slot;
  unsigned int route_id;
  journey::transport::transport_type type;
  std::string mumo_type_name;
  unsigned short mumo_price;
};

std::tuple<journey::transport::transport_type, std::string, unsigned short>
get_mumo_info(label const& current_label, label const& next_label) {
  journey::transport::transport_type type = journey::transport::Walk;
  std::string mumo_type_name = "";
  unsigned short mumo_price = 0;
  if (next_label._used_edge_type == edge::HOTEL_EDGE) {
    type = journey::transport::Mumo;
    mumo_type_name = "Hotel";
    mumo_price = next_label._db_costs - current_label._db_costs;
  } else if ((next_label._used_edge_type == edge::MUMO_EDGE ||
              next_label._used_edge_type == edge::TIME_DEPENDENT_MUMO_EDGE) &&
             /* assure that this is not a dummy edge */
             next_label._now > current_label._now) {
    type = journey::transport::Mumo;
    mumo_type_name = "Taxi"; /* todo: read type from slot */
    mumo_price = next_label._db_costs - current_label._db_costs;
  }
  return std::make_tuple(type, mumo_type_name, mumo_price);
}

std::pair<std::vector<intermediate::stop>, std::vector<intermediate::transport>>
parse_label_chain(label const* terminal_label) {
  std::vector<label const*> labels;
  label const* c = terminal_label;
  do {
    labels.insert(begin(labels), c);
  } while ((c = c->_pred));

  std::vector<intermediate::stop> stops;
  std::vector<intermediate::transport> transports;
  enum state { AT_STATION, PRE_CONNECTION, IN_CONNECTION, PRE_WALK, WALK };

  light_connection const* last_con = nullptr;
  time walk_arrival = INVALID_TIME;
  int station_index = -1;

  auto next_state = [](int s, label const* c, label const* n) -> state {
    switch (s) {
      case AT_STATION:
        if (n && n->_node->is_station_node()) {
          return WALK;
        } else {
          return PRE_CONNECTION;
        }
      case PRE_CONNECTION: return IN_CONNECTION;
      case IN_CONNECTION:
        if (c->_connection == nullptr) {
          return n->_node->is_station_node() ? WALK : AT_STATION;
        } else {
          return IN_CONNECTION;
        }
      case WALK:
        if (n && n->_node->is_station_node()) {
          return WALK;
        } else {
          return AT_STATION;
        }
    }
    return static_cast<state>(s);
  };

  int current_state;

  auto it = begin(labels);
  if (labels[1]->_node->is_station_node()) {
    current_state = WALK;
  } else if (labels[1]->_node->is_foot_node()) {
    current_state = WALK;
    ++it;
  } else {
    current_state = AT_STATION;
  }

  for (; it != end(labels);) {
    auto current = *it;

    switch (current_state) {
      case AT_STATION: {
        int a_platform = UNKNOWN_TRACK;
        int d_platform = UNKNOWN_TRACK;
        time a_time = walk_arrival;
        time d_time = INVALID_TIME;
        if (a_time == INVALID_TIME && last_con != nullptr) {
          a_platform = last_con->_full_con->a_platform;
          a_time = last_con->a_time;
        }

        walk_arrival = INVALID_TIME;

        auto s1 = std::next(it, 1);
        auto s2 = std::next(it, 2);
        if (s1 != end(labels) && s2 != end(labels) &&
            (*s2)->_connection != nullptr) {
          d_platform = (*s2)->_connection->_full_con->d_platform;
          d_time = (*s2)->_connection->d_time;
        }

        stops.emplace_back((unsigned int)++station_index,
                           current->_node->get_station()->_id, a_platform,
                           d_platform, a_time, d_time,
                           a_time != INVALID_TIME && d_time != INVALID_TIME &&
                               last_con != nullptr);
        break;
      }

      case WALK:
        assert(std::next(it) != end(labels));

        stops.emplace_back(
            (unsigned int)++station_index, current->_node->get_station()->_id,
            last_con == nullptr ? UNKNOWN_TRACK
                                : last_con->_full_con->a_platform,
            UNKNOWN_TRACK,
            stops.empty() ? INVALID_TIME : (last_con == nullptr)
                                               ? current->_now
                                               : last_con->a_time,
            current->_now, last_con != nullptr);

        {
          auto mumo_info = get_mumo_info(*current, *(*std::next(it)));
          transports.emplace_back(
              station_index, (unsigned int)station_index + 1,
              (*std::next(it))->_now - current->_now, -1, 0,
              std::get<0>(mumo_info), std::get<1>(mumo_info),
              std::get<2>(mumo_info));
        }

        walk_arrival = (*std::next(it))->_now;

        last_con = nullptr;
        break;

      case IN_CONNECTION:
        transports.emplace_back((unsigned int)station_index,
                                (unsigned int)station_index + 1,
                                current->_connection, current->_node->_route);

        // do not collect the last connection route node.
        assert(std::next(it) != end(labels));
        auto succ = *std::next(it);
        if (succ->_node->is_route_node()) {
          stops.emplace_back(
              (unsigned int)++station_index, current->_node->get_station()->_id,
              current->_connection->_full_con->a_platform,
              succ->_connection->_full_con->d_platform,
              current->_connection->a_time, succ->_connection->d_time, false);
        }

        last_con = current->_connection;
        break;
    }

    ++it;
    if (it != end(labels)) {
      current = *it;
      auto next = it == end(labels) || std::next(it) == end(labels)
                      ? nullptr
                      : *std::next(it);
      current_state = next_state(current_state, current, next);
    }
  }

  transports.front().slot = terminal_label->get_slot(true);
  transports.back().slot = terminal_label->get_slot(false);

  return {stops, transports};
}

}  // namespace intermediate

journey::transport generate_journey_transport(unsigned int from,
                                              unsigned int to,
                                              intermediate::transport const& t,
                                              schedule const& sched,
                                              unsigned int route_id) {
  std::string name;
  std::string cat_name;
  unsigned cat_id = 0;
  unsigned clasz = 0;
  unsigned train_nr = 0;
  std::string line_identifier;
  unsigned duration = t.duration;
  int slot = -1;
  std::string direction;
  std::string provider;

  if (t.con != nullptr) {
    std::string print_train_nr;
    connection_info const* con_info = t.con->_full_con->con_info;
    line_identifier = con_info->line_identifier;
    cat_id = con_info->family;
    clasz = t.con->_full_con->clasz;
    cat_name = sched.categories[con_info->family]->name;
    train_nr = con_info->train_nr <= 999999 ? con_info->train_nr
                                            : con_info->original_train_nr;
    if (train_nr != 0) {
      print_train_nr = boost::lexical_cast<std::string>(train_nr);
    } else if (train_nr == 0 && !line_identifier.empty()) {
      print_train_nr = line_identifier;
    } else {
      print_train_nr = "";
    }
    if (con_info->dir_ != nullptr) {
      direction = *con_info->dir_;
    }
    if (con_info->provider_ != nullptr) {
      provider = con_info->provider_->full_name;
    }
    switch (sched.categories[con_info->family]->output_rule) {
      case category::CATEGORY_AND_TRAIN_NUM:
        name = cat_name + " " + print_train_nr;
        break;

      case category::CATEGORY: name = cat_name; break;

      case category::TRAIN_NUM: name = print_train_nr; break;

      case category::NOTHING: break;

      case category::PROVIDER_AND_TRAIN_NUM:
        if (con_info->provider_ != nullptr) {
          name = con_info->provider_->short_name + " ";
        }
        name += print_train_nr;
        break;

      case category::PROVIDER:
        if (con_info->provider_ != nullptr) {
          name = con_info->provider_->short_name;
        }
        break;

      case category::CATEGORY_AND_LINE:
        name = cat_name + " " + line_identifier;
        break;
    }
  }

  return {
      from,     to,       t.type,           name,        cat_name, cat_id,
      clasz,    train_nr, line_identifier,  duration,    slot,     direction,
      provider, route_id, t.mumo_type_name, t.mumo_price};
}

std::vector<journey::transport> generate_journey_transports(
    std::vector<intermediate::transport> const& transports,
    schedule const& sched) {
  auto con_info_eq = [](connection_info const* a,
                        connection_info const* b) -> bool {
    if (a == nullptr || b == nullptr) {
      return false;
    } else {
      // equals comparison ignoring attributes:
      return a->line_identifier == b->line_identifier &&
             a->family == b->family && a->train_nr == b->train_nr;
    }
  };

  std::vector<journey::transport> journey_transports;

  bool isset_last = false;
  intermediate::transport const* last = nullptr;
  connection_info const* last_con_info = nullptr;
  unsigned from = 1;

  for (auto const& transport : transports) {
    connection_info const* con_info = nullptr;
    if (transport.con != nullptr) {
      con_info = transport.con->_full_con->con_info;
    }

    if (!con_info_eq(con_info, last_con_info)) {
      if (last != nullptr && isset_last) {
        journey_transports.push_back(generate_journey_transport(
            from, transport.from, *last, sched, last->route_id));
      }

      isset_last = true;
      last = &transport;
      from = transport.from;
    }

    last_con_info = con_info;
  }

  auto back = transports.back();
  journey_transports.push_back(
      generate_journey_transport(from, back.to, *last, sched, back.route_id));

  return journey_transports;
}

std::vector<journey::stop> generate_journey_stops(
    std::vector<intermediate::stop> const& stops, schedule const& sched) {
  std::vector<journey::stop> journey_stops;
  for (auto const& stop : stops) {
    journey_stops.push_back(
        {stop.index, stop.interchange, sched.stations[stop.station_id]->name,
         sched.stations[stop.station_id]->eva_nr,
         sched.stations[stop.station_id]->width,
         sched.stations[stop.station_id]->length,
         stop.a_time != INVALID_TIME
             ? journey::stop::event_info{true, motis_to_unixtime(
                                                   sched.schedule_begin_,
                                                   stop.a_time),
                                         motis_to_unixtime(
                                             sched.schedule_begin_,
                                             stop.a_time),
                                         sched.tracks[stop.a_platform]}
             : journey::stop::event_info{false, 0, 0, ""},
         stop.d_time != INVALID_TIME
             ? journey::stop::event_info{true, motis_to_unixtime(
                                                   sched.schedule_begin_,
                                                   stop.d_time),
                                         motis_to_unixtime(
                                             sched.schedule_begin_,
                                             stop.d_time),
                                         sched.tracks[stop.d_platform]}
             : journey::stop::event_info{false, 0, 0, ""}});
  }
  return journey_stops;
}

std::vector<journey::attribute> generate_journey_attributes(
    std::vector<intermediate::transport> const& transports) {
  interval_map<attribute const*> attributes;
  for (auto const& transport : transports) {
    if (transport.con == nullptr) {
      continue;
    } else {
      for (auto const& attribute :
           transport.con->_full_con->con_info->attributes) {
        attributes.add_entry(attribute, transport.from, transport.to);
      }
    }
  }

  std::vector<journey::attribute> journey_attributes;
  for (auto const& attribute_range : attributes.get_attribute_ranges()) {
    auto const& attribute = attribute_range.first;
    auto const& attribute_ranges = attribute_range.second;
    auto const& code = attribute->_code;
    auto const& text = attribute->_str;

    for (auto const& range : attribute_ranges) {
      journey_attributes.push_back({static_cast<unsigned>(range.from),
                                    static_cast<unsigned>(range.to), code,
                                    text});
    }
  }

  return journey_attributes;
}

journey labels_to_journey(label const* label, schedule const& sched) {
  journey j;
  auto parsed = intermediate::parse_label_chain(label);
  std::vector<intermediate::stop> const& s = parsed.first;
  std::vector<intermediate::transport> const& t = parsed.second;

  j.stops = generate_journey_stops(s, sched);
  j.transports = generate_journey_transports(t, sched);
  j.attributes = generate_journey_attributes(t);

  j.duration = label->_travel_time[0];
  j.transfers = label->_transfers[0] - 1;
  j.price = label->_total_price[0];
  j.night_penalty = label->_night_penalty;
  return j;
}

}  // namespace motis
