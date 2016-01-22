#include "motis/routing/output/to_journey.h"

#include <string>

#include "boost/lexical_cast.hpp"

#include "motis/routing/output/interval_map.h"

namespace motis {
namespace routing {
namespace output {

journey::transport generate_journey_transport(unsigned int from,
                                              unsigned int to,
                                              intermediate::transport const& t,
                                              schedule const& sched,
                                              unsigned int route_id) {
  bool walk = false;
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

  if (t.con == nullptr) {
    walk = true;
    slot = t.slot;
  } else {
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

  return {from,     to,       walk,
          name,     cat_name, cat_id,
          clasz,    train_nr, line_identifier,
          duration, slot,     direction,
          provider, route_id};
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

}  // namespace output
}  // namespace routing
}  // namespace motis
