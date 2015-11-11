#include "motis/routing/journey.h"

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
  stop(int index, int station_id, int a_platform, int d_platform, time a_time,
       time d_time, bool interchange)
      : index(index),
        station_id(station_id),
        a_platform(a_platform),
        d_platform(d_platform),
        a_time(a_time),
        d_time(d_time),
        interchange(interchange) {}

  int index;
  int station_id;
  int a_platform, d_platform;
  time a_time, d_time;
  bool interchange;
};

struct transport {
  transport() = default;
  transport(int from, int to, light_connection const* con)
      : from(from),
        to(to),
        con(con),
        duration(con->a_time - con->d_time),
        slot(-1) {}

  transport(int from, int to, int duration, int slot)
      : from(from), to(to), con(nullptr), duration(duration), slot(slot) {}

  int from, to;
  light_connection const* con;
  int duration;
  int slot;
};

std::pair<std::vector<intermediate::stop>, std::vector<intermediate::transport>>
parse_label_chain(label const* terminal_label) {
  std::vector<label const*> labels;
  label const* c = terminal_label;
  do {
    labels.insert(begin(labels), c);
  } while ((c = c->_pred));

  std::vector<intermediate::stop> stops;
  std::vector<intermediate::transport> transports;
  enum state {
    AT_STATION,
    PRE_CONNECTION,
    IN_CONNECTION,
    WALK
  } state = AT_STATION;
  light_connection const* last_con = nullptr;
  time walk_arrival = INVALID_TIME;
  int station_index = -1;

  for (auto it = begin(labels); it != end(labels); ++it) {
    auto current = *it;

    if (state == IN_CONNECTION && current->_connection == nullptr) {
      state = current->_node->is_station_node() ? AT_STATION : WALK;
    }

    if (state == AT_STATION && std::next(it) != end(labels) &&
        (*std::next(it))->_node->is_station_node()) {
      state = WALK;
    }

    switch (state) {
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

        stops.emplace_back(++station_index, current->_node->get_station()->_id,
                           a_platform, d_platform, a_time, d_time,
                           a_time != INVALID_TIME && d_time != INVALID_TIME &&
                               last_con != nullptr);

        state = PRE_CONNECTION;
        break;
      }

      case PRE_CONNECTION: state = IN_CONNECTION; break;

      case WALK:
        assert(std::next(it) != end(labels));

        stops.emplace_back(
            ++station_index, current->_node->get_station()->_id,
            last_con == nullptr ? UNKNOWN_TRACK
                                : last_con->_full_con->a_platform,
            UNKNOWN_TRACK,
            stops.empty() ? INVALID_TIME : (last_con == nullptr)
                                               ? current->_now
                                               : last_con->a_time,
            current->_now, last_con != nullptr);

        transports.emplace_back(station_index, station_index + 1,
                                (*std::next(it))->_now - current->_now, -1);

        walk_arrival = (*std::next(it))->_now;

        last_con = nullptr;

        state = AT_STATION;
        break;

      case IN_CONNECTION:
        transports.emplace_back(station_index, station_index + 1,
                                current->_connection);

        // do not collect the last connection route node.
        assert(std::next(it) != end(labels));
        auto succ = *std::next(it);
        if (succ->_node->is_route_node()) {
          stops.emplace_back(
              ++station_index, current->_node->get_station()->_id,
              current->_connection->_full_con->a_platform,
              succ->_connection->_full_con->d_platform,
              current->_connection->a_time, succ->_connection->d_time, false);
        }

        last_con = current->_connection;
        break;
    }
  }

  transports.front().slot = terminal_label->get_slot(true);
  transports.back().slot = terminal_label->get_slot(false);

  return {stops, transports};
}

}  // namespace intermediate

journey::transport generate_journey_transport(int from, int to,
                                              intermediate::transport const& t,
                                              schedule const& sched) {
  bool walk = false;
  std::string name;
  std::string cat_name;
  int cat_id = 0;
  int train_nr = 0;
  std::string line_identifier;
  int duration = t.duration;
  int slot = -1;
  std::string direction;
  std::string provider;

  if (t.con == nullptr) {
    walk = true;
    slot = t.slot;
  } else {
    connection_info const* con_info = t.con->_full_con->con_info;
    line_identifier = con_info->line_identifier;
    cat_name = sched.categories[con_info->family]->name;
    train_nr = con_info->train_nr;
    if (con_info->dir_ != nullptr) {
      direction = *con_info->dir_;
    }
    if (con_info->provider_ != nullptr) {
      provider = con_info->provider_->full_name;
    }
    switch (sched.categories[con_info->family]->output_rule) {
      case category::CATEGORY_AND_TRAIN_NUM:
        name = cat_name + " " + boost::lexical_cast<std::string>(train_nr);
        break;

      case category::CATEGORY: name = cat_name; break;

      case category::TRAIN_NUM:
        name = boost::lexical_cast<std::string>(train_nr);
        break;

      case category::NOTHING: break;

      case category::PROVIDER_AND_TRAIN_NUM:
        if (con_info->provider_ != nullptr) {
          name = con_info->provider_->short_name + " ";
        }
        name += boost::lexical_cast<std::string>(train_nr);
        break;

      case category::PROVIDER:
        if (con_info->provider_ != nullptr) {
          name = con_info->provider_->short_name;
        }
        break;
    }
  }

  return {from,     to,     walk,      name,
          cat_name, cat_id, train_nr,  line_identifier,
          duration, slot,   direction, provider};
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
  int from = 1;

  for (auto const& transport : transports) {
    connection_info const* con_info = nullptr;
    if (transport.con != nullptr) {
      con_info = transport.con->_full_con->con_info;
    }

    if (!con_info_eq(con_info, last_con_info)) {
      if (last != nullptr && isset_last) {
        journey_transports.push_back(
            generate_journey_transport(from, transport.from, *last, sched));
      }

      isset_last = true;
      last = &transport;
      from = transport.from;
    }

    last_con_info = con_info;
  }

  auto back = transports.back();
  journey_transports.push_back(
      generate_journey_transport(from, back.to, *last, sched));

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
                                         sched.tracks[stop.a_platform]}
             : journey::stop::event_info{false, 0, ""},
         stop.d_time != INVALID_TIME
             ? journey::stop::event_info{true, motis_to_unixtime(
                                                   sched.schedule_begin_,
                                                   stop.d_time),
                                         sched.tracks[stop.d_platform]}
             : journey::stop::event_info{false, 0, ""}});
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
      journey_attributes.push_back({range.from, range.to, code, text});
    }
  }

  return journey_attributes;
}

journey to_journey(label const* label, schedule const& sched) {
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
  return j;
}

}  // namespace motis
